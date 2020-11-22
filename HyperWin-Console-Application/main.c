#include "utils.h"
#include "comops.h"
#include "hwstatus.h"
#include "progops.h"

INT main()
{
    HANDLE Handle;
    if ((Handle = CreateFileA("\\\\.\\HyperWin",
        GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL)) == INVALID_HANDLE_VALUE)
    {
        hvPrint("CreateFile failed: %d\n", GetLastError());
        return HYPERWIN_CREATE_FAILED;
    }

    ProgramLoop(Handle);
    return 0;
}