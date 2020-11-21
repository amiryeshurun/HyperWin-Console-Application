#ifndef __COMMUNICATION_OPS_H_
#define __COMMUNICATION_OPS_H_

#include "hwtypes.h"

HWSTATUS SendInitSignal(IN HANDLE Handle, IN PGENERIC_COM_STRUCT Args);
HWSTATUS MarkProcessProtected(IN HANDLE Handle, IN HANDLE ProcessHandle);

#endif