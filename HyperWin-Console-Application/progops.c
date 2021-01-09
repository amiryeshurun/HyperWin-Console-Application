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
    PWCHAR* Tokens;
    HWSTATUS Status;

    while (TRUE)
    {
        fgetws(InputBuffer, BUFFER_MAX_SIZE, stdin);
        if (InputBuffer[wcslen(InputBuffer) - 1] == L'\n')
            InputBuffer[wcslen(InputBuffer) - 1] = L'\0';
        if ((Status = GetTokens(InputBuffer, &Tokens)) != HYPERWIN_STATUS_SUCCUESS)
        {
            PrintErrorMessage(Status);
            continue;
        }
        Status = DispatchCommand(CommunicationDriver, Tokens);
        if (Status == HYPERWIN_EXIT)
        {
            FreeTokens(Tokens);
            break;
        }
        else if(Status != HYPERWIN_STATUS_SUCCUESS)
            PrintErrorMessage(Status);
        FreeTokens(Tokens);
    }
    
}

HWSTATUS DispatchCommand(IN HANDLE CommunicationDriver, IN PWCHAR* Tokens)
{
    if (!wcsncmp(*Tokens, L"exit", 4))
        return HYPERWIN_EXIT;
    else if (!wcsncmp(*Tokens, L"protect-process", wcslen(L"protect-process")))
        return HandleProtectProcess(CommunicationDriver, ++Tokens);
    else if (!wcsncmp(*Tokens, L"get-process", wcslen(L"get-process")))
        return HandleGetProcess();
    else if (!wcsncmp(*Tokens, L"protect-file-data", wcslen(L"protect-file-data")))
        return HandleProtectFileData(CommunicationDriver, ++Tokens);
    else if (!wcscmp(*Tokens, L"get-file-id", wcslen(L"get-file-id")))
        return HandleGetFileId(++Tokens);
    else if (!wcscmp(*Tokens, L"remove-file-protection", wcslen(L"remove-file-protection")))
        return HandleRemoveProtection(CommunicationDriver, ++Tokens);
    else if (!wcscmp(*Tokens, L"create-group", wcslen(L"remove-file-protection")))
        return HandleCreateGroup(CommunicationDriver, ++Tokens);

}

HWSTATUS HandleProtectProcess(IN HANDLE CommunicationDriver, IN PWCHAR* Tokens)
{
    PWCHAR ProcessName;
    DWORD64 ProcessId;
    HANDLE ProcessHandle;
    HWSTATUS HwStatus = HYPERWIN_STATUS_SUCCUESS;

    while (*Tokens)
    {
        if (!wcsncmp(*Tokens, L"-n", 2))
        {
            ProcessName = *(++Tokens);
            if (!ProcessName || GetProcessIdByName(ProcessName, &ProcessId) != HYPERWIN_STATUS_SUCCUESS)
                return HYPERWIN_PROCESS_NOT_FOUND;
        }
        else if (!wcsncmp(*Tokens, L"-i", 2))
        {
            Tokens++;
            ProcessId = _wtoi64(*Tokens);
        }
        else if (!wcsncmp(*Tokens, L"--self", 6))
            ProcessId = -1;
        else
            return HYPERWIN_UNKNOWN_COMMAND;
        Tokens++;
    }

    if (ProcessId == -1)
        ProcessHandle = GetCurrentProcess();
    else
    {
        if ((ProcessHandle = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, ProcessId))
            == INVALID_HANDLE_VALUE)
            return HYPERWIN_INVALID_HANDLE;
    }
    HwStatus = MarkProcessProtected(CommunicationDriver, ProcessHandle);
    CloseHandle(ProcessHandle);
    return HwStatus;
}

HWSTATUS HandleGetProcess()
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
    return HYPERWIN_STATUS_SUCCUESS;
}

HWSTATUS HandleProtectFileData(IN HANDLE CommunicationDriver, IN PWCHAR* Tokens)
{
    PWCHAR FilePath = NULL, ContentUtf16 = NULL;
    BYTE HiddenContent[BUFFER_MAX_SIZE];
    BOOLEAN OperationSpecified = FALSE;
    DWORD ProtectionOperation = 0, EncodingTypeEnum = 0x1;
    HANDLE FileHandle = NULL;
    HWSTATUS HwStatus = HYPERWIN_STATUS_SUCCUESS;

    while (*Tokens)
    {
        if (!wcsncmp(*Tokens, L"-p", 2))
        {
            // Token now contains the full file path as unicode string
            if ((FileHandle = CreateFileW(*(++Tokens),
                MAXIMUM_ALLOWED,
                FILE_SHARE_READ | FILE_SHARE_WRITE,
                NULL, OPEN_EXISTING,
                FILE_ATTRIBUTE_NORMAL,  
                NULL)) == INVALID_HANDLE_VALUE)
            {
                if (GetLastError() == ERROR_FILE_NOT_FOUND)
                    return HYPERWIN_FILE_NOT_FOUND;
            }
        }
        else if (!wcsncmp(*Tokens, L"-h", 2))
        {
            OperationSpecified = TRUE;
            if (FileHandle == NULL)
                return HYPERWIN_PATH_MUST_BE_SPECIFIED;
            ProtectionOperation = FILE_PROTECTION_HIDE;
            ContentUtf16 = _wcsdup(*(++Tokens));
        }
        else if (!wcsncmp(*Tokens, L"-e", 2))
        {
            if (*(Tokens + 1) == NULL)
                return HYPERWIN_UNKNOWN_ENCODING_TYPE;
            Tokens++;
            if (!wcscmp(*Tokens, L"utf-16"))
                EncodingTypeEnum = ENCODING_TYPE_UTF_16;
            else if (!wcscmp(*Tokens, L"utf-8"))
                EncodingTypeEnum = ENCODING_TYPE_UTF_8;
            else
                return HYPERWIN_UNKNOWN_ENCODING_TYPE;
        }
        Tokens++;
    }
    if (!OperationSpecified)
        return HYPERWIN_UNKNOWN_OPERATION;

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
    HwStatus = ProtectFileData(CommunicationDriver,
        FileHandle,
        ProtectionOperation,
        EncodingTypeEnum,
        HiddenContent,
        NULL);
    CloseHandle(FileHandle);
    return HwStatus;
}

HWSTATUS HandleGetFileId(IN PWCHAR* Tokens)
{
    PWCHAR FilePath = NULL;
    HANDLE FileHandle = NULL;
    _NtQueryInformationFile NtQueryInformationFile;
    IO_STATUS_BLOCK IoStatusBlock;
    HWSTATUS Status;
    DWORD64 FileId;
    NTSTATUS NtStatus;

    if (!(*Tokens) || wcscmp(*Tokens, L"-p", 2))
        return HYPERWIN_PATH_MUST_BE_SPECIFIED;
        
    // Token now contains a name of a file
    if ((FileHandle = CreateFileW(*(++Tokens),
        MAXIMUM_ALLOWED,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        NULL, OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL)) == INVALID_HANDLE_VALUE)
    {
        if (GetLastError() == ERROR_FILE_NOT_FOUND)
            return HYPERWIN_FILE_NOT_FOUND;
    }
    if ((Status = LoadNtdllFunction("NtQueryInformationFile", &NtQueryInformationFile)) != HYPERWIN_STATUS_SUCCUESS)
    {
        CloseHandle(FileHandle);
        return Status;
    }
    NtStatus = NtQueryInformationFile(FileHandle, &IoStatusBlock, &FileId, sizeof(FileId), 6);
    if (!NT_SUCCESS(NtStatus))
    {
        CloseHandle(FileHandle);
        return HYPERWIN_QUERY_INFO_FAILED;;
    }
    wprintf(L"%llx\n", FileId);
    CloseHandle(FileHandle);
    return HYPERWIN_STATUS_SUCCUESS;
}

HWSTATUS HandleRemoveProtection(IN HANDLE CommunicationDriver, IN PWCHAR* Tokens)
{
    HANDLE FileHandle = NULL;
    HWSTATUS HwStatus = HYPERWIN_STATUS_SUCCUESS;

    while (*Tokens)
    {
        if (!wcscmp(*Tokens, L"-p"))
        {
            FileHandle = CreateFileW(*(++Tokens),
                MAXIMUM_ALLOWED,
                FILE_SHARE_READ | FILE_SHARE_WRITE,
                NULL,
                OPEN_EXISTING,
                FILE_ATTRIBUTE_NORMAL,
                NULL);
            if (GetLastError() == ERROR_FILE_NOT_FOUND)
                return HYPERWIN_FILE_NOT_FOUND;
        }
        Tokens++;
    }
    if (!FileHandle)
        return HYPERWIN_PATH_MUST_BE_SPECIFIED;
    HwStatus = RemoveFileProtection(CommunicationDriver, FileHandle);
    CloseHandle(FileHandle);
    return HwStatus;
}

HWSTATUS HandleCreateGroup(IN HANDLE CommunicationDriver, IN PWCHAR* Tokens)
{
    return HYPERWIN_STATUS_SUCCUESS;
}