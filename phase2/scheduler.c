#ifndef SCHEDULER
#define SCHEDULER

#include "../e/initial.e"
#include "../e/pcb.e"
#include "../e/asl.e"
#include "../h/types.h"
#include "../h/const.h"
#include "/usr/local/include/umps2/umps/libumps.e"
#include "../phase1/p1test.c"


void addokbuf(char *strp) {
	char *tstrp = strp;
	while ((*mp++ = *strp++) != '\0')
		;
	mp--;
	termprint(tstrp, 0);
}

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
    setTIMER(5000);
    addokbuf("in scheduler\n");
    STCK(*time);
    addokbuf("in scheduler 2\n");
    LDST(&(process->p_s));
  }
}

#endif
