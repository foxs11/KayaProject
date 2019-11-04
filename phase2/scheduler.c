#ifndef SCHEDULER
#define SCHEDULER

#include "../e/initial.e"
#include "../e/pcb.e"
#include "../e/asl.e"
#include "../h/types.h"
#include "../h/const.h"
#include "/usr/local/include/umps2/umps/libumps.e"
#include "../phase1/p1test.c"

void bDebug(unsigned int a, unsigned int b) {
  int i;
  i = 42;
}

void scheduler(){
  unsigned int cp0status;
  pcb_PTR process = removeProcQ(&readyQue);
  if(process == NULL){
    if(processCount == 0){
      HALT();
    }
    else{
      if(softBlockCount == 0){
        addokbuf("scheduler panic\n");
        PANIC();
      }
      else{
        cp0status = SCHEDULERINTSUNMASKED;
        waitFlag = 1;
        setSTATUS(cp0status); /* has a return value...?  */
        WAIT();
      }
    }
  }
  else{
    waitFlag = 0;
    currentProcess = process;
    setTIMER(5000);
    STCK(time);
    bDebug(process->p_s.s_status, 0);
    LDST(&(process->p_s));
  }
}

#endif
