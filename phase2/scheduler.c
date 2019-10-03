#include "initial.c"
#include "../phase1/pcb.c"
#include "../phase1/asl.c"
#include "../h/types.h"
#include "../h/const.h"

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
        WAIT();
      }
    }
  }
  else{
    currentProcess = process;
    LDIT(5000);
    STCK(time);
    LDST(&(process->p_s));
  }
}