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

void terminateRecursively(pcb_PTR processToKill) { /* handle 2 device/not device asl cases from video */
  while (!emptyChild(processToKill)) {
    terminateRecursively(removeChild(processToKill));
  }
  if (processToKill->p_semAdd != NULL) { /* on ASL */
    freePcb(outBlocked(processToKill));
    processCount--;
    softBlockedCount--;
  }
  else if (processToKill == currentProcess) { /* current proc */
    freePcb(outChild(processToKill));
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

void specifyExceptionStateVector(){
  

  state_t *syscallOld = (state_t *) SYSCALLOLDAREA;
  int exceptionType = syscallOld->s_a1;
  state_PTR oldState = syscallOld->s_a2;
  state_PTR newState = syscallOld->s_a3;

  if (exceptionType == 0){
    if (currentProcess->p_oldTLB != NULL){
      terminateProcess();
    }
    else {
      currentProcess->p_oldTBL = oldState;
      currentProcess->p_newTBL = newState;
    }
  }
  else if (exceptionType == 1){
    if (currentProcess->p_oldPgm != NULL){
      terminateProcess();
    }
    else {
      currentProcess->p_oldPgm = oldState;
      currentProcess->p_newPgm = newState;
    }
  }
  else if (exceptionType == 2){
    if (currentProcess->p_oldSys != NULL){
      terminateProcess();
    }
    else {
      currentProcess->p_oldSys = oldState;
      currentProcess->p_newSys = newState;
    }
  }

  LDST(&syscallOld);
}

void passUpOrDie(int exceptionType){
  state_PTR oldState = NULL;
  if (exceptionType == 0) {
    if (currentProcess->oldTLB != NULL) {
      oldState = (state_t *) TLBMANAGEMENTOLDAREA;
      currentProcess->oldTLB = oldState;
      LDST(currentProcess->newTLB);
    }
  }
  else if (exceptionType == 1) {
    if (currentProcess->oldPgm != NULL) {
      oldState = (state_t *) PROGRAMTRAPOLDAREA;
      currentProcess->oldPgm = oldState;
      LDST(currentProcess->newPgm);
    }
  }
  else if (exceptionType == 2) {
    if (currentProcess->oldSys != NULL) {
      oldState = (state_t *) SYSCALLOLDAREA;
      currentProcess->oldSys = oldState;
      LDST(currentProcess->newSys);
    }
  }
  /* sys 5 hasnt been called before, kill it */
  terminateProcess();

  
}
