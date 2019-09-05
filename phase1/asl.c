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

HIDDEN void cleanSemd(semd_PTR p){
  p->s_next = NULL;
	p->s_semAdd = NULL;
  p->s_procQ = NULL;
}

HIDDEN semd_PTR searchSemd(int *semd){
  semd_PTR semdListPTR = semd_h;
  while (semdListPTR->s_next->s_semAdd < semd) {
    semdListPTR = semdListPTR->s_next;
  }
  return semdListPTR;
}

HIDDEN void freeSemd(semd_PTR p){
  cleanSemd(p);
  p->s_next = semdFree_h;
  semdFree_h = p;
}

HIDDEN semd_PTR allocateSemd(int *semAdd){
  semd_PTR allocatedSemd; 
  if (semdFree_h == NULL) {
    addokbuf("in allocate\n");
    return NULL;
  }
  /* one on free list */
  else if (semdFree_h->s_next == NULL) {
    addokbuf("in allocate1\n");
    allocatedSemd = semdFree_h;
    addokbuf("in allocate2\n");
    semdFree_h = NULL;
    addokbuf("in allocate3\n");
    allocatedSemd->s_semAdd = semAdd;
    addokbuf("in allocate4\n");
    return allocatedSemd;
  }
  else { /* more than one on free list */
    addokbuf("in allocate5\n");
    allocatedSemd = semdFree_h;
    addokbuf("in allocate6\n");
    semdFree_h = semdFree_h->s_next;
    addokbuf("in allocate7\n");
    allocatedSemd->s_semAdd = semAdd;
    addokbuf("in allocate8\n");
    return allocatedSemd;
  }
}

int insertBlocked (int *semAdd, pcb_PTR p){
  semd_PTR parent = searchSemd(semAdd);
  addokbuf("in insertBlocked\n");
  if(parent->s_next->s_semAdd == semAdd){
    addokbuf("in insertBlocked1\n");
    insertProcQ(&(parent->s_next->s_procQ), p);
    p->p_semAdd = semAdd;
    return FALSE;
  }
  else{ /* semd not found and needs to be allocated */
    semd_PTR newSemd = allocateSemd(semAdd);
    if (newSemd == NULL) {
      addokbuf("in insertBlocked2\n");
      return TRUE; /* no more free semd's and insert is blocked */
    }
    addokbuf("in insertBlocked3\n");
    newSemd->s_next = parent->s_next;
    parent->s_next = newSemd;
    insertProcQ(&(newSemd->s_procQ), p);
    p->p_semAdd = semAdd;
    return FALSE;
  }
} /* search active semdList if found: insertProcQ(p and tp found in semd) if not found: allocate new semd, put new node into active list, perform found */

pcb_PTR outBlocked (pcb_PTR p){
  semd_PTR parent = searchSemd(&(*(p->p_semAdd)));
  if (parent->s_next->s_semAdd != p->p_semAdd) {
    return NULL;
  }
  else { /* p should be on the right process queue */
    if (headProcQ(parent->s_next->s_procQ) == p) { /* are we removing the last pcb? */
      pcb_PTR outedPcb = outProcQ(&(parent->s_next->s_procQ), p);
      parent->s_next = parent->s_next->s_next;
      freeSemd(parent->s_next);
      return outedPcb;
    }
    else{
      return outProcQ(&(parent->s_next->s_procQ), p);
    }
  }
} /* search active semdList if not found: error case, if found: outProcQ on the process queue, value is returned. If processqueue not empty: your done, if it is empty: deallocate this semd node */

pcb_PTR removeBlocked (int *semAdd){
  semd_PTR parent = searchSemd(semAdd);
  if (parent->s_next->s_semAdd != semAdd) {
    addokbuf("in removeBlocked null return\n");
    return NULL;
  }
  else {
    addokbuf("in removeBlocked2\n");
    return outBlocked(headProcQ(parent->s_next->s_procQ));
  }
}


pcb_PTR headBlocked (int *semAdd){
  semd_PTR parent = searchSemd(semAdd);
  if (*(parent->s_next->s_semAdd) != *semAdd) {
    return NULL;
  }
  else {
    return headProcQ(parent->s_next->s_procQ);
  }
}

void initASL (){
  static semd_t semdTable[MAXPROC+2];
  int i;
  for(i=0; i<MAXPROC+2; i++){
    freeSemd(&semdTable[i]);
  }
  semd_h = allocateSemd(0);
  semd_h->s_next = allocateSemd(MAXINT);
}

/***************************************************************/

#endif
