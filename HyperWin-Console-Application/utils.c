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

HWSTATUS GetTokens(IN PWCHAR InputBuffer, OUT PWCHAR** OutputTokens)
{
    DWORD64 Count = 0, CurrentCount, i;
    PWCHAR CurrentToken, * Tokens = NULL;
    BOOLEAN Quotes = FALSE;
    HWSTATUS Status;

    while (*InputBuffer != '\0')
    {
        // Does the current token start with '"'?
        if (*InputBuffer == '"')
        {
            Quotes = TRUE;
            InputBuffer++;
        }
        else
            Quotes = FALSE;
        CurrentToken = InputBuffer;
        CurrentCount = 0;
        if (Quotes)
        {
            while (*InputBuffer != '"' && *InputBuffer != '\0')
            {
                InputBuffer++;
                CurrentCount++;
            }
            if (*InputBuffer == '\0')
            {
                Status = HYPERWIN_PARSING_ERROR;
                goto ErrorCleanup;
            }
            // Skip the last '"'
            InputBuffer++;
        }
        else
        {
            while (*InputBuffer != ' ' && *InputBuffer != '\0')
            {
                InputBuffer++;
                CurrentCount++;
            }
        }
        Tokens = (PWCHAR*)realloc(Tokens, (++Count) * sizeof(PWCHAR));
        if (!Tokens)
        {
            Status = HYPERWIN_ALLOCATION_ERROR;
            goto ErrorCleanup;
        }
        Tokens[Count - 1] = (PWCHAR)malloc((CurrentCount + 1)* sizeof(WCHAR));
        if (!Tokens[Count - 1])
        {
            Status = HYPERWIN_ALLOCATION_ERROR;
            goto ErrorCleanup;
        }
        memcpy(Tokens[Count - 1], CurrentToken, CurrentCount * 2);
        Tokens[Count - 1][CurrentCount] = '\0';
        if (*InputBuffer == '\0')
            break;
        InputBuffer++;
    }
    Tokens = (PWCHAR*)realloc(Tokens, (++Count) * sizeof(PWCHAR));
    if (!Tokens[Count - 1])
    {
        Status = HYPERWIN_ALLOCATION_ERROR;
        goto ErrorCleanup;
    }
    Tokens[Count - 1] = NULL;
    
    *OutputTokens = Tokens;
    return HYPERWIN_STATUS_SUCCUESS;
ErrorCleanup:
    if (Tokens)
    {
        for (i = 0; i < Count; i++)
            free(Tokens[i]);
        free(Tokens);
    }
    return Status;
}

VOID PrintErrorMessage(IN HWSTATUS Status)
{
    switch (Status)
    {
    case HYPERWIN_PARSING_ERROR:
        hvPrint(L"Could not parge string\n");
        break;
    case HYPERWIN_ALLOCATION_ERROR:
        hvPrint(L"Allocation error\n");
        break;
    default:
        hvPrint(L"Error: %d\n", Status);
    }
}

VOID FreeTokens(IN PWCHAR* Tokens)
{
    PWCHAR* Backup = Tokens;
    while (*Tokens)
        free(*(Tokens++));
    free(Backup);
}

VOID PrintTokens(IN PWCHAR* Tokens)
{
    while (*Tokens)
        _putws(*(Tokens++));
}
