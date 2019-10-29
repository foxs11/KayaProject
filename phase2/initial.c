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
#define clockSem devSemTable[EIGHTDEVLINES * DEVSPERLINE + DEVSPERLINE];


extern void test();

void main(){
  devregarea_t *foo = (devregarea_t *) RAMBASEADDR;
  int ramTop = foo->rambase + foo->ramsize;

  state_t * sysCallNew = (state_t *) SYSCALLNEWAREA;
  state_t * pgmTrapNew = (state_t *) PROGRAMTRAPNEWAREA;
  state_t * tlbMgmtNew = (state_t *) TLBMANAGEMENTNEWAREA;
  state_t * interruptNew = (state_t *) INTERRUPTNEWAREA;

  sysCallNew->s_sp = ramTop;
  pgmTrapNew->s_sp = ramTop;
  tlbMgmtNew->s_sp = ramTop;
  interruptNew->s_sp = ramTop;

  sysCallNew->s_pc = (memaddr) sysCallHandler;
  pgmTrapNew->s_pc = (memaddr) pgmTrapHandler;
  tlbMgmtNew->s_pc = (memaddr) tlbMgmtHandler;
  interruptNew->s_pc = (memaddr) interruptHandler;

  sysCallNew->s_t9 = (memaddr) sysCallHandler;
  pgmTrapNew->s_t9 = (memaddr) pgmTrapHandler;
  tlbMgmtNew->s_t9 = (memaddr) tlbMgmtHandler;
  interruptNew->s_t9 = (memaddr) interruptHandler;

  sysCallNew->s_status = INTSMASKED | VMOFF | PROCLOCALTIMEON | KERNELON;
  pgmTrapNew->s_status = INTSMASKED | VMOFF | PROCLOCALTIMEON | KERNELON;
  tlbMgmtNew->s_status = INTSMASKED | VMOFF | PROCLOCALTIMEON | KERNELON;
  interruptNew->s_status = INTSMASKED | VMOFF | PROCLOCALTIMEON | KERNELON;

  processCount = 0;
  softBlockCount = 0;
  currentProcess = NULL;
  readyQue = mkEmptyProcQ();

  initPcbs();
  initASL();

  int i;
  for(i=0; i< (EIGHTDEVLINES * DEVSPERLINE + DEVSPERLINE + 1); i++){
    devSemTable[i] = 0;
  }
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
