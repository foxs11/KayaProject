#ifndef EXCEPTIONS
#define EXCEPTIONS

#include "../e/pcb.e"
#include "../e/asl.e"
#include "../h/types.h"
#include "../h/const.h"
#include "../e/initial.e"

extern void pgmTrapHandler();
extern void tlbMgmtHandler();
extern void sysCallHandler();

#endif
