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
    CHAR InputBuffer[BUFFER_MAX_SIZE];
    PCHAR Token;
    while (TRUE)
    {
        fgets(InputBuffer, BUFFER_MAX_SIZE, stdin);
        if (InputBuffer[strlen(InputBuffer) - 1] == '\n')
            InputBuffer[strlen(InputBuffer) - 1] = '\0';
        Token = strtok(InputBuffer, " ");
        if (!strncmp(Token, "exit", 4))
            break;
        else if (!strncmp(Token, "protect-process", strlen("protect-process")))
            HandleProtectProcess(CommunicationDriver, InputBuffer);
        else if (!strncmp(Token, "get-process", strlen("get-process")))
            HandleGetProcess();
    }
}

VOID HandleProtectProcess(IN HANDLE CommunicationDriver, IN PCHAR Buffer)
{
    PCHAR Token;
    DWORD64 ProcessId;
    while ((Token = strtok(NULL, " ")) != NULL)
    {
        if (!strncmp(Token, "-n", 2))
        {
            PCHAR ProcessName;
            if ((ProcessName = strtok(NULL, " ")) != NULL)
            {
                if (GetProcessIdByName(ProcessName, &ProcessId) != HYPERWIN_STATUS_SUCCUESS)
                {
                    hvPrint("Could not find process name: %s\n", ProcessName);
                    return;
                }
                goto ProtectProcessById;
            }
            else
            {
                hvPrint("You must ennter a process name");
                return;
            }
        }
        else if (!strncmp(Token, "-i", 2))
        {
            if ((Token = strtok(NULL, " ")) != NULL)
            {
                ProcessId = atoi(Token);
                goto ProtectProcessById;
            }
            else
            {
                hvPrint("You must enter a process ID\n");
                return;
            }
        }
        else if (!strncmp(Token, "--self", 6))
        {
            ProcessId = -1;
            goto ProtectProcessById;
        }
        else
        {
            hvPrint("Unknown option: %s\n", Token);
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
            hvPrint("Failed to open a handle to the desired process\n");
            return;
        }
    }
    if (MarkProcessProtected(CommunicationDriver, ProcessHandle) != HYPERWIN_STATUS_SUCCUESS)
    {
        hvPrint("Could not mark process as protected\n");
        return;
    }
    hvPrint("Successfully sent a request to mark process as protected\n");
}

VOID HandleGetProcess()
{
    PROCESSENTRY32W Entry;
    HANDLE Snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
    CHAR ProcessName[BUFFER_MAX_SIZE];
    Entry.dwSize = sizeof(PROCESSENTRY32W);

    if (Process32First(Snapshot, &Entry))
    {
        while (Process32Next(Snapshot, &Entry))
        {
            WideCharToMultiByte(CP_ACP, 0, Entry.szExeFile, -1, ProcessName, BUFFER_MAX_SIZE, NULL, NULL);
            printf("%s: %d\n", ProcessName, Entry.th32ProcessID);
        }
    }
}