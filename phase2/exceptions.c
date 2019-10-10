#ifndef EXCEPTIONS
#define EXCEPTIONS

#include "../e/pcb.e"
#include "../e/asl.e"
#include "../h/types.h"
#include "../h/const.h"
#include "../e/initial.e"

void pgmTrapHandler(){
  if (currentProcess->p_oldPgm == NULL){
      terminateProcess();
  }
  else {
    state_t *globalPgmOld = (state_t *) PROGRAMTRAPOLDAREA;
    stateCopy(globalPgmOld, currentProcess->p_oldPgm);

    LDST(&(currentProcess->p_newPgm));
  }
}

void tlbMgmtHandler(){
  if (currentProcess->p_oldTLB == NULL){
      terminateProcess();
  }
  else {
    state_t *globalTLBOld = (state_t *) TLBMANAGEMENTOLDAREA;
    stateCopy(globalTLBOld, currentProcess->p_oldTLB);

    LDST(&(currentProcess->p_newTLB));
  }
}

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
  syscallDispatch(syscallNum, kernelMode);
}

void syscallDispatch(int syscallNum, int kernelMode){
  if(1<= syscallNum <= 8){
    if(kernelMode == TRUE) {
      switch(syscallNum){
        case 1:
          createProcess(); /* done */
        case 2:
          terminateProcess(); /* done */
        case 3:
          verhogen(); /* done */
        case 4:
          passeren(); /* done */
        case 5:
          specifyExceptionStateVector(); /* done */
        case 6:
          getCPUTime(); /* done */
        case 7:
          waitForClock();
        case 8:
          waitForIODevice(); /* done */
      }
    }
    else { /* syscall 1-8 user mode, make it look like a priveleged instruction error */
      state_t *syscallOld = (state_t *) SYSCALLOLDAREA;
      state_t *pgmOld = (state_t *) PROGRAMTRAPOLDAREA;
      stateCopy(syscallOld, pgmOld);

      unsigned int* cause = &(pgmOld->s_cause);  /*fucky bitwise ops that could go wrong */
      *cause = *cause | RIMASKON;
      unsigned int finalCause = ~(*cause) | RIMASKTOTURNOFF;
      *cause = ~finalCause;

      pgmTrapHandler();
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
    insertChild(currentProcess, p);
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
    softBlockCount--;
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

void stateCopyPCB(pcb_PTR p, state_PTR s){
  p->p_s.s_asid = s->s_asid;
  p->p_s.s_cause = s->s_cause;
  p->p_s.s_status = s->s_status;
  p->p_s.s_pc = s->s_pc;

  int i = 0;
  for (i; i < STATEREGNUM; i++) {
    p->p_s.s_reg[i] = s->s_reg[i];
  }
}

void stateCopy(state_PTR old, state_PTR new){
  new->s_asid = old->s_asid;
  new->s_cause = old->s_cause;
  new->s_status = old->s_status;
  new->s_pc = old->s_pc;
  
  int i = 0;
  for (i; i < STATEREGNUM; i++) {
    new->s_reg[i] = old->s_reg[i];
  }
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
      currentProcess->p_oldTLB = oldState;
      currentProcess->p_newTLB = newState;
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
    if (currentProcess->p_oldTLB != NULL) {
      oldState = (state_t *) TLBMANAGEMENTOLDAREA;
      currentProcess->p_oldTLB = oldState;
      LDST(currentProcess->p_newTLB);
    }
  }
  else if (exceptionType == 1) {
    if (currentProcess->p_oldPgm != NULL) {
      oldState = (state_t *) PROGRAMTRAPOLDAREA;
      currentProcess->p_oldPgm = oldState;
      LDST(currentProcess->p_newPgm);
    }
  }
  else if (exceptionType == 2) {
    if (currentProcess->p_oldSys != NULL) {
      oldState = (state_t *) SYSCALLOLDAREA;
      currentProcess->p_oldSys = oldState;
      LDST(currentProcess->p_newSys);
    }
  }
  /* sys 5 hasnt been called before, kill it */
  terminateProcess();  
}

void verhogen(){
  state_t *oldSys = (state_t *) SYSCALLOLDAREA;
  int *mutex = oldSys->s_a1;
  *mutex++;
  if (mutex <= 0){
    pcb_PTR temp = removeBlocked(&mutex);
    insertProcQ(&readyQue, temp);
  }
  LDST(&oldSys);
}

void passeren(){
  state_t *oldSys = (state_t *) SYSCALLOLDAREA;
  int mutex = oldSys->s_a1;
  mutex--;
  if (mutex < 0){
    insertBlocked(&mutex, currentProcess);

    cpu_t currTime = NULL;
    STCK(currTime);
    currentProcess->p_time = currentProcess->p_time + (currTime - (*time));

    currentProcess = NULL;
    scheduler();
  }
  LDST(&oldSys);
}

getCPUTime(){
  state_t *oldSys = (state_t *) SYSCALLOLDAREA;
  
  cpu_t currTime = NULL;
  STCK(currTime);
  currentProcess->p_time = currentProcess->p_time + (currTime - (*time));

  oldSys->s_v0 = currentProcess->p_time;

  LDST(&oldSys);
}

waitForClock(){
  state_t *oldSys = (state_t *) SYSCALLOLDAREA;
  int semAdd = devSemTable[EIGHTDEVLINES * DEVSPERLINE + DEVSPERLINE];
  semAdd--;
  if (semAdd < 0){
    insertBlocked(&semAdd, currentProcess);

    cpu_t currTime = NULL;
    STCK(currTime);
    currentProcess->p_time = currentProcess->p_time + (currTime - (*time));

    currentProcess = NULL;
    scheduler();
  }
  LDST(&oldSys);
}



void waitForIODevice(){
  state_t *oldSys = (state_t *) SYSCALLOLDAREA;
  int lineNumber = oldSys->s_a1;
  int deviceNumber = oldSys->s_a2;
  int termRead = oldSys->s_a3;
  int * semAdd = devSemTable[getSemArrayNum(lineNumber, deviceNumber)];
  semAdd--;
  if (semAdd < 0) {
    cpu_t currTime = NULL;
    STCK(currTime);
    currentProcess->p_time = currentProcess->p_time + (currTime - (*time));
    softBlockCount++;
    insertBlocked(semAdd, currentProcess);
    currentProcess = NULL;
    scheduler();
  }
  /* error */
}

#endif
