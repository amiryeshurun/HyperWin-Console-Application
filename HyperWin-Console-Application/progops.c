#include "progops.h"
#include "utils.h"
#include "comops.h"
#include "hwstatus.h"
#include <stdio.h>
#include <string.h>

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
    }
}

VOID HandleProtectProcess(IN HANDLE CommunicationDriver, IN PCHAR Buffer)
{
    PCHAR Token;
    while ((Token = strtok(NULL, " ")) != NULL)
    {
        if (!strncmp(Token, "-n", 2))
        {
            PCHAR ProcessName;
            if ((ProcessName = strtok(NULL, " ")) != NULL)
            {
                DWORD64 ProcessId;
                if (GetProcessIdByName(ProcessName, &ProcessId) != HYPERWIN_STATUS_SUCCUESS)
                {
                    hvPrint("Could not find process name: %s\n", ProcessName);
                    return;
                }
                HANDLE ProcessHandle;
                if ((ProcessHandle = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, ProcessId))
                    == NULL)
                {
                    hvPrint("Failed to open a handle to the desired process\n");
                    return;
                }
                if (MarkProcessProtected(CommunicationDriver, ProcessHandle) != HYPERWIN_STATUS_SUCCUESS)
                {
                    hvPrint("Could not mark process as protected\n");
                    return;
                }
            }
            else
            {
                hvPrint("You must ennter a process name");
                return;
            }
        }
    }
}