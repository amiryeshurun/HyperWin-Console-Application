#ifndef __HYPERWIN_PROG_OPS_H_
#define __HYPERWIN_PROG_OPS_H_

#include "hwtypes.h"

VOID ProgramLoop(IN HANDLE CommunicationDriver);
VOID HandleProtectProcess(IN HANDLE CommunicationDriver, IN PCHAR Buffer);
VOID HandleGetProcess();

#endif