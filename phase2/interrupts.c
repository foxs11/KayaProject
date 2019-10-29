#ifndef INTERRUPTS
#define INTERRUPTS

#include "../e/pcb.e"
#include "../e/asl.e"
#include "../h/types.h"
#include "../h/const.h"
#include "../e/initial.e"

void interruptDebug(unsigned int a, unsigned int b) {
  int i = 0;
}

int getLineNumber(unsigned int cause){
	int lineNumber = NULL;

	if (CHECK_BIT(cause, 9)) {
  	lineNumber = 0;
  }
  else if (CHECK_BIT(cause, 10)) {
  	lineNumber = 1;
  }
  else if (CHECK_BIT(cause, 11)) {
  	lineNumber = 2;
  }
  else if (CHECK_BIT(cause, 12)) {
  	lineNumber = 3;
  }
  else if (CHECK_BIT(cause, 13)) {
  	lineNumber = 4;
  }
  else if (CHECK_BIT(cause, 14)) {
  	lineNumber = 5;
  }
  else if (CHECK_BIT(cause, 15)) {
  	lineNumber = 6;
  }
  else if (CHECK_BIT(cause, 16)) {
  	lineNumber = 7;
  }
}

int getDeviceNumber(int lineNumber){
	if (3 >= lineNumber >= 7){
  		unsigned int address = (lineNumber - 3) * 4 + LINE3INTBITMAP;
  		unsigned int bitMap = (memaddr) address;

  		int deviceNumber = NULL;
  		if (CHECK_BIT(bitMap, 9)) {
  			deviceNumber = 0;
	  	}
	  	else if (CHECK_BIT(bitMap, 1)) {
	  		deviceNumber = 1;
		}
	  	else if (CHECK_BIT(bitMap, 2)) {
	  		deviceNumber = 2;
	  	}
	  	else if (CHECK_BIT(bitMap, 3)) {
	  		deviceNumber = 3;
	  	}
	  	else if (CHECK_BIT(bitMap, 4)) {
	  		deviceNumber = 4;
	  	}
	  	else if (CHECK_BIT(bitMap, 5)) {
	  		deviceNumber = 5;
	  	}
	 	 else if (CHECK_BIT(bitMap, 6)) {
	  		deviceNumber = 6;
	 	}
	 	 else if (CHECK_BIT(bitMap, 7)) {
		  	deviceNumber = 7;
	  	}
	  	return deviceNumber;
	}
}

unsigned int ackTerminal(int *devSemNum){
	volatile devregarea_t *deviceRegs;
	unsigned int intStatus;

	deviceRegs = (devregarea_t *) RAMBASEADDR;

	intStatus = deviceRegs->devreg[(*devSemNum)].t_transm_status;
	if ((intStatus & 0x0F) != READY) {
		deviceRegs->devreg[(*devSemNum)].t_transm_command = ACK;
	}

	else {
		intStatus = deviceRegs->devreg[(*devSemNum)].t_recv_command = ACK;

		*devSemNum = *devSemNum + DEVPERINT;
	}

	return intStatus;
}

int getSemArrayNum(int lineNumber, int deviceNumber, int termOffset){
	addokbuf("in getSemArrayNum 1\n");
	int arrayNum = ((lineNumber-3)*8);
	addokbuf("in getSemArrayNum 2\n");
	arrayNum = arrayNum + deviceNumber + termOffset;
	addokbuf("in getSemArrayNum 3\n");
	return arrayNum;
}

int getDevRegIndex(int lineNumber, int deviceNumber) {
  int devIndex = ((lineNumber - 3) * 8) + deviceNumber - 1;
  return devIndex;
}

void interruptHandler(){
  cpu_t currTime = 0;
	addokbuf("in intHandler \n");
  STCK(currTime);
	if(currentProcess != NULL){
		addokbuf("in intHandler 1\n");
  	currentProcess->p_time = currentProcess->p_time + (currTime - (time));
	}
	addokbuf("in intHandler 2\n");
  state_t *interruptOld = (state_t *) INTERRUPTOLDAREA;
  unsigned int cause = interruptOld->s_cause;
  int lineNumber = NULL;
  lineNumber = getLineNumber(cause);
  addokbuf("in intHandler 3\n");
	interruptDebug(lineNumber, 0);
  if (lineNumber > 2 && lineNumber < 8){ /* maybe remove line 5? */
		addokbuf("in intHandler 4\n");
  	int deviceNumber = getDeviceNumber(lineNumber);
  	/* have line and device, get register area associated */
  	devregarea_t *foo = (devregarea_t *) RAMBASEADDR;

    int termOffset = 0;

  	int devRegIndex = getDevRegIndex(lineNumber, deviceNumber);
    device_t * device = &(foo->devreg[devRegIndex]);

    if (lineNumber == 7){
			addokbuf("in intHandler 5\n");
      unsigned int intStatus = device->t_transm_status;
      if ((intStatus & 0x0F) == READY) { /* recv */
        termOffset = 8; 
      }
    }

    int * semAdd = &(devSemTable[getSemArrayNum(lineNumber, deviceNumber, termOffset)]);  /*change for terminal math */
  	/* increment or decrement sem here? */
  	pcb_PTR p = NULL;
  	if ((*semAdd) <= 0) {
  		p = removeBlocked(semAdd);
  		p->p_s.s_v0 = device->d_status;
  		insertProcQ(&readyQue, p);
  		softBlockCount--;
  		/*ack the interrupt */
  		if (lineNumber == 7) {
  			ackTerminal(&devRegIndex);
  		}
  		else{
  			device->d_command = 1;
  		}
  		if (waitFlag == 1) {
  			scheduler();
  		}
  		else{
        STCK(time);
  			LDST(interruptOld);
  		}
  		
  	}
  }
  else { /* line number not between 3 and 7 */
  	if (lineNumber == 1) {
			addokbuf("in intHandler 6\n");
      scheduler();
    }
    else { /* line number 2 */
			addokbuf("in intHandler 7\n");
      LDIT(100000);
			addokbuf("in intHandler 8\n");
      if (headBlocked(&(devSemTable[EIGHTDEVLINES * DEVSPERLINE + DEVSPERLINE])) != NULL) { /* are there processes blocked on IT */
        addokbuf("in intHandler 9\n");
				devSemTable[EIGHTDEVLINES * DEVSPERLINE + DEVSPERLINE]++;
        if (devSemTable[EIGHTDEVLINES * DEVSPERLINE + DEVSPERLINE] <= 0){
         	addokbuf("in intHandler 6\n");
				  while (headBlocked(&(devSemTable[EIGHTDEVLINES * DEVSPERLINE + DEVSPERLINE])) != NULL) {
            addokbuf("in intHandler 6\n");
						pcb_PTR temp = removeBlocked(&(devSemTable[EIGHTDEVLINES * DEVSPERLINE + DEVSPERLINE]));
            insertProcQ(&readyQue, temp);
          }
        }
      }
      
      LDST(interruptOld);
    }
  }
}


#endif
