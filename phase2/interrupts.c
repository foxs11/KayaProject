#ifndef INTERRUPTS
#define INTERRUPTS

#include "../e/pcb.e"
#include "../e/asl.e"
#include "../h/types.h"
#include "../h/const.h"
#include "../e/initial.e"

void intDebug(unsigned int a, unsigned int b){
  int i;
  i = 42;
}

int getLineNumber(unsigned int cause){
	int lineNumber = NULL;

	if (CHECK_BIT(cause, 8)) {
  	lineNumber = 0;
  }
  else if (CHECK_BIT(cause, 9)) {
  	lineNumber = 1;
  }
  else if (CHECK_BIT(cause, 10)) {
  	lineNumber = 2;
  }
  else if (CHECK_BIT(cause, 11)) {
  	lineNumber = 3;
  }
  else if (CHECK_BIT(cause, 12)) {
  	lineNumber = 4;
  }
  else if (CHECK_BIT(cause, 13)) {
  	lineNumber = 5;
  }
  else if (CHECK_BIT(cause, 14)) {
  	lineNumber = 6;
  }
  else if (CHECK_BIT(cause, 15)) {
  	lineNumber = 7;
  }
	return lineNumber;
}

int getDeviceNumber(int lineNumber){
	if (2 < lineNumber && lineNumber < 8){
  		intDevBitMap_PTR bitMap = (intDevBitMap_PTR) LINE3INTBITMAP;
      unsigned int lineNumberBitMap;
      switch (lineNumber){
        case 3: 
          lineNumberBitMap = bitMap->i_lineThree;
          break;
        case 4: 
          lineNumberBitMap = bitMap->i_lineFour;
          break;
        case 5: 
          lineNumberBitMap = bitMap->i_lineFive;
          break;
        case 6: 
          lineNumberBitMap = bitMap->i_lineSix;
          break;
        case 7: 
          lineNumberBitMap = bitMap->i_lineSeven;
          break;

      }

  		int deviceNumber = NULL;
  		if (CHECK_BIT(lineNumberBitMap, 0)) {
  			deviceNumber = 0;
	  	}
	  	else if (CHECK_BIT(lineNumberBitMap, 1)) {
	  		deviceNumber = 1;
			}
	  	else if (CHECK_BIT(lineNumberBitMap, 2)) {
	  		deviceNumber = 2;
	  	}
	  	else if (CHECK_BIT(lineNumberBitMap, 3)) {
	  		deviceNumber = 3;
	  	}
	  	else if (CHECK_BIT(lineNumberBitMap, 4)) {
	  		deviceNumber = 4;
	  	}
	  	else if (CHECK_BIT(lineNumberBitMap, 5)) {
	  		deviceNumber = 5;
	  	}
	 	 else if (CHECK_BIT(lineNumberBitMap, 6)) {
	  		deviceNumber = 6;
	 	}
	 	 else if (CHECK_BIT(lineNumberBitMap, 7)) {
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
	int arrayNum = ((lineNumber-3)*8);
	arrayNum = arrayNum + deviceNumber + termOffset;
	return arrayNum;
}

int getDevRegIndex(int lineNumber, int deviceNumber) {
  int devIndex = ((lineNumber - 3) * 8) + deviceNumber;
  return devIndex;
}

void interruptHandler(){
	addokbuf("in interuptHandler 1 \n");
	int termOffset = 0;
  cpu_t currTime = 0;
  STCK(currTime);
	if(currentProcess != NULL){
		addokbuf("in interuptHandler 2 \n");
  	currentProcess->p_time = currentProcess->p_time + (currTime - (time));
	}
  state_t *interruptOld = (state_t *) INTERRUPTOLDAREA;
  unsigned int cause = interruptOld->s_cause;
  int lineNumber = NULL;
  lineNumber = getLineNumber(cause);
  if (lineNumber > 2 && lineNumber < 8){ /* maybe remove line 5? */
		addokbuf("in interuptHandler 3 \n");
  	int deviceNumber = getDeviceNumber(lineNumber);
  	/* have line and device, get register area associated */
  	devregarea_t *foo = (devregarea_t *) RAMBASEADDR;

  	int devRegIndex = getDevRegIndex(lineNumber, deviceNumber);
    device_t * device = &(foo->devreg[devRegIndex]);

    if (lineNumber == 7){
			addokbuf("in interuptHandler 4 \n");
      unsigned int intStatus = device->t_transm_status;
      if ((intStatus & 0x0F) == READY) { /* recv */
				addokbuf("in interuptHandler 5 \n");
        termOffset = 8; 
      }
    }

    int * semAdd = &(devSemTable[getSemArrayNum(lineNumber, deviceNumber, termOffset)]);  /*change for terminal math */
		(*semAdd)++;
  	pcb_PTR p = NULL;
  	if ((*semAdd) <= 0) {
			addokbuf("in interuptHandler 6 \n");
  		p = removeBlocked(semAdd);
  		p->p_s.s_v0 = device->d_status;
  		insertProcQ(&readyQue, p);
      intDebug(p->p_s.s_pc, 0);
  		softBlockCount--;
  		/*ack the interrupt */
  		if (lineNumber == 7) {
				addokbuf("in interuptHandler 7 \n");
  			ackTerminal(&devRegIndex);
  		}
  		else{
  			device->d_command = 1;
  		}
  		if (waitFlag == 1) {
				addokbuf("in interuptHandler 8 \n");
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
			addokbuf("in interuptHandler 9 \n");
      scheduler();
    }
    else { /* line number 2 */
      LDIT(100000);
			addokbuf("in interuptHandler 10 \n");
      if (headBlocked(&(devSemTable[(EIGHTDEVLINES * DEVSPERLINE) + DEVSPERLINE])) != NULL) { /* are there processes blocked on IT */
				devSemTable[(EIGHTDEVLINES * DEVSPERLINE) + DEVSPERLINE]++;
				if (devSemTable[(EIGHTDEVLINES * DEVSPERLINE) + DEVSPERLINE] <= 0){
					while (headBlocked(&(devSemTable[(EIGHTDEVLINES * DEVSPERLINE) + DEVSPERLINE])) != NULL) {
						pcb_PTR temp = removeBlocked(&(devSemTable[EIGHTDEVLINES * DEVSPERLINE + DEVSPERLINE]));
            insertProcQ(&readyQue, temp);
            softBlockCount--;
          }
        }
      }
      LDST(interruptOld);
    }
  }
}


#endif
