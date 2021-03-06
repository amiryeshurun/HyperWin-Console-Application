#ifndef __HYPERWIN_UTILS_H_
#define __HYPERWINS_UTILS_H_

#include <Windows.h>
#include <debugapi.h>
#include "hwtypes.h"

VOID DebugOutA(PCHAR fmt, ...);
HWSTATUS GetProcessIdByName(IN PWCHAR ProcessName, OUT PDWORD64 ProcessId);
HWSTATUS GetTokens(IN PWCHAR InputBuffer, OUT PWCHAR** OutputTokens);
VOID PrintErrorMessage(IN HWSTATUS Status);
VOID FreeTokens(IN PWCHAR* Tokens);
VOID PrintTokens(IN PWCHAR* Tokens);

#define hvPrint(str, ...) wprintf(L"HyperWin :: " \
												str, \
												__VA_ARGS__)
#define DEBUG_LEVEL_INFO 1
#define DEBUG_LEVEL_WARNING 2
#define DEBUG_LEVEL_DEBUG 3

#define DEBUG_LEVEL DEBUG_LEVEL_DEBUG

#define BUFFER_MAX_SIZE 1024

#endif