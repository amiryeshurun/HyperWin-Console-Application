#ifndef __COMMUNICATION_OPS_H_
#define __COMMUNICATION_OPS_H_

#include "hwtypes.h"

HWSTATUS SendInitSignal(IN HANDLE Handle, IN PGENERIC_COM_STRUCT Args);
HWSTATUS MarkProcessProtected(IN HANDLE Handle, IN HANDLE ProcessHandle);
HWSTATUS ProtectFileData(IN HANDLE Handle, IN HANDLE FileHandle, IN DWORD ProtectionOperation, IN DWORD Encoding, IN PVOID Content,
	IN PVOID Extension);
HWSTATUS RemoveFileProtection(IN HANDLE Handle, IN HANDLE FileHandle);

#endif