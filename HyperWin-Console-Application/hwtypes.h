#ifndef __HYPERWIN_STRUCTS_H_
#define __HYPERWIN_STRUCTS_H_

#include <Windows.h>
#include "x86_64.h"

#define HWSTATUS DWORD64
#define FILE_PATH_MAX_LENGTH 256

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
        struct _PROTECT_FILE_DATA
        {
            HANDLE FileHandle;
            DWORD64 ContentLength;
            BYTE Content[FILE_PATH_MAX_LENGTH];
            DWORD ProtectionOperation;
            DWORD EncodingType;
        } ProtectFileData;
        struct _REMOVE_FILE_OPERATION
        {
            HANDLE FileHandle;
        } RemoveFileProtection;
    } ArgumentsUnion;
} GENERIC_COM_STRUCT, * PGENERIC_COM_STRUCT;

typedef struct _APPLICATION_DATA
{
    HWND ProcessNameTextbox;
} APPLICATION_DATA, *PAPPLICATION_DATA;

enum
{
    FILE_PROTECTION_HIDE = 0x1
};

enum
{
    ENCODING_TYPE_UTF_8 = 0x1,
    ENCODING_TYPE_UTF_16 = 0x2
};

#endif