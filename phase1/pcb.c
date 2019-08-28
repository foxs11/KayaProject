#ifndef PCB
#define PCB

/************************* PROCQ.E *****************************
*
*  The externals declaration file for the Process Control Block
*    Module.
*
*  Written by Mikeyg
*/

#include "../h/types.h"

static pcb_PTR freeList;

void freePcb (pcb_PTR p){
  cleanPcb(p);
  if(emptyProcQ(freeList)){
    freeList = p;
  }
  else{
    freeList->p_next->p_prev = p;
    p->p_next = freeList->p_next;
    p->p_prev = freeList;
    freeList->p_next = p;
    freeList = p;
  }
}

pcb_PTR allocPcb (){
  if(emptyProcQ(freeList)){
    return NULL;
  }
  else{
    pcb_PTR allocatedPcb = removeProcQ(&freeList);
    return cleanPcb(allocatedPcb);
  }
}

HIDDEN cleanPcb(pcb_PTR x){
  x->p_next = NULL;

chris

}
void initPcbs (){
  static pcb_t procTable[MAXPROC];
  for(i=0; i<MAXPROC; i++){
    freePCB(&procTable[1]);
  }
}

pcb_PTR mkEmptyProcQ (){
  return (NULL);
}

int emptyProcQ (pcb_PTR tp){
  return(tp==NULL);
}

void insertProcQ (pcb_PTR *tp, pcb_PTR p){
  if(emptyProcQ(*tp)){
    *tp = p;
    p->p_prev = p;
    p->p_next = p;
  }
  else{
    *tp->p_next->p_prev = p; //head prev
    p->p_next = *tp->p_next; // new node next
    *tp->p_next = p; // old tail next
    p->p_prev = *tp; // new node prev
    *tp = p; // new tail pointer
  }
}

pcb_PTR removeProcQ (pcb_PTR *tp){
  if(emptyProcQ(*tp)){
    return NULL;
  }
  else{
    return outProcQ(tp, *tp->p_next);
  }
}

pcb_PTR outProcQ (pcb_PTR *tp, pcb_PTR p){
  pcb_PTR tailPointer = *tp;
  if(emptyProcQ(*tp)){
    return NULL;
  }
  else if(p==*tp){
    *tp->p_next->p_prev = *tp->p_prev;
    *tp->p_prev->p_next = *tp->p_next;
    **tp = *tp->p_prev;
    return *tailPointer;
  }
  else{
    current_PTR = *tp->p_next;
    while(current_PTR != *tailPointer){
      if(current_PTR != p){
        current_PTR = current_PTR->p_next;
      }
      else{
        *current_PTR->p_next->p_prev = *current_PTR->p_prev;
        *current_PTR->p_prev->p_next = *current_PTR->p_next;
        return current_PTR;
      }
    }
    return NULL;
  }
}

pcb_PTR headProcQ (pcb_PTR tp){
  if(emptyProcQ(tp)){
    return NULL;
  }
  else{
    return tp->p_next;
  }
}

int emptyChild (pcb_PTR p){
chris
}

void insertChild (pcb_PTR prnt, pcb_PTR p){
sammy
}

pcb_PTR removeChild (pcb_PTR p){
chris
}

pcb_PTR outChild (pcb_PTR p){
chris
}

/***************************************************************/

#endif
