#include "progops.h"
#include "utils.h"
#include "comops.h"
#include "hwstatus.h"
#include <stdio.h>
#include <string.h>
#include <Windows.h>
#include <tlhelp32.h>
#include "ntdll_helpers.h"

VOID ProgramLoop(IN HANDLE CommunicationDriver)
{
    WCHAR InputBuffer[BUFFER_MAX_SIZE];
    PWCHAR Token;
    while (TRUE)
    {
        fgetws(InputBuffer, BUFFER_MAX_SIZE, stdin);
        if (InputBuffer[wcslen(InputBuffer) - 1] == L'\n')
            InputBuffer[wcslen(InputBuffer) - 1] = L'\0';
        Token = wcstok(InputBuffer, L" ", NULL);
        if (!wcsncmp(Token, L"exit", 4))
            break;
        else if (!wcsncmp(Token, L"protect-process", wcslen(L"protect-process")))
            HandleProtectProcess(CommunicationDriver, InputBuffer);
        else if (!wcsncmp(Token, L"get-process", wcslen(L"get-process")))
            HandleGetProcess();
        else if (!wcsncmp(Token, L"protect-file-data", wcslen(L"protect-file-data")))
            HandleProtectFileData(CommunicationDriver);
        else if (!wcscmp(Token, L"get-file-id", wcslen(L"get-file-id")))
            HandleGetFileId();
        else if (!wcscmp(Token, L"remove-file-protection", wcslen(L"remove-file-protection")))
            HandleRemoveProtection(CommunicationDriver);
    }
}

VOID HandleProtectProcess(IN HANDLE CommunicationDriver)
{
    PWCHAR Token, ProcessName;
    DWORD64 ProcessId;
    HANDLE ProcessHandle;
    HWSTATUS HwStatus;

    while ((Token = wcstok(NULL, L" ", NULL)) != NULL)
    {
        if (!wcsncmp(Token, L"-n", 2))
        {
            if ((ProcessName = wcstok(NULL, L" ", NULL)) != NULL)
            {
                if (GetProcessIdByName(ProcessName, &ProcessId) != HYPERWIN_STATUS_SUCCUESS)
                {
                    hvPrint(L"Could not find process name: %ls\n", ProcessName);
                    return;
                }
                goto ProtectProcessById;
            }
            else
            {
                hvPrint(L"You must ennter a process name");
                return;
            }
        }
        else if (!wcsncmp(Token, L"-i", 2))
        {
            if ((Token = wcstok(NULL, L" ", NULL)) != NULL)
            {
                ProcessId = _wtoi64(Token);
                goto ProtectProcessById;
            }
            else
            {
                hvPrint(L"You must enter a process ID\n");
                return;
            }
        }
        else if (!wcsncmp(Token, L"--self", 6))
        {
            ProcessId = -1;
            goto ProtectProcessById;
        }
        else
        {
            hvPrint(L"Unknown option: %ls\n", Token);
            return;
        }
    }

ProtectProcessById:
    if (ProcessId == -1)
        ProcessHandle = GetCurrentProcess();
    else
    {
        if ((ProcessHandle = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, ProcessId))
            == NULL)
        {
            hvPrint(L"Failed to open a handle to the desired process\n");
            return;
        }
    }
    if ((HwStatus = MarkProcessProtected(CommunicationDriver, ProcessHandle)) != HYPERWIN_STATUS_SUCCUESS)
    {
        hvPrint(L"Could not mark process as protected, HWSTATUS: %lld\n", HwStatus);
        return;
    }
    hvPrint(L"Successfully sent a request to mark process as protected\n");
}

VOID HandleGetProcess()
{
    PROCESSENTRY32W Entry;
    HANDLE Snapshot;
    WCHAR ProcessName[BUFFER_MAX_SIZE];

    Snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
    Entry.dwSize = sizeof(PROCESSENTRY32W);

    if (Process32First(Snapshot, &Entry))
    {
        while (Process32Next(Snapshot, &Entry))
        {
            wprintf(L"%ls: %d\n", Entry.szExeFile, Entry.th32ProcessID);
        }
    }
}

VOID HandleProtectFileData(IN HANDLE CommunicationDriver)
{
    PWCHAR Token, FilePath = NULL, ContentUtf16 = NULL;
    BYTE HiddenContent[BUFFER_MAX_SIZE];
    BOOLEAN OperationSpecified = FALSE;
    DWORD ProtectionOperation = 0, EncodingTypeEnum = 0x1;
    HANDLE FileHandle = NULL;
    HWSTATUS HwStatus;

    while ((Token = wcstok(NULL, L" ", NULL)) != NULL)
    {
        if (!wcsncmp(Token, L"-p", 2))
        {
            if ((Token = wcstok(NULL, L" ", NULL)) == NULL)
            {
                hvPrint(L"You must enter a file path\n");
                return;
            }
            // Token now contains the full file path as unicode string
            if ((FileHandle = CreateFileW(Token,
                MAXIMUM_ALLOWED,
                FILE_SHARE_READ | FILE_SHARE_WRITE,
                NULL, OPEN_EXISTING,
                FILE_ATTRIBUTE_NORMAL,  
                NULL)) == INVALID_HANDLE_VALUE)
            {
                if (GetLastError() == ERROR_FILE_NOT_FOUND)
                {
                    hvPrint(L"This file does not exist\n");
                    return;
                }
            }
        }
        if (!wcsncmp(Token, L"-h", 2))
        {
            OperationSpecified = TRUE;
            if (FileHandle == NULL)
            {
                hvPrint(L"You can not use -h without specifying the file path\n");
                return;
            }
            ProtectionOperation = FILE_PROTECTION_HIDE;
            if ((Token = wcstok(NULL, L" ", NULL)) == NULL)
            {
                hvPrint(L"You must specify the contect to hide\n");
                return;
            }
            ContentUtf16 = _wcsdup(Token);
        }
        if (!wcsncmp(Token, L"-e", 2))
        {
            if ((Token = wcstok(NULL, L" ", NULL)) == NULL)
            {
                hvPrint(L"You must specify encoding type when using the -e flag\n");
                return;
            }
            if (!wcscmp(Token, L"utf-16"))
                EncodingTypeEnum = ENCODING_TYPE_UTF_16;
        }
    }

PerformProtection:
    if (!OperationSpecified)
    {
        hvPrint(L"You must specify the protection operation\n");
        return;
    }
    // If the encoding type is utf-8, we must convert the input
    if (EncodingTypeEnum == ENCODING_TYPE_UTF_8)
    {
        WideCharToMultiByte(CP_ACP, 
            WC_COMPOSITECHECK,
            ContentUtf16,
            -1,
            HiddenContent,
            BUFFER_MAX_SIZE,
            NULL,
            NULL);
    }
    // Else, just copy it to the buffer
    else if (EncodingTypeEnum == ENCODING_TYPE_UTF_16)
        memcpy(HiddenContent, ContentUtf16, (wcslen(ContentUtf16) + 1)* sizeof(WCHAR));
    // Send a request to HyperWin
    if ((HwStatus = ProtectFileData(CommunicationDriver,
        FileHandle,
        ProtectionOperation,
        EncodingTypeEnum,
        HiddenContent,
        NULL))
        != HYPERWIN_STATUS_SUCCUESS)
    {
        hvPrint(L"Failed to protect the specified data, HWSTATUS: %lld\n", HwStatus);
    }
    CloseHandle(FileHandle);
}

VOID HandleGetFileId()
{
    PWCHAR Token, FilePath = NULL;
    HANDLE FileHandle = NULL;
    _NtQueryInformationFile NtQueryInformationFile;
    IO_STATUS_BLOCK IoStatusBlock;
    HWSTATUS Status;
    DWORD64 FileId;
    NTSTATUS NtStatus;

    if((Token = wcstok(NULL, L" ", NULL)) != NULL)
    {
        // Token now contains a name of a file
        if ((FileHandle = CreateFileW(Token,
            MAXIMUM_ALLOWED,
            FILE_SHARE_READ | FILE_SHARE_WRITE,
            NULL, OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL,
            NULL)) == NULL)
        {
            hvPrint(L"Could not open file: %d", GetLastError());
            return;
        }
        if ((Status = LoadNtdllFunction("NtQueryInformationFile", &NtQueryInformationFile)) != HYPERWIN_STATUS_SUCCUESS)
        {
            hvPrint(L"Failed to load NtQuertyInformationFile: %d\n", Status);
            CloseHandle(FileHandle);
            return;
        }
        NtStatus = NtQueryInformationFile(FileHandle, &IoStatusBlock, &FileId, sizeof(FileId), 6);
        if (!NT_SUCCESS(NtStatus))
        {
            hvPrint(L"Failed to get file info. NTSTATUS: %llx, Error code: %d\n", NtStatus, GetLastError());
            CloseHandle(FileHandle);
            return;
        }
        wprintf(L"%llx\n", FileId);
        CloseHandle(FileHandle);
    }
    else
        hvPrint(L"You must enter a file path\n");
}

VOID HandleRemoveProtection(IN HANDLE CommunicationDriver)
{
    HANDLE FileHandle = NULL;
    PWCHAR Token;
    HWSTATUS HwStatus;

    while ((Token = wcstok(NULL, L" ", NULL)) != NULL)
    {
        if (!wcscmp(Token, L"-p"))
        {
            if ((Token = wcstok(NULL, L" ", NULL)) == NULL)
            {
                hvPrint(L"You must enter a path to a file\n");
                return;
            }
            FileHandle = CreateFileW(Token,
                MAXIMUM_ALLOWED,
                FILE_SHARE_READ | FILE_SHARE_WRITE,
                NULL,
                OPEN_EXISTING,
                FILE_ATTRIBUTE_NORMAL,
                NULL);
            if (FileHandle == INVALID_HANDLE_VALUE)
            {
                hvPrint(L"Could not open file: %d", GetLastError());
                return;
            }
        }
    }
    if ((HwStatus = RemoveFileProtection(CommunicationDriver, FileHandle))
            != HYPERWIN_STATUS_SUCCUESS)
        hvPrint(L"Could not remove protection from file, HWSTATUS: %lld\n", HwStatus);
    CloseHandle(FileHandle);
}