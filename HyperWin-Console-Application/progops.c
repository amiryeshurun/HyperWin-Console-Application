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
            HandleProtectFileData();
    }
}

VOID HandleProtectProcess(IN HANDLE CommunicationDriver, IN PWCHAR Buffer)
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

VOID HandleProtectFileData()
{
    PWCHAR Token;
    while ((Token = wcstok(NULL, L" ", NULL)) != NULL)
    {
        if (!wcsncmp(Token, L"-p", 2))
        {
            if ((Token = wcstok(NULL, L" ", NULL)) == NULL)
            {
                hvPrint(L"You must enter a file path\n");
                return;
            }
            OFSTRUCT FileData;
            // Token now contains the full file path as ANSI string
            HFILE FileHandle = OpenFile(Token, &FileData, OF_EXIST);
            if (FileHandle == -1)
            {
                hvPrint(L"This file does not exist\n");
                return;
            }
        }
    }
}