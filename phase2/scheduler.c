#include "initial.c"
#include "../phase1/pcb.c"
#include "../phase1/asl.c"
#include "../h/types.h"
#include "../h/const.h"
#include "/usr/local/include/umps2/umps/libumps.e"

void scheduler(){
  pcb_PTR process = removeProcQ(&readyQue);
  if(process == NULL){
    if(processCount == 0){
      HALT();
    }
    else{
      if(softBlockCount == 0){
        PANIC();
      }
      else{
        
        unsigned int cp0status = getSTATUS();
        cp0status = cp0status | SCHEDULERINTSUNMASKED;
        setSTATUS(cp0status); /* has a return value...? */
        waitFlag = 1;
        WAIT();
      }
    }
  }
  else{
    currentProcess = process;
    setTimer(5000);
    STCK(*time);
    LDST(&(process->p_s));
  }
}

void setStatus(unsigned int * status){

}