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

HWSTATUS ProtectFileData(IN HANDLE Handle, IN HANDLE FileHandle, IN DWORD ProtectionOperation, IN DWORD Encoding, IN PVOID Content,
	IN PVOID Extension)
{
	GENERIC_COM_STRUCT Args;
	INT Dummy;
	DWORD64 size;

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
	if (!DeviceIoControl(Handle, CTL_CODE_HW, &Args, sizeof(Args), NULL, 0, &Dummy, NULL))
	{
		hvPrint(L"DeviceIoControl failed: %d\n", GetLastError());
		return HYPERWIN_IOCTL_FAILED;
	}
	return HYPERWIN_STATUS_SUCCUESS;
}