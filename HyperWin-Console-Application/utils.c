#include "utils.h"
#include "hwstatus.h"
#include <Windows.h>
#include <tlhelp32.h>
#include <stdio.h>
#include <string.h>

VOID DebugOutA(PCHAR fmt, ...)
{
	CHAR str[1025];
	va_list args;
	va_start(args, fmt);
	wvsprintfA(str, fmt, args);
	va_end(args);
	OutputDebugStringA(str);
}

HWSTATUS GetProcessIdByName(IN PWCHAR ProcessName, OUT PDWORD64 ProcessId)
{
    PROCESSENTRY32W Entry;
    HANDLE Snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
    DWORD64 len = wcslen(ProcessName);

    Entry.dwSize = sizeof(PROCESSENTRY32W);
    if (Process32First(Snapshot, &Entry))
    {
        while (Process32Next(Snapshot, &Entry))
        {
            if (!wcscmp(Entry.szExeFile, ProcessName))
            {
                *ProcessId = Entry.th32ProcessID;
                CloseHandle(Snapshot);
                return HYPERWIN_STATUS_SUCCUESS;
            }
        }
    }
    CloseHandle(Snapshot);
    return HYPERWIN_PROCESS_NOT_FOUND;
}