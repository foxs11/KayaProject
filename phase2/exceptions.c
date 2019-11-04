#ifndef EXCEPTIONS
#define EXCEPTIONS

#include "../e/pcb.e"
#include "../e/asl.e"
#include "../h/types.h"
#include "../h/const.h"
#include "../e/initial.e"

void aDebug(unsigned int a, unsigned int b, unsigned int c) {
  int i = 0;
}

void pgmTrapHandler(){
  state_t *pgmOld = (state_t *) PROGRAMTRAPOLDAREA;
  unsigned int cause = pgmOld->s_cause;
  unsigned int status = pgmOld->s_status;
  unsigned int sp = pgmOld->s_sp;
  devregarea_t *foo = (devregarea_t *) RAMBASEADDR;
  unsigned int ramTop = foo->rambase + foo->ramsize;
  passUpOrDie(1);
}

void tlbMgmtHandler(){
  passUpOrDie(0);
}

void sysCallHandler(){
  state_t *syscallOld = (state_t *) SYSCALLOLDAREA;
  syscallOld->s_pc = syscallOld->s_pc + 4;
  
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
  if(syscallNum > 0 && syscallNum < 9){
    if(kernelMode == TRUE) {
      switch(syscallNum){
        case 1:
          addokbuf("about to enter 1\n");
          createProcess(); /* done */
          break;
        case 2:
        addokbuf("about to enter 2\n");
          terminateProcess(); /* done */
          break;
        case 3:
        addokbuf("about to enter 3\n");
          verhogen(); /* done */
          break;
        case 4:
        addokbuf("about to enter 4\n");
          passeren(); /* done */
          break;
        case 5:
        addokbuf("about to enter 5\n");
          specifyExceptionStateVector(); /* done */
          break;
        case 6:
        addokbuf("about to enter 6\n");
          getCPUTime(); /* done */
          break;
        case 7:
        addokbuf("about to enter 7\n");
          waitForClock();
          break;
        case 8:
          addokbuf("about to enter 8\n");
          waitForIODevice(); /* done */
          break;
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
    passUpOrDie(2); /* no parameter???? */
  }
}

void createProcess(){
  state_t *syscallOld = (state_t *) SYSCALLOLDAREA;
  state_PTR newProcState = syscallOld->s_a1;
  pcb_PTR p = allocPcb();
  if (p==NULL) {
    addokbuf("p is null");
    syscallOld->s_v0 = -1;
    return;
  }
  else { /* pcb allocated */
    stateCopyPCB(p, newProcState);
    processCount++;
    insertChild(currentProcess, p);
    insertProcQ(&readyQue, p);
    syscallOld->s_v0 = 0;
    LDST(syscallOld);
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
    if (processToKill->p_semAdd == (&(devSemTable[EIGHTDEVLINES * DEVSPERLINE + DEVSPERLINE]))) {
      (*processToKill->p_semAdd)++;
    }
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

  LDST(syscallOld);
}

void passUpOrDie(int exceptionType){
  if(currentProcess == NULL){
  }
  state_PTR oldState = NULL;
  if (exceptionType == 0) {
    if (currentProcess->p_oldTLB != NULL) {
      oldState = (state_t *) TLBMANAGEMENTOLDAREA;
      stateCopy(oldState, currentProcess->p_oldTLB);
      LDST(currentProcess->p_newTLB);
    }
  }
  else if (exceptionType == 1) {
    if (currentProcess->p_oldPgm != NULL) {
      oldState = (state_t *) PROGRAMTRAPOLDAREA;
      stateCopy(oldState, currentProcess->p_oldPgm);
      LDST(currentProcess->p_newPgm);
    }
  }
  else if (exceptionType == 2) {
    if (currentProcess->p_oldSys != NULL) {
      oldState = (state_t *) SYSCALLOLDAREA;
      stateCopy(oldState, currentProcess->p_oldSys);
      LDST(currentProcess->p_newSys);
    }
  }
  /* sys 5 hasnt been called before, kill it */
  terminateProcess();  
}

void verhogen(){
  state_t *oldSys = (state_t *) SYSCALLOLDAREA;
  int *mutex = oldSys->s_a1;
  (*mutex)++;
  if (*mutex <= 0){
    pcb_PTR temp = removeBlocked(&mutex);
    softBlockCount--;
    if(temp != NULL){
    }
    insertProcQ(&readyQue, temp);
  }
  LDST(oldSys);
}

void passeren(){
  state_t *oldSys = (state_t *) SYSCALLOLDAREA;
  int * mutex = oldSys->s_a1;
  (*mutex)--;
  if (*mutex < 0){
    cpu_t currTime = 0;
    STCK(currTime);
    currentProcess->p_time = currentProcess->p_time + (currTime - (time));
    insertBlocked(&mutex, currentProcess);
    softBlockCount++;
    currentProcess = NULL;
    scheduler();
  }
  LDST(oldSys);
}

getCPUTime(){
  state_t *oldSys = (state_t *) SYSCALLOLDAREA;
  
  cpu_t currTime = 0;
  STCK(currTime);
  currentProcess->p_time = currentProcess->p_time + (currTime - (time));

  oldSys->s_v0 = currentProcess->p_time;

  LDST(oldSys);
}

waitForClock(){
  addokbuf("waitForClock 1\n");
  state_t *oldSys = (state_t *) SYSCALLOLDAREA;
  int * semAdd = &(devSemTable[EIGHTDEVLINES * DEVSPERLINE + DEVSPERLINE]);
  (*semAdd)--;
  if ((*semAdd) < 0){
    if(currentProcess == NULL){
      addokbuf("current process is null\n");
    }
    addokbuf("waitForClock 2\n");
    insertBlocked(semAdd, currentProcess);
    addokbuf("waitForClock 3\n");
    softBlockCount++;
    addokbuf("waitForClock 4\n");
    cpu_t currTime = 0;
    addokbuf("waitForClock 5\n");
    STCK(currTime);
    addokbuf("waitForClock 6\n");
    currentProcess->p_time = currentProcess->p_time + (currTime - (time));
    addokbuf("waitForClock 7\n");
    currentProcess = NULL;
    addokbuf("waitForClock 8\n");
    scheduler();
  }
  addokbuf("waitForClock 9\n");
  LDST(oldSys);
}



void waitForIODevice(){
  state_t *oldSys = (state_t *) SYSCALLOLDAREA;
  stateCopy(oldSys, &(currentProcess->p_s));
  unsigned int pc = oldSys->s_pc;
  int lineNumber = oldSys->s_a1;
  int deviceNumber = oldSys->s_a2;
  int termRead = oldSys->s_a3;
  int offset = 0;
  if (lineNumber == 7) {
    if (termRead == TRUE) {
      offset = 8;
    }
  }
  int semNumber = getSemArrayNum(lineNumber, deviceNumber, offset);
  int * semAdd = &(devSemTable[semNumber]);
  (*semAdd)--;
  if ((*semAdd) < 0) {
    cpu_t currTime = 0;
    STCK(currTime);
    currentProcess->p_time = currentProcess->p_time + (currTime - (time));
    softBlockCount++;
    insertBlocked(semAdd, currentProcess);
    currentProcess = NULL;
    scheduler();
  }
  /* error */
}

#endif
