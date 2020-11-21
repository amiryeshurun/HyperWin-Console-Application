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
		hvPrint("DeviceIoControl failed: %d\n", GetLastError());
		return HYPERWIN_IOCTL_FAILED;
	}
	hvPrint("Operation complete\n");
	return HYPERWIN_STATUS_SUCCUESS;
}

HWSTATUS MarkProcessProtected(IN HANDLE Handle, IN HANDLE ProcessHandle)
{
	GENERIC_COM_STRUCT Args;
	Args.Operation = OPERATION_PROTECTED_PROCESS;
	INT Dummy;
	if (!DeviceIoControl(Handle, CTL_CODE_HW, &Args, sizeof(Args), NULL, 0, &Dummy, NULL))
	{
		hvPrint("DeviceIoControl failed: %d\n", GetLastError());
		return HYPERWIN_IOCTL_FAILED;
	}
	hvPrint("Successfully sent a request to mark process as protected\n");
	return HYPERWIN_STATUS_SUCCUESS;
}