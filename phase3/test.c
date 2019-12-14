#include "../h/types.h"
#include "../h/const.h"
#include "../e/initial.e"

pgb_PTR bufferArray[16] = (pgb_PTR) ENDOFOS; /* should this be an array of non-pointers? */
int diskBufferMutexes[8];

pgb_PTR framePool[UPROCNUM*2] = (pgb_PTR) FRAMEPOOL;

static int currentFrame = 0;

st_PTR segTable = (st_PTR) SEGTBLS;

devregarea_t *foo = (devregarea_t *) RAMBASEADDR;
int ramTop = foo->rambase + foo->ramsize;

pgb_PTR stacks[UPROCNUM*3] = (pgb_PTR) ramTop - (1024 * UPROCNUM * 3);

state_t states[UPROCNUM*3]; 


/*1 ksegOS page table
8 kuseg2 page table
1 kuseg3 page table

1 "master" sem initialized to 0

1 semaphore for each device and two for each terminal for mutex initialized to 1

frame pool data structure size is 16? 12? 10? array of structs
	one entry for each frame in struct
		each entry: processID, segment #, page #, pointer to pg table entry

user process data structure (array of structs)
	8 of them, each entry has semaphore (used for delay and psuedo p and v, intiialized to 0), kuseg2 pg table, backing store address,
	6 state_t areas for old and new passupordie 
*/
int masterSem = 0;



/* kseg0s page tables */

pt_t ksegOSPT;

pt_t kuseg3PT;

uproc_t uprocs[UPROCNUM];

void test(){
	for (int i = 0; i < UPROCNUM; i++){

	
		states[i*3].s_sp = &(stacks[i*3+1]); 
  		states[i*3 + 1].s_sp = &(stacks[i*3+2]);
  		states[i*3 + 2].s_sp = &(stacks[i*3+3]);


  		states[i*3].s_pc = (memaddr) sysCallUpper; 
  		states[i*3 + 1].s_pc = (memaddr) pgmTrapHandler; /*change*/
  		states[i*3 + 2].s_pc = (memaddr) tlbMgmtHandler;


  		states[i*3].s_t9 = (memaddr) sysCallUpper; 
  		states[i*3 + 1].s_t9 = (memaddr) pgmTrapHandler; /*change*/
  		states[i*3 + 2].s_t9 = (memaddr) tlbMgmtHandler;


  		states[i*3].s_status = INTSUNMASKED | VMON | PROCLOCALTIMEON | KERNELON;
  		states[i*3 + 1].s_status = INTSUNMASKED | VMON | PROCLOCALTIMEON | KERNELON;
  		states[i*3 + 2].s_status = INTSUNMASKED | VMON | PROCLOCALTIMEON | KERNELON;
  	}


	for (int i = 0; i < 8; i++){
		diskBufferMutexes[i] = 1;
	}

	for (int i = 0; i < UPROCNUM; i++){
		segTable->entries[i].s_ksegOS = &ksegOSPT;
		segTable->entries[i].s_kuseg3 = &kuseg3PT;
	}

	


	for (int i = 0; i < UPROCNUM; i++){
		state_PTR initialState = NULL;
		initialState->s_asid = i+1;

		uprocs[i].u_pt.p_header = 0;
		int magicNum = 42;
		magicNum << 24;
		uprocs[i].u_pt.p_header += magicNum;
		uprocs[i].u_sem = 0;
		segTable->entries[i].s_kuseg2 = &(uprocs[i].u_pt);

		for (int j = 0; j < 32; j++) {
			unsigned int segNo = 2 << 30;
			unsigned int vpn = 0x8000 << 12;
			unsigned int shiftedASID = (i + 1) << 6;

			uprocs[i].u_pt.p_entries[j].p_HI = segNo + vpn + shiftedASID;

			uprocs[i].u_pt.p_entries[j].p_LO = 1 << 10;
		}

		/* s_sp same as process's sys stack page */
		initialState->s_sp = something; /* change */
		initialState->s_pc = stub;
		initialState->s_t9 = stub;

		initialState->s_status = INTSUNMASKED | PROCLOCALTIMEON | VMOFF | KERNELON;

		SYSCALL(1, initialState);
	}
}

void stub(){
	int asid = getASID();

	/* 3 sys 5s*/
	SYSCALL(5, 0, &(states[(asid - 1)*3]), &(states[(asid - 1)*3]));
	SYSCALL(5, 1, &(states[(asid - 1)*3 +1]), &(states[(asid - 1)*3 +1]));
	SYSCALL(5, 2, &(states[(asid - 1)*3 +2]), &(states[(asid - 1)*3 +2]));

	readFromTape();

	/* LDST */
}

void readBlockFromTape(int asid){
	

}

int getBufferIndex(int isItDisk, int devNum){
	return (8*isItDisk + devNum);
}

void copyBufferToBuffer(fromIndex, toIndex){
	for (int i = 0; i < 1024; i++){
		bufferArray[toIndex]->p_words[i] = bufferArray[fromIndex]->p_words[i]; /* might need some pointers */
	}
}



void readFromTape(){
	int asid = getASID();
	int tapeDeviceNumber = asid - 1;
	int diskDeviceNumber = 0;

	devregarea_t *foo = (devregarea_t *) RAMBASEADDR;

	int devRegIndex = getDevRegIndex(4, tapeDeviceNumber);
    device_t * tapeDevReg = &(foo->devreg[devRegIndex]); /* gain addressability to tape device's device registers */

    devRegIndex = getDevRegIndex(3, diskDeviceNumber);
    device_t * diskDevReg = &(foo->devreg[devRegIndex]);

	if (tapeDevReg->d_status == 1 && tapeDevReg->d_data1 == 0){
		PANIC(); /* no tape inserted */
	}

	int track = ((asid - 1)*32)/256;

	int sector = (asid*32) % 256;

	while(tapeDevReg->d_data1 != 0 || tapeDevReg->d_data1 != 1) { /* if not at end of tape/file */

		unsigned int origStatus = getSTATUS();

		unsigned int newStatus = ~origStatus;
		newStatus = newStatus | IECON;
		newStatus = ~newStatus;

		setSTATUS(newStatus);

		tapeDevReg->d_data0 = &(bufferArray[getBufferIndex(0, tapeDeviceNumber)]);
		tapeDevReg->d_command = 3;

		setSTATUS(origStatus);

		SYSCALL(WAITIO, 4, asid-1, FALSE); /* block read into tape buffer */

		SYSCALL(PASSERN, &(diskBufferMutexes[0])); /* gain mutual exclusion on disk 0 buffer */

		copyBufferToBuffer(bufferArray[getBufferIndex(0, tapeDeviceNumber)], bufferArray[getBufferIndex(1, 0)]);

		/* check for device ready? */

		/* assume mutex on disk device registers */

		unsigned int origStatus = getSTATUS();

		newStatus = ~origStatus;
		newStatus = newStatus | IECON;
		newStatus = ~newStatus;

		setSTATUS(newStatus);

		diskDevReg->d_data0 = 0;
		diskDevReg->d_command = 2;

		setSTATUS(origStatus);

		SYSCALL(WAITIO, 3, 0, FALSE);

		unsigned int origStatus = getSTATUS();

		newStatus = ~origStatus;
		newStatus = newStatus | IECON;
		newStatus = ~newStatus;

		diskDevReg->d_data0 = &(bufferArray[getBufferIndex(1, 0)]);
		diskDevReg->d_command = 4;

		setSTATUS(origStatus);

		SYSCALL(WAITIO, 3, 0, FALSE);

		SYSCALL(VERHOGEN, &(diskBufferMutexes[0]));

		sector++;
	}
}

void tlbHandler(){
	int processId = currentProcess->p_s.s_asid;
	unsigned int cause = TLBMANAGEMENTOLDAREA->s_cause;
	if(cause == 2 || cause == 3){
		unsigned int segno = TLBMANAGEMENTOLDAREA->s_asid >> 30;
		unsigned int vpn = TLBMANAGEMENTOLDAREA->s_asid << 2;
		vpn = vpn >> 14;
		SYSCALL(PASSERN, framePool[processId - 1]);
		for(int i = 0; i < uprocs[processId]-1].u_pt.p_header - 42; i++)
			unsigned int entryHigh = uprocs[processId]-1].u_pt.p_entries[i].p_HI;
			entryHigh = entryHigh << 2;
			entryHigh = entryHigh >> 13;
			unsigned int entryLow = uprocs[processId]-1].u_pt.p_entries[i].p_LO;
			entryLow = entryLow << 22;
			entryLow = entryLow >> 31;
			if(entryHigh == vpn) {
				if(entryLow = 1){
					SYSCALL(VERHOGEN, framePool[processId - 1]);
					LDST(TLBMANAGEMENTOLDAREA);
				}
			}
			int frameToUse = currentFrame % 16;
			if(frameisoccupied){
				//mark PTE entry as invalid
				//write the frame to backingstore
				//read in missing page
				//update swap pool
				//update PTE
				TLBCLR() //clear tlb
				//shave PTE into tlb
				//v whateverpageitis
				//LDST(ugiafdghakjlhjgkldsjhklasdjhklagsdjkn)
			}
			
		}
	}
	else {
		death /* fix */
	}
}

void programTrapHandler(){}

void upperSyscallHandler(){
	int sysNum = currentProcess->p_oldSys->s_a0;
	int kernelMode;
  int kernelStatus = currentProcess->p_oldSys->s_status & KERNELOFF;
  if(kernelStatus == ALLOFF){
   kernelMode = TRUE;
  }
  else{
    kernelMode = FALSE;
  }
	if(kernelMode == TRUE) {
		switch(sysNum){
			case 9:
				readFromTerminal();
				break;
			case 10:
				writeToTerminal(currentProcess->p_oldSys->s_a1, currentProcess->p_oldSys->s_a2);
				break;
			case 11:
				vVirtualSemaphore();
				break;
			case 12:
				pVirtualSemaphore();
				break;
			case 13:
				delay();
				break;
			case 14:
				diskPut();
				break;
			case 15:
				diskGet();
				break;
			case 16:
				writeToPrinter();
				break;
			case 17:
				getTOD();
				break;
			case 18:
				terminate();
				break;
		}
	}
	else {}
}

void writeToTerminal(char *msg, int length) {
	
	char * s = msg;
	devregtr * base = (devregtr *) (TERM0ADDR);
	devregtr status;
	SYSCALL(PASSERN, (int)&term_mut, 0, 0);				/* P(term_mut) */
	for (int i = 0; i < length; i++) {
		*(base + 3) = PRINTCHR | (((devregtr) *s) << BYTELEN);
		status = SYSCALL(WAITIO, TERMINT, 0, 0);	
		if ((status & TERMSTATMASK) != RECVD)
			PANIC();
		s++;	
	}
	SYSCALL(VERHOGEN, (int)&term_mut, 0, 0);				/* V(term_mut) */
	LDST(currentProcess->oldSys);
}

void writeToPrinter(char){}

void getTOD(){
	currentProcess->p_oldSys->s_v0 = getTIMER();
	LDST(currentProcess->oldSys);
}

void terminate(){}