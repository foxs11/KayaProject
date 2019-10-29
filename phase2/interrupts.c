#ifndef INTERRUPTS
#define INTERRUPTS

#include "../e/pcb.e"
#include "../e/asl.e"
#include "../h/types.h"
#include "../h/const.h"
#include "../e/initial.e"

void addressDebug(unsigned int a, unsigned int b) {
  int i = 0;
} 

void lineDebug(unsigned int a, unsigned int b) {
  int i = 0;
} 

void interruptDebug(unsigned int a, unsigned int b) {
  int i = 0;
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
  int devIndex = ((lineNumber - 3) * 8) + deviceNumber - 1;
  return devIndex;
}

void interruptHandler(){
  cpu_t currTime = 0;
  STCK(currTime);
	if(currentProcess != NULL){
  	currentProcess->p_time = currentProcess->p_time + (currTime - (time));
	}
  state_t *interruptOld = (state_t *) INTERRUPTOLDAREA;
  unsigned int cause = interruptOld->s_cause;
  int lineNumber = NULL;
  lineNumber = getLineNumber(cause);
	lineDebug(lineNumber, 0);
  if (lineNumber > 2 && lineNumber < 8){ /* maybe remove line 5? */
		addokbuf("in intHandler 1 \n");
  	int deviceNumber = getDeviceNumber(lineNumber);
  	/* have line and device, get register area associated */
  	devregarea_t *foo = (devregarea_t *) RAMBASEADDR;

    int termOffset = 0;

  	int devRegIndex = getDevRegIndex(lineNumber, deviceNumber);
    device_t * device = &(foo->devreg[devRegIndex]);

    if (lineNumber == 7){
			addokbuf("in intHandler 2 \n");
      unsigned int intStatus = device->t_transm_status;
      if ((intStatus & 0x0F) == READY) { /* recv */
        termOffset = 8; 
      }
    }

		unsigned int devSemNum = getSemArrayNum(lineNumber, deviceNumber, termOffset);
		interruptDebug(devSemNum, 0);
    int * semAdd = &(devSemTable[getSemArrayNum(lineNumber, deviceNumber, termOffset)]);  /*change for terminal math */
  	addokbuf("in intHandler 3 \n");
		(*semAdd)++;
  	pcb_PTR p = NULL;
  	if ((*semAdd) <= 0) {
			addokbuf("in intHandler 4 \n");
  		p = removeBlocked(semAdd);
  		p->p_s.s_v0 = device->d_status;
  		insertProcQ(&readyQue, p);
  		softBlockCount--;
  		/*ack the interrupt */
  		if (lineNumber == 7) {
				addokbuf("in intHandler 5 \n");
  			ackTerminal(&devRegIndex);
  		}
  		else{
				addokbuf("in intHandler 6 \n");
  			device->d_command = 1;
  		}
  		if (waitFlag == 1) {
				addokbuf("in intHandler 7 \n");
  			scheduler();
  		}
  		else{
				addokbuf("in intHandler 8 \n");
        STCK(time);
  			LDST(interruptOld);
			}
  	}
  }
  else { /* line number not between 3 and 7 */
  	if (lineNumber == 1) {
      scheduler();
    }
    else { /* line number 2 */
      LDIT(100000);
			addokbuf("in line2 1 \n");
      if (headBlocked(&(devSemTable[(EIGHTDEVLINES * DEVSPERLINE) + DEVSPERLINE])) != NULL) { /* are there processes blocked on IT */
				addokbuf("in line2 2 \n");
				devSemTable[(EIGHTDEVLINES * DEVSPERLINE) + DEVSPERLINE]++;
        addokbuf("in line2 3 \n");
				if (devSemTable[(EIGHTDEVLINES * DEVSPERLINE) + DEVSPERLINE] <= 0){
				  addokbuf("in line2 4 \n");
					while (headBlocked(&(devSemTable[(EIGHTDEVLINES * DEVSPERLINE) + DEVSPERLINE])) != NULL) {
						pcb_PTR temp = removeBlocked(&(devSemTable[EIGHTDEVLINES * DEVSPERLINE + DEVSPERLINE]));
            insertProcQ(&readyQue, temp);
          }
        }
      }
      addokbuf("in line2 5 \n");
      LDST(interruptOld);
    }
  }
}


#endif
