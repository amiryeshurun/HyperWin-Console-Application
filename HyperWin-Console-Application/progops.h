#ifndef __HYPERWIN_PROG_OPS_H_
#define __HYPERWIN_PROG_OPS_H_

#include "hwtypes.h"

VOID ProgramLoop(IN HANDLE CommunicationDriver);
VOID HandleProtectProcess(IN HANDLE CommunicationDriver);
VOID HandleGetProcess();
VOID HandleProtectFileData(IN HANDLE CommunicationDriver);
VOID HandleGetFileId();

#endif