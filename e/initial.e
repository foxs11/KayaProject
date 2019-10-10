#include "../e/pcb.e"
#include "../e/asl.e"
#include "../h/types.h"
#include "../h/const.h"
#include "../e/exceptions.e"
#include "../e/scheduler.e"

/* global variables */
extern int processCount;
extern int softBlockCount;
extern pcb_PTR currentProcess;
extern pcb_PTR readyQue;
extern cpu_t *time;
extern int devSemTable[];