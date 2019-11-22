#ifndef INTERRUPTS
#define INTERRUPTS

/* INTERRUPTS.C handles the processing of all interrupts. Interrupts
are handled in order of priority from lowest line and device number
to highest. For interrupts that are not associated with the clocks,
a status is returned in oldInt's v0 indicating the result of the device's
operation. The device is then ACKed, and a V operation is performed on the
device's semaphore to return the process that has finished it's wait for IO
to the ready queue. Then the scheduler is called if there is no current process,
or control is returned to the current process if it exists.
For processor local timer interrupts (that signal the end
of a quantum), the current process is returned to the readyQ and the
scheduler is called. For pseudo clock ticks, all processes blocked on the 
psuedo clock are returned to the readyQ via a mass V and the psuedo clock is
reloaded with the value of a pseudo clock tick. */

#include "../e/pcb.e"
#include "../e/asl.e"
#include "../h/types.h"
#include "../h/const.h"
#include "../e/initial.e"
#include "../e/exceptions.e"

/* Given a cause register, the function returns the lowest line number with a pending device interrupt */
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

/* Given a line number, the function returns the lowest device number with a pending 
device interrupt by looking at the appropriate interrupting device bitmap */
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

/* Given a the index of a terminal device semaphore that has an interrupt pending, 
the function ACKs the appropriate command word in the terminal's device register 
area and returns the status field of the terminal "personality" that was ACKED.
If both a receive and a transmit have pending interrupts, transmit takes priority */ 
unsigned int ackTerminal(int *devSemNum){
	unsigned int intStatus;
	volatile devregarea_t *deviceRegs;

	deviceRegs = (devregarea_t *) RAMBASEADDR;

	intStatus = deviceRegs->devreg[(*devSemNum)].t_transm_status;
	if ((intStatus & 0x0F) != READY) {
		deviceRegs->devreg[(*devSemNum)].t_transm_command = ACK;
	}

	else {
		intStatus = deviceRegs->devreg[(*devSemNum)].t_recv_status;
		deviceRegs->devreg[(*devSemNum)].t_recv_command = ACK;

		*devSemNum = *devSemNum + DEVPERINT;
	}

	return intStatus;
}

/* Given a line number, device number, and an offset, calculates the index
of the appropriate semaphore for the given device. Offset will be 0 the device
in question is not a terminal or if it is a transmit, and 8 if a receive. */ 
int getSemArrayNum(int lineNumber, int deviceNumber, int termOffset){
	int arrayNum = ((lineNumber-3)*8);
	arrayNum = arrayNum + deviceNumber + termOffset;
	return arrayNum;
}

/* Given a line number and device number, calculates and returns the index
of the appropriate device register area. This index is later used to access
the device register as a struct */
int getDevRegIndex(int lineNumber, int deviceNumber) {
  int devIndex = ((lineNumber - 3) * 8) + deviceNumber;
  return devIndex;
}

/* This is the function that handles interrupts. When an interrupt context switch occurs,
Execution resumes here. It identifies the highest priority interrupt, acks it, Vs the appropriate sema4
and returns a status in oldInt's v0 register if the interrupt wasn't for a clock device.
For pseudo clock tick interrupts, all processes waiting on the pseudo clock are woken up,
and for the end of a quantum control is returned to the scheduler. */
void interruptHandler(){
	unsigned int status = 0;
	int termOffset = 0;
  cpu_t currTime = 0;
  STCK(currTime);
	if(currentProcess != NULL){ /* if in a wait state, do not attribute time */
  	currentProcess->p_time = currentProcess->p_time + (currTime - (time));
	}
  state_t *interruptOld = (state_t *) INTERRUPTOLDAREA;
  unsigned int cause = interruptOld->s_cause;
  int lineNumber = NULL;
  lineNumber = getLineNumber(cause);
  if (lineNumber > 2 && lineNumber < 8){
  	int deviceNumber = getDeviceNumber(lineNumber);
  	/* have line and device, get register area associated */
  	devregarea_t *foo = (devregarea_t *) RAMBASEADDR;

  	int devRegIndex = getDevRegIndex(lineNumber, deviceNumber);
    device_t * device = &(foo->devreg[devRegIndex]);

    if (lineNumber == 7){
			status = ackTerminal(&devRegIndex);
      if ((status & 0x0F) == READY) { /* recv, so offset dev sem index */
        termOffset = 8; 
      }
    }
		else{
			status = device->d_status;
  		device->d_command = ACK;
  	}
    /* perform a v operation on the appropriate dev semaphore */
    int * semAdd = &(devSemTable[getSemArrayNum(lineNumber, deviceNumber, termOffset)]);
		(*semAdd)++;
  	pcb_PTR p = NULL;
  	if ((*semAdd) <= 0) {
  		p = removeBlocked(semAdd);
			p->p_semAdd = NULL;
  		p->p_s.s_v0 = status;
  		insertProcQ(&readyQue, p);
  		softBlockCount--;
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
  	if (lineNumber == 1) { /* end of a quantum, put current process back on ready q and call scheduler to give another process a turn */ 
			stateCopy(interruptOld, &(currentProcess->p_s));
			insertProcQ(&readyQue, currentProcess);
			currentProcess = NULL;
      scheduler();
    }
    else { /* line number 2 */
      LDIT(100000);
      if (headBlocked(&(devSemTable[(EIGHTDEVLINES * DEVSPERLINE) + DEVSPERLINE])) != NULL) { /* are there processes blocked on IT */
				if (devSemTable[(EIGHTDEVLINES * DEVSPERLINE) + DEVSPERLINE] <= 0){
					while (headBlocked(&(devSemTable[(EIGHTDEVLINES * DEVSPERLINE) + DEVSPERLINE])) != NULL) {
						pcb_PTR temp = removeBlocked(&(devSemTable[EIGHTDEVLINES * DEVSPERLINE + DEVSPERLINE]));
						temp->p_semAdd = NULL;
						devSemTable[(EIGHTDEVLINES * DEVSPERLINE) + DEVSPERLINE]++;
            insertProcQ(&readyQue, temp);
            softBlockCount--;
          }
        }
      }
      if (currentProcess != NULL){
        LDST(interruptOld);
      }
      else{ /* wait state resume */
        scheduler();
      }
    }
  }
}


#endif
