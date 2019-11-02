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
  addokbuf("in scheduler 1")
  unsigned int cp0status;
  pcb_PTR process = removeProcQ(&readyQue);
  if(process == NULL){
    addokbuf("in scheduler 2")
    if(processCount == 0){
      HALT();
    }
    else{
      addokbuf("in scheduler 3")
      if(softBlockCount == 0){
        PANIC();
      }
      else{
        addokbuf("in scheduler 4")
        cp0status = SCHEDULERINTSUNMASKED;
        waitFlag = 1;
        setSTATUS(cp0status); /* has a return value...?  */
        WAIT();
      }
    }
  }
  else{
    addokbuf("in scheduler 5")
    waitFlag = 0;
    currentProcess = process;
    setTIMER(5000);
    addokbuf("in scheduler 6")
    STCK(time);
    addokbuf("in scheduler 7")
    LDST(&(process->p_s));
  }
}

#endif
