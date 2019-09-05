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

HIDDEN semd_PTR allocateSemd(int semAdd){
  if (semdFree_h == NULL) {
    return NULL;
  }
  else {
    /* one on free list */
    if (semdFree_h->s_next == NULL) {
      semd_PTR allocatedSemd = semdFree_h;
      semdFree_h = NULL;
      allocatedSemd->s_semAdd = &semAdd;
      return allocatedSemd;
    }
    else { /* more than one on free list */
      semd_PTR allocatedSemd = semdFree_h;
      semdFree_h = semdFree_h->s_next;
      allocatedSemd->s_semAdd = &semAdd;
      return allocatedSemd;
    }
  }
}

int insertBlocked (int *semAdd, pcb_PTR p){
  semd_PTR parent = searchSemd(semAdd);
  if(parent->s_next->s_semAdd == semAdd){
    addokbuf("bad insert\n");
    insertProcQ(&(parent->s_next->s_procQ), p);
    p->p_semAdd = semAdd;
    return FALSE;
  }
  else{ /* semd not found and needs to be allocated */
    addokbuf("good insert\n");
    semd_PTR newSemd = allocateSemd(*semAdd);
    if (newSemd == NULL) {
      return TRUE; /* no more free semd's and insert is blocked */
    }
    addokbuf("good insert2\n");
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
