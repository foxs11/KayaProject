#include "../phase1/pcb.c"
#include "../phase1/asl.c"
#include "../h/types.h"
#include "../h/const.h"

void pgmTrapHandler(){}
void tlbMgmtHandler(){}
void sysCallHandler(){
  state_t *syscallOld = (state_t *) SYSCALLOLDAREA;
  int syscallNum = syscallOld->s_a0;
  int kernelMode;
  int kernelStatus = syscallOld->s_status & KERNELOFF;
  if(kernelStatus == ALLOFF){
   kernelMode = TRUE;
  }
  else{
    kernelMode = FALSE;
  }
  if(kernelMode){
    syscallDispatch(syscallNum);
  }
  else{
    //TODO: make program trap for this else
  }
}

void syscallDispatch(int syscallNum){
  if(1<= syscallNum <= 8){
    switch(syscallNum){
      case 1:
        createProcess();
      case 2:
        terminateProcess();
      case 3:
        verhogen();
      case 4:
        passeren();
      case 5:
        specifyExceptionStateVector();
      case 6:
        getCPUTime();
      case 7:
        waitForClock();
      case 8:
        waitForIODevice();
    }
  }
  else{
    passUpOrDie();
  }
}
