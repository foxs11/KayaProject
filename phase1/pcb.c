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
sammy
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

}

pcb_PTR mkEmptyProcQ (){
  return NULL;
}

int emptyProcQ (pcb_PTR tp){
  if(*tp == NULL) {
    return TRUE;
  }
  else {
    return FALSE;
  }
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
sammy
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
