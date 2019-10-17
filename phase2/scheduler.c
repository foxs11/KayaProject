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
  int i = 0;
}

void scheduler(){
  addokbuf("in scheduler 1\n");
  pcb_PTR process = removeProcQ(&readyQue);
  addokbuf("in scheduler 2\n");
  if(process == NULL){
    addokbuf("in scheduler 3\n");
    if(processCount == 0){
      addokbuf("in scheduler 4\n");
      HALT();
    }
    else{
      addokbuf("in scheduler 5\n");
      if(softBlockCount == 0){
        addokbuf("in scheduler 6\n");
        PANIC();
      }
      else{
        addokbuf("in scheduler 7\n");
        unsigned int cp0status = getSTATUS();
        bDebug(cp0status, 1);
        addokbuf("in scheduler 8\n");
        cp0status = cp0status | SCHEDULERINTSUNMASKED;
        bDebug(cp0status, 1);
        addokbuf("in scheduler 9\n");
        unsigned int garbage = 0;
        garbage = setSTATUS(cp0status); /* has a return value...? */
        addokbuf("in scheduler 10\n");
        waitFlag = 1;
        addokbuf("in scheduler 11\n");
        WAIT();
      }
    }
  }
  else{
    currentProcess = process;
    setTIMER(5000);
    addokbuf("in scheduler 12\n");
    STCK(time);
    addokbuf("in scheduler 13\n");
    LDST(&(process->p_s));
  }
}

#endif
