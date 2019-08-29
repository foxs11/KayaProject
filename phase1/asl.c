#ifndef ASL
#define ASL

/************************** ASL.E ******************************
*
*  The externals declaration file for the Active Semaphore List
*    Module.
*
*  Written by Mikeyg
*/

#include "../h/types.h"
#include "../h/const.h"
#include "../e/pcb.e"
#include "pcb.c"

/* static semd_PTR *semd_h;
static semd_PTR *semdFree_h;

extern int insertBlocked (int *semAdd, pcb_PTR p){
  if(seachSemd(semAdd)){
    insertProcQ(semAdd->s_procQ, p);
  }
  else{
    allocateSemd(semAdd);
    insertProcQ(semAdd->s_procQ, p);
  }
} /*search active semdList if found: insertProcQ(p and tp found in semd) if not found: allocate new semd, put new node into active list, perform found */

/* extern pcb_PTR removeBlocked (int *semAdd){}

extern pcb_PTR outBlocked (pcb_PTR p){
} /*search active semdList if not found: error case, if found: outProcQ on the process queue, value is returned. If processqueue not empty: your done, if it is empty: deallocate this semd node */

/* extern pcb_PTR headBlocked (int *semAdd){chris}

extern void initASL (){chris}

/* these where suggested to be made in the videos */
/* HIDDEN allocateSemd(){}

HIDDEN freeSemd(){}

HIDDEN int searchSemd(int *semd){
  semd_PTR semdListPTR = semd_h;
  while(semd != NULL){
    if(semd == semdListPTR){
      return TRUE;
    }
    else{
      semdListPTR = semdListPTR->s_next; 
    }
  }
  return FALSE;
} */

/***************************************************************/

#endif
