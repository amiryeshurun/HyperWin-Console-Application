#ifndef __HYPERWIN_PROG_OPS_H_
#define __HYPERWIN_PROG_OPS_H_

#include "hwtypes.h"

VOID ProgramLoop(IN HANDLE CommunicationDriver);
HWSTATUS HandleProtectProcess(IN HANDLE CommunicationDriver, IN PWCHAR* Tokens);
HWSTATUS HandleGetProcess();
HWSTATUS HandleProtectFileData(IN HANDLE CommunicationDriver, IN PWCHAR* Tokens);
HWSTATUS HandleGetFileId(IN PWCHAR* Tokens);
HWSTATUS HandleRemoveProtection(IN HANDLE CommunicationDriver, IN PWCHAR* Tokens);
HWSTATUS HandleCreateGroup(IN HANDLE CommunicationDriver, IN PWCHAR* Tokens);
HWSTATUS DispatchCommand(IN HANDLE CommunicationDriver, IN PWCHAR* Tokens);

#endif