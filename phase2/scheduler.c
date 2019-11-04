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
  addokbuf("scheduler 1\n");
  pcb_PTR process = removeProcQ(&readyQue);
  if(process == NULL){
    if(processCount == 0){
      addokbuf("scheduler 2\n");
      HALT();
    }
    else{
      if(softBlockCount == 0){
        addokbuf("scheduler panic\n");
        PANIC();
      }
      else{
        addokbuf("scheduler 3\n");
        cp0status = SCHEDULERINTSUNMASKED;
        addokbuf("scheduler 4\n");
        waitFlag = 1;
        addokbuf("scheduler 5\n");
        setSTATUS(cp0status); /* has a return value...?  */
        addokbuf("scheduler 6\n");
        WAIT();
      }
    }
  }
  else{
    addokbuf("scheduler 7\n");
    waitFlag = 0;
    currentProcess = process;
    setTIMER(5000);
    STCK(time);
    bDebug(process->p_s.s_status, 0);
    LDST(&(process->p_s));
  }
}

#endif
