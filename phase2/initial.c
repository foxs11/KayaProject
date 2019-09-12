#include "../phase1/pcb.c"
#include "../phase1/asl.c"
#include "../h/types.h"
#include "../h/const.h"

/* global variables */
int processCount;
int softBlockCount;
pcb_PTR currentProcess;
pcb_PTR readyQue;

void main(){
  devregarea_t *foo = (devregarea_t *) RAMBASEADDR;
  int ramTop = foo->rambase + foo->ramsize;

  state_t * sysCallNew = (state_t *) SYSCALLNEWAREA;
  state_t * sysCallOld = (state_t *) SYSCALLOLDAREA;
  state_t * pgmTrapNew = (state_t *) PROGRAMTRAPNEWAREA;
  state_t * pgmTrapOld = (state_t *) PROGRAMTRAPOLDAREA;
  state_t * tlbMgmtNew = (state_t *) TLBMANAGEMENTNEWAREA;
  state_t * tlbMgmtOld = (state_t *) TLBMANAGEMENTOLDAREA;
  state_t * interruptNew = (state_t *) INTERRUPTNEWAREA;
  state_t * interruptOld = (state_t *) INTERRUPTOLDAREA;

  /* TODO: set pc for each of the stat_t variables */
  sysCallNew->s_sp = ramTop;
  sysCallOld->s_sp = ramTop;
  pgmTrapNew->s_sp = ramTop;
  pgmTrapOld->s_sp = ramTop;
  tlbMgmtNew->s_sp = ramTop;
  tlbMgmtOld->s_sp = ramTop;
  interruptNew->s_sp = ramTop;
  interruptOld->s_sp = ramTop;

  sysCallNew->s_pc = (memaddr) sysCallHandler;
  pgmTrapNew->s_pc = (memaddr) pgmTrapHandler;
  tlbMgmtNew->s_pc = (memaddr) tlbMgmtHandler;
  interruptNew->s_pc = (memaddr) interruptHandler;

  sysCallNew->s_status = STATUSMASK;
  pgmTrapNew->s_status = STATUSMASK;
  tlbMgmtNew->s_status = STATUSMASK;
  interruptNew->s_status = STATUSMASK;

  processCount = 0;
  softBlockCount = 0;
  currentProcess = NULL;
  readyQue = mkEmptyProcQ();

  initPcbs();
  initASL();
}