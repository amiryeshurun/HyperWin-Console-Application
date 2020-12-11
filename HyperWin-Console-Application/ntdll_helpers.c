#include "ntdll_helpers.h"
#include "hwstatus.h"

HWSTATUS LoadNtdllFunction(IN PCHAR FunctionName, OUT PVOID* FunctionPtr)
{
	HANDLE LibraryHandle;
	if (LibraryHandle = LoadLibraryW(L"ntdll.dll"))
	{
		*FunctionPtr = GetProcAddress(LibraryHandle, FunctionName);
		FreeLibrary(LibraryHandle);
		return HYPERWIN_STATUS_SUCCUESS;
	}
	return HYPERWIN_LIBRARY_NOT_FOUND;
}