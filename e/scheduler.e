#ifndef SCHEDULER
#define SCHEDULER

#include "../e/initial.e"
#include "../e/pcb.e"
#include "../e/asl.e"
#include "../h/types.h"
#include "../h/const.h"
#include "/usr/local/include/umps2/umps/libumps.e"

extern state_t * waitState;

extern void scheduler();
extern void wait();

#endif
