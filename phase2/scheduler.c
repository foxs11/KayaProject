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
  unsigned int cp0status;
  if(headProcQ(readyQue) == NULL){
    addokbuf("readyQue head is null \n");
  }
  else {
    addokbuf("readyQue head is not null \n");
  }
  pcb_PTR process = removeProcQ(&readyQue);
  if(process == NULL){
    addokbuf("scheduler 1\n");
    if(processCount == 0){
      HALT();
    }
    else{
      addokbuf("scheduler 2\n");
      bDebug(softBlockCount, 0);
      if(softBlockCount == 0){
        addokbuf("scheduler 3\n");
        PANIC();
      }
      else{
        addokbuf("scheduler 4\n");
        cp0status = SCHEDULERINTSUNMASKED;
        addokbuf("scheduler 5\n");
        bDebug(cp0status, 1);
        addokbuf("scheduler 6\n");
        waitFlag = 1;
        addokbuf("scheduler 7\n");
        setSTATUS(cp0status); /* has a return value...?  */
        addokbuf("scheduler 8\n");
        WAIT();
      }
    }
  }
  else{
    waitFlag = 0;
    currentProcess = process;
    setTIMER(5000);
    STCK(time);
    bDebug(process->p_s.s_pc, 0);
    LDST(&(process->p_s));
  }
}

#endif
