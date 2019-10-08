#include "../phase1/pcb.c"
#include "../phase1/asl.c"
#include "../h/types.h"
#include "../h/const.h"
#include "exceptions.c"
#include "scheduler.c"

/* global variables */
int processCount;
int softBlockCount;
pcb_PTR currentProcess;
pcb_PTR readyQue;
cpu_t *time;
int waitFlag;
int devSemTable[DEVICESEMNUM];

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
  for(i=0; i<DEVICESEMNUM; i++){
    (devSemTable[i])->s_semAdd = i+1;
    (*(devSemTable[i])->s_semAdd) = 0;
  }



  pcb_PTR p = allocPcb();
  p->p_s.s_sp = ramTop - PAGESIZE;
  p->p_s.s_pc = (memaddr) phase2testmain;
  p->p_s.s_status = FIRSTPROCESSSTATUSMASK;

  processCount++;
  insertProcQ(&readyQue, p);
  scheduler();
}