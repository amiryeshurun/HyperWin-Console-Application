#include "utils.h"
#include "comops.h"
#include "hwstatus.h"
#include "progops.h"

INT main()
{
    /*
    HANDLE Handle = CreateFileA("\\\\.\\HyperWin", GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE,
        NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

    if (Handle == INVALID_HANDLE_VALUE)
    {
        hvPrint("CreateFile failed: %d\n", GetLastError());
        return HYPERWIN_CREATE_FAILED;
    }
    GENERIC_COM_STRUCT Request;
    if (MarkProcessProtected(Handle, &Request) != HYPERWIN_STATUS_SUCCUESS)
        return HYPERWIN_INIT_FAILED;
    */
    ProgramLoop(NULL);
    return 0;
}