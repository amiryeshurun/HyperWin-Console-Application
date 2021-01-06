#include "comops.h"
#include "hwops.h"
#include "hwstatus.h"
#include "utils.h"

HWSTATUS SendInitSignal(IN HANDLE Handle, IN PGENERIC_COM_STRUCT Args)
{
	CHAR Message[] = "Hello from guest!";
	DWORD64 Length = strlen(Message);
	INT Dummy;
	HWSTATUS HwStatus;

	Args->Operation = OPERATION_INIT;
	Args->ArgumentsUnion.InitArgs.IsMessageAvailable = TRUE;
	Args->ArgumentsUnion.InitArgs.MessageLength = Length;
	strcpy_s(Args->ArgumentsUnion.InitArgs.Message, Length + 1, Message);
	if (!DeviceIoControl(Handle, CTL_CODE_HW, Args, sizeof(*Args), &HwStatus, sizeof(HWSTATUS), &Dummy, NULL))
	{
		hvPrint(L"DeviceIoControl failed: %d\n", GetLastError());
		return HYPERWIN_IOCTL_FAILED;
	}
	return HwStatus;
}

HWSTATUS MarkProcessProtected(IN HANDLE Handle, IN HANDLE ProcessHandle)
{
	GENERIC_COM_STRUCT Args;
	INT Dummy;
	HWSTATUS HwStatus;

	Args.Operation = OPERATION_PROTECTED_PROCESS;
	Args.ArgumentsUnion.ProtectProcess.Handle = ProcessHandle;
	if (!DeviceIoControl(Handle, CTL_CODE_HW, &Args, sizeof(Args), &HwStatus, sizeof(HWSTATUS), &Dummy, NULL))
	{
		hvPrint(L"DeviceIoControl failed: %d\n", GetLastError());
		return HYPERWIN_IOCTL_FAILED;
	}
	return HwStatus;
}

HWSTATUS ProtectFileData(IN HANDLE Handle, IN HANDLE FileHandle, IN DWORD ProtectionOperation, IN DWORD Encoding, IN PVOID Content,
	IN PVOID Extension)
{
	GENERIC_COM_STRUCT Args;
	INT Dummy;
	DWORD64 size;
	HWSTATUS HwStatus;

	Args.Operation = OPERATION_PROTECT_FILE_DATA;
	Args.ArgumentsUnion.ProtectFileData.FileHandle = FileHandle;
	if (Encoding == ENCODING_TYPE_UTF_8)
		size = strlen((PCHAR)Content);
	else if (Encoding == ENCODING_TYPE_UTF_16)
		size = 2 * wcslen((PWCHAR)Content);
	else
		return HYPERWIN_UNKNOWN_ENCODING_TYPE;
	Args.ArgumentsUnion.ProtectFileData.EncodingType = Encoding;
	Args.ArgumentsUnion.ProtectFileData.ContentLength = size;
	memcpy(Args.ArgumentsUnion.ProtectFileData.Content, Content, size);
	Args.ArgumentsUnion.ProtectFileData.ProtectionOperation = ProtectionOperation;
	if (!DeviceIoControl(Handle, CTL_CODE_HW, &Args, sizeof(Args), &HwStatus, sizeof(HWSTATUS), &Dummy, NULL))
	{
		hvPrint(L"DeviceIoControl failed: %d\n", GetLastError());
		return HYPERWIN_IOCTL_FAILED;
	}
	return HwStatus;
}

HWSTATUS RemoveFileProtection(IN HANDLE Handle, IN HANDLE FileHandle)
{
	GENERIC_COM_STRUCT Args;
	INT Dummy;
	DWORD64 size;
	HWSTATUS HwStatus;

	Args.Operation = OPERATION_REMOVE_FILE_PROTECTION;
	Args.ArgumentsUnion.RemoveFileProtection.FileHandle = FileHandle;
	if (!DeviceIoControl(Handle, CTL_CODE_HW, &Args, sizeof(Args), &HwStatus, sizeof(HWSTATUS), &Dummy, NULL))
	{
		hvPrint(L"DeviceIoControl failed: %d\n", GetLastError());
		return HYPERWIN_IOCTL_FAILED;
	}
	return HwStatus;
}