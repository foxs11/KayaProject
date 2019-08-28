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
#include "../h/const.h"

static pcb_PTR pcbFree_h;

void freePcb (pcb_PTR p){
  cleanPcb(p);
  // if(emptyProcQ(pcbFree_h)){
  //   pcbFree_h = p;
  // }
  // else{
  //   pcbFree_h->p_next->p_prev = p;
  //   p->p_next = pcbFree_h->p_next;
  //   p->p_prev = pcbFree_h;
  //   pcbFree_h->p_next = p;
  //   pcbFree_h = p;
  // }
  insertProcQ(&pcbFree_h, p);
}

pcb_PTR allocPcb (){
  if(emptyProcQ(pcbFree_h)){
    return NULL;
  }
  else{
    pcb_PTR allocatedPcb = removeProcQ(&pcbFree_h);
    return cleanPcb(allocatedPcb);
  }
}

HIDDEN cleanPcb(pcb_PTR x){
  x->p_next = NULL;
  x->p_prev = NULL;
  x->p_child = NULL;
  x->p_sib = NULL;
  x->p_prevsib = NULL;
  x->p_prnt = NULL;
}

void initPcbs (){
  static pcb_t procTable[MAXPROC];
  for(int i=0; i<MAXPROC; i++){
    freePcb(&procTable[i]);
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
  return outProcQ(tp, *tp->p_next);
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
	return (p->p_child == NULL);
}

void insertChild (pcb_PTR prnt, pcb_PTR p){
  if(emptyChild(prnt)){
    prnt->p_child = p;
  }
  else{
    prnt->p_child->p_prevsib = p;
    p->p_sib = prnt->p_child;
    p->p_prnt = prnt;
    prnt->p_child = p;
  }
}

pcb_PTR removeChild (pcb_PTR p){
  return outChild()
}

pcb_PTR outChild (pcb_PTR p){ // wait wtf shouldn't there be 2 parameters?
  if (p->p_prnt == NULL) {
		return NULL;
  }
  else {
		if (p->p_sib == NULL && p->p_prevsib == NULL) { // is p an only child?
			p->p_prnt->p_child = NULL;
		}
		else { // not only child
			if (p->p_sib == NULL) { // is p the last child?
				p->p_prevsib->p_sib = NULL;
			}
			else if (p->p_prevsib == NULL) { //is p the first child?
				p->p_prnt->p_child = p->p_sib;
				p->p_sib->p_prevsib = NULL;
			}
			else { // middle child
				p->p_sib->p_prevsib = p->p_prevsib;
				p->p_prevsib->p_sib = p->p_sib;
			}
		}
		return p;
	}
}

/***************************************************************/

#endif