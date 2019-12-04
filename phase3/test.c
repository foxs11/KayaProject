state_t newSys;
state_t newTLB;
state_t newPgm;

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

int p3

/* kseg0s page tables */

int ksegOSTable[]

void test(){



	for (int i = 0; i < 8; i++){
		state_PTR intialState = NULL;
		/* s_sp same as process's sys stack page */
		initalState->s_sp = something; /* change */
		intialState->s_pc = stub;

		/* t9? */

		SYSCALL(1, intialState);
	}
}

void stub(){
	asid = getASID();
	/* init kuseg2 pg table */

	/* 3 sys 5s*/

	/* read from tape onto backing store */

	/* LDST */
}

































struct pte_t {
	unsigned int HI,
	unsigned int LO
} pte_t, pte_PTR *;
