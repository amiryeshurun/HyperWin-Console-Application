#include "progops.h"
#include "utils.h"
#include "comops.h"
#include "hwstatus.h"
#include <stdio.h>
#include <string.h>
#include <Windows.h>
#include <tlhelp32.h>

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
    }
}

VOID HandleProtectProcess(IN HANDLE CommunicationDriver)
{
    PWCHAR Token;
    DWORD64 ProcessId;
    while ((Token = wcstok(NULL, L" ", NULL)) != NULL)
    {
        if (!wcsncmp(Token, L"-n", 2))
        {
            PWCHAR ProcessName;
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
    NOP
    HANDLE ProcessHandle;
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
    if (MarkProcessProtected(CommunicationDriver, ProcessHandle) != HYPERWIN_STATUS_SUCCUESS)
    {
        hvPrint(L"Could not mark process as protected\n");
        return;
    }
    hvPrint(L"Successfully sent a request to mark process as protected\n");
}

VOID HandleGetProcess()
{
    PROCESSENTRY32W Entry;
    HANDLE Snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
    WCHAR ProcessName[BUFFER_MAX_SIZE];
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
    PWCHAR Token, FilePath = NULL, HiddenContent = NULL;
    BOOLEAN OperationSpecified = FALSE;
    DWORD ProtectionOperation = 0;

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
            HANDLE FileHandle;
            if ((FileHandle = CreateFileW(Token,
                MAXIMUM_ALLOWED,
                FILE_SHARE_READ | FILE_SHARE_WRITE,
                NULL, OPEN_EXISTING,
                FILE_ATTRIBUTE_NORMAL,  
                NULL)) == NULL)
            {
                if (GetLastError() == ERROR_FILE_NOT_FOUND)
                {
                    hvPrint(L"This file does not exist\n");
                    return;
                }
            }
            CloseHandle(FileHandle);
            // The file exists, copy the path
            FilePath = _wcsdup(Token);
        }
        if (!wcsncmp(Token, L"-h", 2))
        {
            OperationSpecified = TRUE;
            if (FilePath == NULL)
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
            HiddenContent = _wcsdup(Token);
        }
    }

PerformProtection:
    if (!OperationSpecified)
    {
        hvPrint(L"You must specify the protection operation\n");
        return;
    }
    // Send a request to HyperWin
    if (ProtectFileData(CommunicationDriver, FilePath, ProtectionOperation, HiddenContent, NULL) 
        != HYPERWIN_STATUS_SUCCUESS)
        hvPrint(L"Failed to protect the specified data\n");
    free(FilePath);
    free(HiddenContent);
}