#ifndef __NTDLL_HELPERS_H_
#define __NTDLL_HELPERS_H_

#include "hwtypes.h"

HWSTATUS LoadNtdllFunction(IN PCHAR FunctionName, OUT PVOID* FunctionPtr);

typedef struct _IO_STATUS_BLOCK {
	union {
		NTSTATUS Status;
		PVOID    Pointer;
	} DUMMYUNIONNAME;
	ULONG_PTR Information;
} IO_STATUS_BLOCK, * PIO_STATUS_BLOCK;

typedef NTSTATUS(__stdcall* _NtQueryInformationFile)(
	HANDLE                 FileHandle,
	PIO_STATUS_BLOCK       IoStatusBlock,
	PVOID                  FileInformation,
	ULONG                  Length,
	ULONG				   FileInformationClass
	);

#define NT_SUCCESS(STATUS) ((STATUS)>=0) 

#endif