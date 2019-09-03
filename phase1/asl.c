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

static semd_PTR semd_h;
static semd_PTR semdFree_h;

extern int insertBlocked (int *semAdd, pcb_PTR p){
  if(seachSemd(semAdd)){
    insertProcQ(getsemdPTR(semAdd)->s_procQ, p);
    p->p_semAdd = *semAdd;
  }
  else{
    allocateSemd(semAdd);
    insertProcQ(getsemdPTR(semAdd)->s_procQ, p);
    p->p_semAdd = *semAdd;
  }
} /* search active semdList if found: insertProcQ(p and tp found in semd) if not found: allocate new semd, put new node into active list, perform found */

extern pcb_PTR removeBlocked (int *semAdd){
  if(!searchSemd(semAdd)){
    return NULL;
  }
  else{
    return outBlocked(getsemdPTR(semAdd)->s_procQ->p_next);
  }
}

extern pcb_PTR outBlocked (pcb_PTR p){
} /* search active semdList if not found: error case, if found: outProcQ on the process queue, value is returned. If processqueue not empty: your done, if it is empty: deallocate this semd node */

extern pcb_PTR headBlocked (int *semAdd){
  if(!searchSemd(semAdd)){
    return NULL;
  }
  else{
    if(getsemdPTR(semAdd)->s_procQ == NULL){
      return NULL;
    }
    else{
      return getsemdPTR(semAdd)->s_procQ->p_next;
    }
  }
}

extern void initASL (){
  static semd_t semdTable[MAXPROC];
  int i;
  for(i=0; i<MAXPROC; i++){
    freeSemd(&semdTable[i]);
  }
}

/* these where suggested to be made in the videos */
HIDDEN allocateSemd(int *semAdd){
  
}

HIDDEN freeSemd(semd_PTR p){
  cleanSemd(p);
  p->s_next = semdFree_h;
  semdFree_h = p;
}

HIDDEN int searchSemd(int *semd){
  semd_PTR semdListPTR = semd_h;
  while(semdListPTR != NULL){
    if(semd == semdListPTR){
      return TRUE;
    }
    else{
      semdListPTR = semdListPTR->s_next; 
    }
  }
  return FALSE;
}

HIDDEN semd_PTR getsemdPTR(int *semd){
  semd_PTR semdListPTR = semd_h;
  while(semdListPTR != NULL){
    if(semd == semdListPTR->s_semAdd){
      return semdListPTR;
    }
    else{
      semdListPTR = semdListPTR->s_next; 
    }
  }
  return FALSE;
}

/***************************************************************/

#endif
