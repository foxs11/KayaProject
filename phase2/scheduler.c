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

void waitDebug(unsigned int a, unsigned int b) {
  int i = 0;
} 

void scheduler(){
  unsigned int cp0status;
  if(headProcQ(readyQue) == NULL){
    addokbuf("readyQue head is null \n");
  }
  else {
    addokbuf("readyQue head is not null \n");
  }
  pcb_PTR process = removeProcQ(&readyQue);
  if(process == NULL){
    if(processCount == 0){
      HALT();
    }
    else{
    bDebug(softBlockCount, 0);
      if(softBlockCount == 0){
        addokbuf("scheduler panic\n");
        PANIC();
      }
      else{
        cp0status = SCHEDULERINTSUNMASKED;
        bDebug(cp0status, 1);
        waitFlag = 1;
        setSTATUS(cp0status); /* has a return value...?  */
        WAIT();
      }
    }
  }
  else{
    addokbuf("asdfasfasdf 1\n");
    waitFlag = 0;
    addokbuf("asdfasfasdf 2\n");
    currentProcess = process;
    addokbuf("asdfasfasdf 3\n");
    setTIMER(5000);
    addokbuf("asdfasfasdf 4\n");
    STCK(time);
    addokbuf("asdfasfasdf 5\n");
    waitDebug(0, 0);
    LDST(&(process->p_s));
  }
}

#endif
