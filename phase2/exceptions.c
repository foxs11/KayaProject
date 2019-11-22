#ifndef EXCEPTIONS
#define EXCEPTIONS

/***************** EXCEPTIONS.C *******************
 * 
 * provides all methodes and functions for program traps, tlb management, and
 * sysCalls 1-8. 
 * 
*/

#include "../e/pcb.e"
#include "../e/asl.e"
#include "../h/types.h"
#include "../h/const.h"
#include "../e/initial.e"

/* SYS 1
*  Gets process state from SYSCALLOLDAREA. Call allocPcb(),
*  if there are no more pcbs available, store off -1 in v0 and return
*  if successful allocation, copy state into new PCB,
*  update process count, insert process as a child onto currentProcess,
*  insert new process on readyQue, load state of old state */
void createProcess(){
  state_t *syscallOld = (state_t *) SYSCALLOLDAREA;
  state_PTR newProcState = syscallOld->s_a1;
  pcb_PTR p = allocPcb();
  if (p==NULL) {
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

/* SYS 2
*  call termination function then call scheduler */
void terminateProcess(){
  terminateRecursively(currentProcess);
  scheduler();
}

/* Takes a pointer to the process the needs to be terminated
*  First, it goes and finds the lowest child of the procosses on the process tree.
*  Then, checks to see if process is being blocked by a device semaphore,
*  If yes, updates semaphore then removes process from it.
*  If not on a device semaphore, checks if it is current process,
*  if yes, calls freePCB on current process. If not current process,
*  its on the readyQue so then removes from readyQue and freesPCB. 
*  Does all of this for every child of the current process and all of the 
*  childerns childrens until at the bottom of the tree starting at the bottom
*  child */
void terminateRecursively(pcb_PTR processToKill) { /* handle 2 device/not device asl cases from video */
  int i;
  int devSem;
  devSem = FALSE;
  i = 0;

  /* This is where it gets the bottom of the children on the tree
  *  and recursively kills all the rest on the way up */
  while (emptyChild(processToKill) == FALSE) {
    pcb_PTR nextProcessToKill=removeChild(processToKill);
    terminateRecursively(nextProcessToKill);
  }
  if (processToKill->p_semAdd != NULL) { /* on ASL */
    while (i < 49){
      if (&(devSemTable[i]) == processToKill->p_semAdd){
        devSem = TRUE; /* process is blocked on a device semaphore */
      }
      i++;
    }
    if (devSem == FALSE){
      (*(processToKill->p_semAdd))++;
    }
    else{
      softBlockCount--;
    }
    freePcb(outBlocked(processToKill));
    processCount--;
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

/* State copy function that takes a pointer to a PCB and a
*  pointer to a state as parameters */
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

/* State copy function that takes a pointer to a state and another
*  pointer to a state as perameters. */
extern void stateCopy(state_PTR old, state_PTR new){
  new->s_asid = old->s_asid;
  new->s_cause = old->s_cause;
  new->s_status = old->s_status;
  new->s_pc = old->s_pc;
  
  int i = 0;
  for (i; i < STATEREGNUM; i++) {
    new->s_reg[i] = old->s_reg[i];
  } 
}

/* SYS 5
*  Gets the state from SYSCALLOLDAREA and gets the exception type,
*  the address where the old process state should be stored in case of
*  an exception, and the process state that should be loaded into the 
*  process if and exception occurs. If exception type is 0 and hasn't been
*  called on this processes yet, update new and old TLB with new and old state,
*  else, terminate process. If exception type 1 and hasn't been called on this 
*  process yet, update new and old PGM withe new and old state, else,
*  terminate process. If exception type 2 and hasn't been called on this process
*  yet, update new and old sys with new and old state, else, terminate process.
*  Finally, load state with the state from SYSCALLOLDAREA */
void specifyExceptionStateVector(){
  state_t *syscallOld = (state_t *) SYSCALLOLDAREA;
  int exceptionType = syscallOld->s_a1;
  state_PTR oldState = syscallOld->s_a2;
  state_PTR newState = syscallOld->s_a3;

  if (exceptionType == 0){
    /* SYS 5 has already been called for exception type 0 */
    if (currentProcess->p_oldTLB != NULL){ 
      terminateProcess();
    }
    else {
      currentProcess->p_oldTLB = oldState;
      currentProcess->p_newTLB = newState;
    }
  }
  else if (exceptionType == 1){
    /* SYS 5 has already been called for exception type 1 */
    if (currentProcess->p_oldPgm != NULL){
      terminateProcess();
    }
    else {
      currentProcess->p_oldPgm = oldState;
      currentProcess->p_newPgm = newState;
    }
  }
  else if (exceptionType == 2){
    /* SYS 5 has already been called for exception type 2 */
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

/* takes an exception type as parameter
*  If exceptionType = 0 and current process has a tlb handler,
*  load state of the tlbHandler
*  If exceptionType = 1 and current process has a pgm handler,
*  load state of the pgmHandler
*  It exceptionTpye = 2 and current process has a sys handler,
*  load state of the sysHandler
*  else, terminate process */
void passUpOrDie(int exceptionType){
  state_PTR oldState = NULL;
  if (exceptionType == 0) {
    /* currentProcess has its of tlbHandler */
    if (currentProcess->p_oldTLB != NULL) {
      oldState = (state_t *) TLBMANAGEMENTOLDAREA;
      stateCopy(oldState, currentProcess->p_oldTLB);
      LDST(currentProcess->p_newTLB);
    }
  }
  else if (exceptionType == 1) {
    /* currentProcess has its of pgmHandler */
    if (currentProcess->p_oldPgm != NULL) {
      oldState = (state_t *) PROGRAMTRAPOLDAREA;
      stateCopy(oldState, currentProcess->p_oldPgm);
      LDST(currentProcess->p_newPgm);
    }
  }
  else if (exceptionType == 2) {
    /* currentProcess has its of sysHandler */
    if (currentProcess->p_oldSys != NULL) {
      oldState = (state_t *) SYSCALLOLDAREA;
      stateCopy(oldState, currentProcess->p_oldSys);
      LDST(currentProcess->p_newSys);
    }
  }
  /* sys 5 hasnt been called before, kill it */
  terminateProcess();  
}

/* SYS 3
*  loads state from SYSCALLOLDAREA and gets the semaphore in a1,
*  incrament semaphore if semaphore <= 0, remove process from 
*  semaphore and add to readyQue, load state of oldSys */
void verhogen(){
  state_t *oldSys = (state_t *) SYSCALLOLDAREA;
  int *mutex = oldSys->s_a1;
  (*mutex)++;
  if (*mutex <= 0){
    pcb_PTR temp = removeBlocked(mutex);
    if(temp != NULL){
      temp->p_semAdd = NULL;
      insertProcQ(&readyQue, temp);
    }
  }
  LDST(oldSys);
}

/* SYS 4
*  load state from SYSCALLOLDAREA, get semaphore from a1,
*  decrament semaphore, if semaphore is less than 0,
*  store off cpu time in currentProcess, copy state from oldSys into current process,
*  insert current process onto semaphore queue, set current process to null,
*  call scheduler. If semaphore greater than 0, load state of oldSys */
void passeren(){
  state_t *oldSys = (state_t *) SYSCALLOLDAREA;
  int * mutex = oldSys->s_a1;
  (*mutex)--;
  if (*mutex < 0){
    cpu_t currTime = 0;
    STCK(currTime);
    currentProcess->p_time = currentProcess->p_time + (currTime - (time));
    stateCopy(oldSys, &(currentProcess->p_s));
    insertBlocked(mutex, currentProcess);
    currentProcess = NULL;
    scheduler();
  }
  LDST(oldSys);
}

/* SYS 6
*  load state from SYSCALLOLDAREA, store off the current time used by procces,
*  add current time to v0, load state of oldSys */
getCPUTime(){
  state_t *oldSys = (state_t *) SYSCALLOLDAREA;
  cpu_t currTime = 0;
  STCK(currTime);
  currentProcess->p_time = currentProcess->p_time + (currTime - (time));
  oldSys->s_v0 = currentProcess->p_time;
  LDST(oldSys);
}

/* SYS 7
*  load state from SYSCALLOLDAREA, get clock semaphore from semaphore table,
*  decrement semaphore, if less than 0, copy state of oldSys into current process,
*  insert current process onto semaphore queue, incrament soft blocked count,
*  store of cpu time in current process, call scheduler.
*  Else, load state of oldSys */
waitForClock(){
  state_t *oldSys = (state_t *) SYSCALLOLDAREA;
  int * semAdd = &(devSemTable[EIGHTDEVLINES * DEVSPERLINE + DEVSPERLINE]);
  (*semAdd)--;
  if ((*semAdd) < 0){
    stateCopy(oldSys, &(currentProcess->p_s));
    insertBlocked(semAdd, currentProcess);
    softBlockCount++;
    cpu_t currTime = 0;
    STCK(currTime);
    currentProcess->p_time = currentProcess->p_time + (currTime - (time));
    currentProcess = NULL;
    scheduler();
  }
  LDST(oldSys);
}

/* SYS 8
*  load state from SYSCALLOLDAREA, get pc, line number, device number, and if
*  terminal read from pc, a1, a2, and a3. If line number is 7 and if terminal read,
*  create offset of 8 to get terminal read semaphores. Calculate what device is being
*  used and get its semaphore. Decrament the semaphore, store of cpu time into 
*  current process, incrament softblocked count, insert current process onto
*  semaphore, call scheduler */
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

/* Program Trap Handler */
void pgmTrapHandler(){
  state_t *pgmOld = (state_t *) PROGRAMTRAPOLDAREA;
  devregarea_t *foo = (devregarea_t *) RAMBASEADDR;
  passUpOrDie(1);
}

/* tbl management handler */
void tlbMgmtHandler(){
  passUpOrDie(0);
}

/* sysCall Handler
*  gets state from SYSCALLOLDAREA, gets the sysCall number,
*  calculates if it is in kernal mode or not
*  passes on to sysCallDispatch to handle the sysCall */
void sysCallHandler(){
  state_t *syscallOld = (state_t *) SYSCALLOLDAREA;
  int * syscallNum = &(syscallOld->s_a0);
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

/* takes a sysCallNum and if kernel mode as parameters
*  if sysCallNum within between 0-9 and in kernel mode,
*  updates the does pc + 4 on the process then calls the 
*  correct function based on the sysCallNum.
*  If in user mode, causeses a privilaged instruction error
*  and passes to program trap handler.
*  If not between 0-9, call passUpOrDie */
void syscallDispatch(int * syscallNum, int kernelMode){
  if(*(syscallNum) > 0 && *(syscallNum) < 9){
    if(kernelMode == TRUE) {
      state_t *syscallOld = (state_t *) SYSCALLOLDAREA;
      syscallOld->s_pc = syscallOld->s_pc + 4; /* update processes pc */
      switch(*syscallNum){
        case 1:
          createProcess();
          break;
        case 2:
          terminateProcess();
          break;
        case 3:
          verhogen();
          break;
        case 4:
          passeren();
          break;
        case 5:
          specifyExceptionStateVector();
          break;
        case 6:
          getCPUTime();
          break;
        case 7:
          waitForClock();
          break;
        case 8:
          waitForIODevice();
          break;
      }
    }
    else { /* syscall 1-8 user mode, make it look like a privileged instruction error */
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
    passUpOrDie(2);
  }
}

#endif
