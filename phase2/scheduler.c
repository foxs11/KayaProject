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
    addokbuf("in scheduler 1 \n");
    if(processCount == 0){
      HALT();
    }
    else{
      addokbuf("in scheduler 2 \n");
      /*bDebug(softBlockCount, 0);*/
      if(softBlockCount == 0){
        PANIC();
      }
      else{
        addokbuf("in scheduler 3 \n");
        cp0status = SCHEDULERINTSUNMASKED;
        /*bDebug(cp0status, 1);*/
        waitFlag = 1;
        setSTATUS(cp0status); /* has a return value...?  */
        WAIT();
      }
    }
  }
  else{
    addokbuf("in scheduler 4 \n");
    waitFlag = 0;
    addokbuf("in scheduler 5 \n");
    currentProcess = process;
    addokbuf("in scheduler 6 \n");
    setTIMER(5000);
    addokbuf("in scheduler 7 \n");
    STCK(time);
    addokbuf("in scheduler 8 \n");
    LDST(&(process->p_s));
  }
}

#endif
