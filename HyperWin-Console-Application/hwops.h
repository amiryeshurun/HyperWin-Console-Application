#ifndef __HYPERWIN_OPERATIONS_H_
#define __HYPERWIN_OPERATIONS_H_

#include <windows.h>

/* IOCTL different codes*/
#define CTL_CODE_HW CTL_CODE(40000, 0x800, METHOD_BUFFERED, GENERIC_READ | GENERIC_WRITE)

/* Communication block different operations */

#define OPERATION_INIT				0x4857494e4954 // HWINIT ASCII
#define OPERATION_PROTECTED_PROCESS 0x70726f74656374  // PROTECT ASCII

#endif