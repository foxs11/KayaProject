#include "../phase1/pcb.c"
#include "../phase1/asl.c"
#include "../h/types.h"
#include "../h/const.h"

void interruptHandler(){
  state_t *interruptOld = (state_t *) INTERRUPTOLDAREA;
  unsigned int cause = interruptOld->s_cause;
  int lineNumber = NULL;
  lineNumber = getLineNumber(cause);
  

  if (3 >= lineNumber >= 7){
  	int deviceNumber = getDeviceNumber(lineNumber);
  	/* have line and device, get register area associated */
  	devregarea_t *foo = (devregarea_t *) RAMBASEADDR;
  	int devIndex = ((lineNumber - 3) * 8) + deviceNumber - 1;
  	device_t * device = &(foo->devreg[devIndex]);

  	/* int handler video 2 */
  	/*get semAdd*/
  	
  }
  else { /* line number not between 3 and 7 */

  }


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