#ifndef __HYPERWIN_STRUCTS_H_
#define __HYPERWIN_STRUCTS_H_

#include <Windows.h>
#include "x86_64.h"

#define HWSTATUS DWORD64

typedef UINT64 OPERATION, * POPERATION;

typedef struct _GENERIC_COM_STRUCT
{
    OPERATION Operation;
    union
    {
        struct _INIT_ARGS
        {
            BOOLEAN IsMessageAvailable;
            DWORD64 MessageLength;
            BYTE Message[PAGE_SIZE];
        } InitArgs;
        struct _PROTECT_PROCESS
        {
            HANDLE Handle;
        } ProtectProcess;
    } ArgumentsUnion;
} GENERIC_COM_STRUCT, * PGENERIC_COM_STRUCT;

typedef struct _APPLICATION_DATA
{
    HWND ProcessNameTextbox;
} APPLICATION_DATA, *PAPPLICATION_DATA;
#endif