#ifndef SCHEDULER
#define SCHEDULER

/* SCHEDULER.C is called whenever a quantum ends or the current process 
   has been terminated. It chooses a new job to run via round robin from the
   ready queue. If there is no ready job but there is at least one job
   blocked on a device sema4 (i.e. we know it will become unblocked), a wait
   state is entered until a job is ready to run. If there are no jobs softblocked
   and none on the ready queue but there are jobs still alive, we invoke a kernel panic
   and if there are no jobs in existence the machine halts.
*/

#include "../e/initial.e"
#include "../e/pcb.e"
#include "../e/asl.e"
#include "../h/types.h"
#include "../h/const.h"
#include "/usr/local/include/umps2/umps/libumps.e"

void scheduler(){
  unsigned int cp0status;
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
        cp0status = SCHEDULERINTSUNMASKED;
        waitFlag = 1;
        setSTATUS(cp0status);
        WAIT(); /* wait for softblocked to return to readyQ */
      }
    }
  }
  else{
    waitFlag = 0;
    currentProcess = process;
    setTIMER(5000); /* start new quantum */
    STCK(time);
    LDST(&(process->p_s));
  }
}

#endif
