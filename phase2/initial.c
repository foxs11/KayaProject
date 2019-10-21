#ifndef INITIAL
#define INITIAL

#include "../e/pcb.e"
#include "../e/asl.e"
#include "../h/types.h"
#include "../h/const.h"
#include "../e/exceptions.e"
#include "../e/scheduler.e"
#include "../e/interrupts.e"

/* global variables */
int processCount;
int softBlockCount;
pcb_PTR currentProcess;
pcb_PTR readyQue;
cpu_t time;
int waitFlag;
int devSemTable[EIGHTDEVLINES * DEVSPERLINE + DEVSPERLINE + 1]; /*compute number differently */
state_t * waitState;

extern void test();

void main(){
  addokbuf("in initial 1\n");
  devregarea_t *foo = (devregarea_t *) RAMBASEADDR;
  addokbuf("in initial 2\n");
  int ramTop = foo->rambase + foo->ramsize;
  addokbuf("in initial 3\n");

  state_t * sysCallNew = (state_t *) SYSCALLNEWAREA;
  addokbuf("in initial 4\n");
  state_t * pgmTrapNew = (state_t *) PROGRAMTRAPNEWAREA;
  addokbuf("in initial 5\n");
  state_t * tlbMgmtNew = (state_t *) TLBMANAGEMENTNEWAREA;
  addokbuf("in initial 6\n");
  state_t * interruptNew = (state_t *) INTERRUPTNEWAREA;
  addokbuf("in initial 7\n");

  sysCallNew->s_sp = ramTop;
  addokbuf("in initial 8\n");
  pgmTrapNew->s_sp = ramTop;
  addokbuf("in initial 9\n");
  tlbMgmtNew->s_sp = ramTop;
  addokbuf("in initial 10\n");
  interruptNew->s_sp = ramTop;
  addokbuf("in initial 11\n");
  waitState->s_sp = ramTop;

  sysCallNew->s_pc = (memaddr) sysCallHandler;
  addokbuf("in initial 12\n");
  pgmTrapNew->s_pc = (memaddr) pgmTrapHandler;
  addokbuf("in initial 13\n");
  tlbMgmtNew->s_pc = (memaddr) tlbMgmtHandler;
  addokbuf("in initial 14\n");
  interruptNew->s_pc = (memaddr) interruptHandler;
  addokbuf("in initial 15\n");
  waitState->s_pc = (memaddr) wait;

  sysCallNew->s_t9 = (memaddr) sysCallHandler;
  addokbuf("in initial 16\n");
  pgmTrapNew->s_t9 = (memaddr) pgmTrapHandler;
  addokbuf("in initial 17\n");
  tlbMgmtNew->s_t9 = (memaddr) tlbMgmtHandler;
  addokbuf("in initial 18\n");
  interruptNew->s_t9 = (memaddr) interruptHandler;
  addokbuf("in initial 19\n");
  waitState->s_t9 = (memaddr) wait;
  addokbuf("in initial 20\n");

  sysCallNew->s_status = INTSMASKED | VMOFF | PROCLOCALTIMEON | KERNELON;
  addokbuf("in initial 21\n");
  pgmTrapNew->s_status = INTSMASKED | VMOFF | PROCLOCALTIMEON | KERNELON;
  addokbuf("in initial 22\n");
  tlbMgmtNew->s_status = INTSMASKED | VMOFF | PROCLOCALTIMEON | KERNELON;
  addokbuf("in initial 23\n");
  interruptNew->s_status = INTSMASKED | VMOFF | PROCLOCALTIMEON | KERNELON;
  addokbuf("in initial 24\n");
  waitState->s_status = SCHEDULERINTSUNMASKED | PROCLOCALTIMEON;

  processCount = 0;
  addokbuf("in initial 25\n");
  softBlockCount = 0;
  addokbuf("in initial 26\n");
  currentProcess = NULL;
  addokbuf("in initial 27\n");
  readyQue = mkEmptyProcQ();

  addokbuf("in initial 28\n");
  initPcbs();
  addokbuf("in initial 29\n");
  initASL();

  int i;
  for(i=0; i< (EIGHTDEVLINES * DEVSPERLINE + DEVSPERLINE + 1); i++){
    devSemTable[i] = 0;
  }
  addokbuf("in initial 30\n");
  pcb_PTR p = allocPcb();
  p->p_s.s_sp = ramTop - PAGESIZE;
  p->p_s.s_pc = (memaddr) test;
  p->p_s.s_t9 = (memaddr) test;
  p->p_s.s_status = FIRSTPROCESSSTATUSMASK;

  processCount++;
  insertProcQ(&readyQue, p);

  LDIT(100000);

  time = 0;

  scheduler();
}

#endif
