#include "../phase1/pcb.c"
#include "../phase1/asl.c"
#include "../h/types.h"
#include "../h/const.h"

void pgmTrapHandler(){}
void tlbMgmtHandler(){}
void sysCallHandler(){
  state_t *syscallOld = (state_t *) SYSCALLOLDAREA;
  int syscallNum = syscallOld->s_a0;
  int kernelMode;
  int kernelStatus = syscallOld->s_status & KERNELOFF;
  if(kernelStatus == ALLOFF){
   kernelMode = TRUE;
  }
  else{
    kernelMode = FALSE;
  }
  if(kernelMode){
    syscallDispatch(syscallNum);
  }
  else{
    //TODO: make program trap for this else
  }
}

void syscallDispatch(int syscallNum){
  if(1<= syscallNum <= 8){
    switch(syscallNum){
      case 1:
        createProcess();
      case 2:
        terminateProcess();
      case 3:
        verhogen();
      case 4:
        passeren();
      case 5:
        specifyExceptionStateVector();
      case 6:
        getCPUTime();
      case 7:
        waitForClock();
      case 8:
        waitForIODevice();
    }
  }
  else{
    passUpOrDie();
  }
}

void createProcess(){
  state_t *syscallOld = (state_t *) SYSCALLOLDAREA;
  state_PTR newProcState = syscallOld->s_a1;
  syscallOld->s_v0 = -1;
  pcb_PTR p = allocPcb();
  if (p==NULL) {
    syscallOld->s_v0 = -1;
    return;
  }
  else { /* pcb allocated */
    stateCopy(p, newProcState);
    processCount++;
    insertChild(currentProc, p);
    insertProcQ(&readyQue, p);
    syscallOld->s_v0 = 1;
    LDST(&syscallOld);
  }
}

void terminateProcess(){
  terminateRecursively(currentProcess);
  scheduler();
}

void terminateRecursively(pcb_PTR processToKill) {
  if (!emptyChild(processToKill)) {
    terminateRecursively(processToKill->p_child);
  }
  if (processToKill->p_semAdd != NULL) { /* on ASL */
    freePcb(outBlocked(processToKill));
    processCount--;
    softBlockedCount--;
  }
  else if (processToKill == currentProcess) { /* current proc */
    freePcb(processToKill);
    processCount--;
  }
  else { /* pcb is on readyQue */
    freePcb(outProcQ(&readyQue, processToKill));
    processCount--;
  }
}

void stateCopy(pcb_PTR p, state_PTR s){
  p->p_s.s_asid = s->s_asid;
  p->p_s.s_cause = s->s_cause;
  p->p_s.s_status = s->s_status;
  p->p_s.s_pc = s->s_pc;
  p->p_s.s_reg = s->s_reg;
}
