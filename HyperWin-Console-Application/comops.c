#include "comops.h"
#include "hwops.h"
#include "hwstatus.h"
#include "utils.h"

HWSTATUS SendInitSignal(IN HANDLE Handle, IN PGENERIC_COM_STRUCT Args)
{
	CHAR Message[] = "Hello from guest!";
	DWORD64 Length = strlen(Message);
	Args->Operation = OPERATION_INIT;
	Args->ArgumentsUnion.InitArgs.IsMessageAvailable = TRUE;
	Args->ArgumentsUnion.InitArgs.MessageLength = Length;
	strcpy_s(Args->ArgumentsUnion.InitArgs.Message, Length + 1, Message);
	INT Dummy;
	if (!DeviceIoControl(Handle, CTL_CODE_HW, Args, sizeof(*Args), NULL, 0, &Dummy, NULL))
	{
		hvPrint(L"DeviceIoControl failed: %d\n", GetLastError());
		return HYPERWIN_IOCTL_FAILED;
	}
	return HYPERWIN_STATUS_SUCCUESS;
}

HWSTATUS MarkProcessProtected(IN HANDLE Handle, IN HANDLE ProcessHandle)
{
	GENERIC_COM_STRUCT Args;
	Args.Operation = OPERATION_PROTECTED_PROCESS;
	Args.ArgumentsUnion.ProtectProcess.Handle = ProcessHandle;
	INT Dummy;
	if (!DeviceIoControl(Handle, CTL_CODE_HW, &Args, sizeof(Args), NULL, 0, &Dummy, NULL))
	{
		hvPrint(L"DeviceIoControl failed: %d\n", GetLastError());
		return HYPERWIN_IOCTL_FAILED;
	}
	return HYPERWIN_STATUS_SUCCUESS;
}

HWSTATUS ProtectFileData(IN HANDLE Handle, IN HANDLE FileHandle, IN DWORD ProtectionOperation, IN PWCHAR Content,
	IN PWCHAR Extension)
{
	GENERIC_COM_STRUCT Args;
	INT Dummy;

	Args.Operation = OPERATION_PROTECT_FILE_DATA;
	Args.ArgumentsUnion.ProtectFileData.FileHandle = FileHandle;
	Args.ArgumentsUnion.ProtectFileData.ContentLength = wcslen(Content);
	memcpy(Args.ArgumentsUnion.ProtectFileData.Content, Content, Args.ArgumentsUnion.ProtectFileData.ContentLength);
	Args.ArgumentsUnion.ProtectFileData.ProtectionOperation = ProtectionOperation;
	if (!DeviceIoControl(Handle, CTL_CODE_HW, &Args, sizeof(Args), NULL, 0, &Dummy, NULL))
	{
		hvPrint(L"DeviceIoControl failed: %d\n", GetLastError());
		return HYPERWIN_IOCTL_FAILED;
	}
	return HYPERWIN_STATUS_SUCCUESS;
}