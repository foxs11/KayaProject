#include "../phase1/pcb.c"
#include "../phase1/asl.c"
#include "../h/types.h"
#include "../h/const.h"
#include "exceptions.c"
#include "scheduler.c"

/* global variables */
extern int processCount;
extern int softBlockCount;
extern pcb_PTR currentProcess;
extern pcb_PTR readyQue;
extern cpu_t *time;
extern semd_PTR devSemTable[];