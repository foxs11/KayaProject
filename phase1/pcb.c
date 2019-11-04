#ifndef PCB
#define PCB

/************************* PROCQ.C *****************************
*
*  Defines methods for maintaining a process queue service.
*  A process queue consists of a doubly linked, circular queue with a tail pointer.
*  Empty queues are defined by a NULL tail pointer.
*  Maintains a process block free queue where unallocated process blocks wait. 
*  This is maintained as a normal process queue without any child trees.
*  Provides methods for insertion and removal from process queues, as well from child trees.
*  Child trees are maintained as doubly linked, linear stacks with a head pointer. 
*/

#include "../h/types.h"
#include "../h/const.h"

void stupidDebug (unsigned int a){
  int a = 0;
}

static pcb_PTR pcbFree_h; /* the tail pointer into the pcb free list */

/* GLOBAL FUNCTIONS */

/* Return TRUE if the queue whose tail is pointed to bytpis empty.
*  Return FALSE otherwise. */
int emptyProcQ (pcb_PTR tp){
  return(tp == NULL);
}

/* Insert the ProcBlk pointed to bypinto the process queue whose
*  tail-pointer is pointed to bytp. Note the double indirection through
*  tp to allow for the possible updating of the tail pointer as well. */
void insertProcQ (pcb_PTR *tp, pcb_PTR p){
  addokbuf("insertProcQ 1\n");
  if(emptyProcQ(*tp)){
    addokbuf("insertProcQ 2\n");
    *tp = p;
    addokbuf("insertProcQ 3\n");
    p->p_prev = p;
    addokbuf("insertProcQ 4\n");
    p->p_next = p;
    addokbuf("insertProcQ 5\n");
  }
  else{
    addokbuf("insertProcQ 6\n");
    (*tp)->p_next->p_prev = p; /* head prev */
    addokbuf("insertProcQ 7\n");
    p->p_next = (*tp)->p_next;  /* new node next */
    addokbuf("insertProcQ 8\n");
    (*tp)->p_next = p; /* old tail next*/
    addokbuf("insertProcQ 9\n");
    p->p_prev = *tp; /* new node prev*/
    addokbuf("insertProcQ 10\n");
    *tp = p; /* new tail pointer*/
    addokbuf("insertProcQ 11\n");
  }
}

/* Insert the element pointed to byponto the pcbFree list. */
void freePcb (pcb_PTR p){
  cleanPcb(p);
  insertProcQ(&pcbFree_h, p);
}

/* Remove the ProcBlk pointed to bypfrom the process queue whose
*  tail-pointer is pointed to by tp.  Update the process queue’s tail
*  pointer if necessary. If the desired entry is not in the indicated
*  queue(an error condition), return NULL; otherwise, return p. Note 
*  that p can point to any element of the process queue. */
pcb_PTR outProcQ (pcb_PTR *tp, pcb_PTR p){
  pcb_PTR tailPointer = *tp;
  if(emptyProcQ(tailPointer)){ /* is the queue empty? if so return null */
    return NULL;
  }
  else if(p==*tp){ /* is p the tp? */
    if(headProcQ(tailPointer) == tailPointer){ /* is p the only procBlk on the queue? */
      *(tp) = NULL;
    }
    else{ /* p is not only node in the queue but it is the tp */
      (*tp)->p_next->p_prev = (*tp)->p_prev;
      (*tp)->p_prev->p_next = (*tp)->p_next;
      *tp = (*tp)->p_prev;
    }
    return (tailPointer);
  }
  else{ /* p is not the tp so search the queue for p */
    pcb_PTR current_PTR = tailPointer->p_next;
    while(current_PTR != tailPointer){
      if(current_PTR != p){ /* p not found yet */
        current_PTR = current_PTR->p_next;
      }
      else{ /* found p */
        current_PTR->p_next->p_prev = current_PTR->p_prev;
        current_PTR->p_prev->p_next = current_PTR->p_next;
        return current_PTR;
      }
    }
    return NULL; /* queue searched and p was not found */
  }
  /* shouldn't get here but was put here to deal with some edge case */
  return NULL;
}

/* Remove the first (i.e. head) element from the process queue whose
*  tail-pointer is pointed to bytp. Return NULL if the process queue
*  was initially empty; otherwise return the pointer to the removed element.
*  Update the process queue’s tail pointer if necessary. */
pcb_PTR removeProcQ (pcb_PTR *tp){
  if(emptyProcQ(*tp)){ /* is the queue empty? */
    stupidDebug(0);
    return NULL;
  }
  else{ /* queue is not empty, remove the head via outProcQ */
    return outProcQ(tp, (*tp)->p_next);
  }
}

/* Return NULL if the pcbFree list is empty. Otherwise, remove
*  an element from the pcbFree list, provide initial values for ALL
*  of the ProcBlk’sfields (i.e. NULL and/or 0) and then return a
*  pointer to the removed element. ProcBlk’s get reused, so it is
*  important that no previous value persist in a ProcBlk when it gets reallocated. */
pcb_PTR allocPcb (){
  if(emptyProcQ(pcbFree_h)){ /* is the free list empty? */
    return NULL;
  }
  else{ /* free list is not empty, so allocate as normal */
    pcb_PTR allocatedPcb = removeProcQ(&pcbFree_h);
    cleanPcb(allocatedPcb);
    return allocatedPcb;
  }
}

/* Initialize the pcbFree list to contain all the elements of the
*  static array of MAXPROC ProcBlk’s. This method will be called
*  only once during data structure initialization. */
void initPcbs (){
  pcbFree_h = NULL;
  static pcb_t procTable[MAXPROC]; /* statically generate 20 pcbs */
  int i;
  for(i=0; i<MAXPROC; i++){
    freePcb(&procTable[i]);
  }
}

/* This method is used to initialize a variable to be tail pointer to a
*  process queue.Return a pointer to the tail of an empty process queue; i.e. NULL. */
pcb_PTR mkEmptyProcQ (){
  return (NULL);
}

/* Return a pointer to thefirst ProcBlk from the process queue whose
*  tail is pointed to bytp. Do not remove this ProcBlkfrom the process
*  queue. Return NULL if the process queue is empty. */
pcb_PTR headProcQ (pcb_PTR tp){
  if(emptyProcQ(tp)){
    return NULL;
  }
  else{
    return tp->p_next;
  }
}

/* Return TRUE if the ProcBlk pointed to byphas no children. Return FALSE otherwise. */
int emptyChild (pcb_PTR p){
	return (p->p_child == NULL);
}

/* Make the ProcBlk pointed to bypa child of the ProcBlk pointed to by prnt. */
void insertChild (pcb_PTR prnt, pcb_PTR p){
  if(emptyChild(prnt)){ /* does the parent have any children already? */
    prnt->p_child = p; /* it does not */
    p->p_prnt = prnt;
  }
  else{ /* it does */
    prnt->p_child->p_prevsib = p;
    p->p_sib = prnt->p_child;
    p->p_prnt = prnt;
    prnt->p_child = p;
  }
}

/* Make the ProcBlk pointed to byp no longer the child of
*   its parent.If the ProcBlk pointed to byphas no parent,
*  return NULL; otherwise, return p. Note that the element 
*  pointed to bypneed not be thefirstchild of its parent. */
pcb_PTR outChild (pcb_PTR p){
  if (p->p_prnt == NULL) { /* is p actually a child? */
		return NULL;
  }
  else { /* p is indeed a child */
		if (p->p_sib == NULL && p->p_prevsib == NULL) { /* is p an only child? */
			p->p_prnt->p_child = NULL;
		}
		else { /* not only child */
			if (p->p_sib == NULL) { /* is p the last child? */
				p->p_prevsib->p_sib = NULL;
			}
			else if (p->p_prevsib == NULL) { /*is p the first child? */
				p->p_prnt->p_child = p->p_sib;
				p->p_sib->p_prevsib = NULL;
			}
			else { /* middle child */
				p->p_sib->p_prevsib = p->p_prevsib;
				p->p_prevsib->p_sib = p->p_sib;
			}
		}
    p->p_prnt = NULL;
		return p;
	}
}

/* Make thefirst child of the ProcBlk pointed to by p no
*  longer a child of p. Return NULL if initially there were
*  no children ofp.Otherwise, return a pointer to this removed first child ProcBlk. */
pcb_PTR removeChild (pcb_PTR p){
  if(emptyChild(p)){ /* does p have children? */
    return NULL;
  }
  return outChild(p->p_child);
}

/* LOCAL FUNCTIONS */

/* Null out the fields of ProcBlk pointed to by p so as to avoid having
* newly allocated or newly deallocated ProcBlks not evidence of their old relationships */
HIDDEN void cleanPcb(pcb_PTR x){
  x->p_next = NULL;
  x->p_prev = NULL;
  x->p_child = NULL;
  x->p_sib = NULL;
  x->p_prevsib = NULL;
  x->p_prnt = NULL;
  x->p_semAdd = NULL;
  x->p_oldSys = NULL;
  x->p_newSys = NULL;
  x->p_oldPgm = NULL;
  x->p_newPgm = NULL;
  x->p_oldTLB = NULL;
  x->p_newTLB = NULL;
  x->p_time = 0;
}
/***************************************************************/

#endif
