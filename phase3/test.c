state_t newSys;
state_t newTLB;
state_t newPgm;

pgb_PTR bufferArray[16] = (pgb_PTR) ENDOFOS; /* should this be an array of non-pointers? */
int diskBufferMutexes[8];

pgb_PTR framePool[UPROCNUM*2] = (pgb_PTR) FRAMEPOOL;

st_PTR segTable = (st_PTR) SEGTBLS;


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
	newSys->s_sp = ramTop; /*change */
  	newTLB->s_sp = ramTop;
  	newPgm->s_sp = ramTop;


  	newSys->s_pc = (memaddr) sysCallHandler; /*change*/
  	newTLB->s_pc = (memaddr) pgmTrapHandler;
  	newPgm->s_pc = (memaddr) tlbMgmtHandler;


  	newSys->s_t9 = (memaddr) sysCallHandler; /*change*/
  	newTLB->s_t9 = (memaddr) pgmTrapHandler;
  	newPgm->s_t9 = (memaddr) tlbMgmtHandler;


  	newSys->s_status = INTSUNMASKED | VMON | PROCLOCALTIMEON | KERNELON;
  	newTLB->s_status = INTSUNMASKED | VMON | PROCLOCALTIMEON | KERNELON;
  	newPgm->s_status = INTSUNMASKED | VMON | PROCLOCALTIMEON | KERNELON;


	for (int i = 0; i < 8; i++){
		diskBufferMutexes[i] = 1;
	}

	for (int i = 0; i < UPROCNUM){
		segTable[i].s_ksegOS = &ksegOSPT;
		segTable[i].s_kuseg3 = &kuseg3PT;
	}


	for (int i = 0; i < UPROCNUM; i++){
		state_PTR intialState = NULL;
		initialState->s_asid = i+1;

		uprocs[i].u_pt.p_header = 0;
		int magicNum = 42;
		magicNum << 24;
		uprocs[i].u_pt.p_header += magicNum;
		uprocs[i].u_sem = 0;
		segTable[i].s_kuseg2 = &(uprocs[i].u_pt);
		/* s_sp same as process's sys stack page */
		initalState->s_sp = something; /* change */
		intialState->s_pc = stub;
		intialState->s_t9 = stub;

		/* entryHI something? */

		/* t9? */

		SYSCALL(1, intialState);
	}
}

void stub(){
	int asid = getASID();
	/* init kuseg2 pg table */


	/* 3 sys 5s*/
	SYSCALL(5, 0, ,);
	SYSCALL(5, 1, ,);
	SYSCALL(5, 2, ,);

	readFromTape();

	/* LDST */
}

void readBlockFromTape(int asid){
	

}

int getBufferIndex(int isItDisk, int devNum){
	return (8*isItDisk + devNum);
}

void copyBufferToBuffer(fromIndex, toIndex){
	while (int i = 0; i < 1024; i++){
		bufferArray[toIndex]->p_words[i] = bufferArray[fromIndex]->p_words[i]; /* might need some pointers */
	}
}



void readFromTape(){
	int asid = getASID();
	int tapeDeviceNumber = asid - 1;

	devregarea_t *foo = (devregarea_t *) RAMBASEADDR;

	int devRegIndex = getDevRegIndex(tapeLineNumber, tapeDeviceNumber);
    device_t * tapeDevReg = &(foo->devreg[devRegIndex]); /* gain addressability to tape device's device registers */

	if (tapeDevReg->d_status == 1 && tapeDevReg->d_data1 == 0){
		PANIC(); /* no tape inserted */
	}

	while(tapeDevReg->d_data1 != 0 || tapeDevReg->d_data1 != 1) { /* if not at end of tape/file */

		tapeDevReg->d_data0 = &(bufferArray[getBufferIndex(0, tapeDeviceNumber)]);
		tapeDevReg->d_command = 3;

		SYSCALL(WAITIO, 4, deviceNumber, FALSE); /* block read into tape buffer */

		SYSCALL(PASSERN, &(diskBufferMutexes[0])); /* gain mutual exclusion on disk 0 buffer */

		copyBufferToBuffer(bufferArray[getBufferIndex(0, tapeDeviceNumber)], bufferArray[getBufferIndex(1, 0)]);




	}

	


}
