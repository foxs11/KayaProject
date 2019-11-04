#ifndef ASL
#define ASL

/************************** ASL.C ******************************
*
*  Provides methods for maintaining the Active Semaphore List.
*  This is maintained as a linear, singly linked list of semd_t nodes in ascending order by semAdd with a head pointer.
*  There are two dummy nodes to guard against edge cases in removal and insertion.
*  Maintains a semaphore descriptor node free list as a singly linked, linear stack.
*  Supports removal and insertion of process blocks onto the procQs of corresponding semd_ts.
*  Semd_ts whose procQs become empty are removed and added back to the free list.
*
*/

#include "../h/types.h"
#include "../h/const.h"
#include "../e/pcb.e"

static semd_PTR semd_h; /* pointer to the head of the ASL */
static semd_PTR semdFree_h; /* pointer to the head of the semd free list */

/* GLOBAL FUNCTIONS */

/* Insert the ProcBlk pointed to bypat the tail of the process queue
*  associated with the semaphore whose physical address issemAdd
*  and set the semaphore address ofpto semAdd. If the semaphore is
*  currently not active (i.e. there is no descriptor for it in the ASL),
*  allocate a new descriptor from the semdFree list, insert it in the ASL
*  (at the appropriate position), initialize all of thefields (i.e. set
*  s_semAdd to semAdd, and s_procq to mkEmptyProcQ()), and proceed as
*  above. If a new semaphore descriptor needs to be allocated and the
*  semdFree list is empty, return TRUE. In all other cases return FALSE.*/
int insertBlocked (int *semAdd, pcb_PTR p){
  addokbuf("insertBlock 1\n");
  semd_PTR parent = searchSemd(semAdd);
  addokbuf("insertBlock 2\n");
  if (parent->s_next->s_semAdd == semAdd){ /* is there a semd with semAdd on the ASL? */
    addokbuf("insertBlock 3\n");
    insertProcQ(&(parent->s_next->s_procQ), p); /* yes */
    p->p_semAdd = semAdd;
    return FALSE;
  }
  else{ /* semd not found and needs to be allocated */
    addokbuf("insertBlock 4\n");
    semd_PTR newSemd = allocateSemd(semAdd);
    if (newSemd == NULL) {
      addokbuf("insertBlock 5\n");
      return TRUE; /* no more free semd's and insert is blocked */
    }
    addokbuf("insertBlock 6\n");
    newSemd->s_next = parent->s_next; /* there are available semds */
    addokbuf("insertBlock 7\n");
    parent->s_next = newSemd;
    addokbuf("insertBlock 8\n");
    insertProcQ(&(newSemd->s_procQ), p);
    addokbuf("insertBlock 9\n");
    p->p_semAdd = semAdd;
    addokbuf("insertBlock 10\n");
    return FALSE;
  }
}

/* Remove the ProcBlk pointed to bypfrom the process queue associated
*  with p’s semaphore (p→psemAdd) on the ASL. If ProcBlkpointed
*  to by p does not appear in the process queue associated with p’s
*  semaphore, which is an error condition, return NULL; otherwise, return p. */
pcb_PTR outBlocked (pcb_PTR p){
  semd_PTR parent = searchSemd(&(*(p->p_semAdd)));
  if (parent->s_next->s_semAdd != p->p_semAdd) { /* did we find the procBlk on the ASL? */
    return NULL; /* no */
  }
  /* p should be on the right process queue */
  else if (headProcQ(parent->s_next->s_procQ->p_next) == p) { /* are we removing the last pcb? */
    pcb_PTR outedPcb = outProcQ(&(parent->s_next->s_procQ), p);
    semd_PTR tempToBeRemoved = parent->s_next;
    parent->s_next = parent->s_next->s_next;
    freeSemd(tempToBeRemoved);
    return outedPcb;
  }
  else{ /* more than one pcb in procQ */
    return outProcQ(&(parent->s_next->s_procQ), p);
  }
}

/* Search the ASL for a descriptor of this semaphore.  If none is
*  found, return NULL; otherwise, remove the first (i.e. head)
*  ProcBlk from the process queue of the found semaphore descriptor
*  and re-turn a pointer to it. If the process queue for this
*  semaphore becomes empty (emptyProcQ(sprocq)is TRUE), remove the
*  semaphore descriptor from the ASL and return it to the semdFree list. */
pcb_PTR removeBlocked (int *semAdd){
  semd_PTR parent = searchSemd(semAdd);
  if (parent->s_next->s_semAdd != semAdd) { /* is the next node the one we are looking for? */
    return NULL; /* no */
  }
  else { /* yes */
    return outBlocked(headProcQ(parent->s_next->s_procQ));
  }
}

/* Return a pointer to the ProcBlk that is at the head of the process
*  queue associated with the semaphore semAdd. Return NULL if semAdd
*  is not found on the ASL or if the process queue associated with semAdd is empty. */
pcb_PTR headBlocked (int *semAdd){
  semd_PTR parent = searchSemd(semAdd);
  if (parent->s_next->s_semAdd != semAdd) { /* is the next node the one we are looking for? */
    return NULL; /* no */
  }
  else if (emptyProcQ(parent->s_next->s_procQ)){ /* yes, but is the procQ empty? */
    return NULL; /* yes */
  }
  else { /* no */
    return headProcQ(parent->s_next->s_procQ);
  }
}

/* Initialize the semdFree list to contain all the elements of the array
*  staticsemdt semdTable[MAXPROC]. This method will be only called once
*  during data structure initializa-tion. */
void initASL (){
  static semd_t semdTable[MAXPROC+2];
  semd_h = NULL;
  semdFree_h = NULL;
  int i;
  for(i=0; i<MAXPROC+2; i++){
    freeSemd(&semdTable[i]);
  }
  semd_h = allocateSemd(0); /* set up sentinel nodes */
  semd_h->s_next = allocateSemd(MAXINT);
  semd_h->s_next->s_next = NULL;
}

/* LOCAL FUNCTIONS */

/* null out the fields of a semd node that is either about to be allocated
*  or about to be returned to the free list */
HIDDEN void cleanSemd(semd_PTR p){
  p->s_next = NULL;
  p->s_semAdd = NULL;
  p->s_procQ = NULL;
}

/* This function takes a semd as a parameter and searches the ASL for that node.
*  Whether the node in question is on the ASL or not, the function returns
*  the previous node to where the node would be or is. */
HIDDEN semd_PTR searchSemd(int *semd){
  semd_PTR semdListPTR = semd_h;
  while (semdListPTR->s_next->s_semAdd < semd) {
    semdListPTR = semdListPTR->s_next;
  }
  return semdListPTR;
}

/* This function, given a semd, cleans it and returns it to the free list */
HIDDEN void freeSemd(semd_PTR p){
  cleanSemd(p);
  p->s_next = semdFree_h;
  semdFree_h = p;
}

/* This function allocates a new semd from the free list to be later inserted
*  into the ASL. It sets the allocated semd's s_semAdd to the parameter semAdd.
*  Returns null if the free list is empty, and handles the cases of just one
*  on the free list and more than one differently in order to maintain the integrity
*  of the free list. */
HIDDEN semd_PTR allocateSemd(int *semAdd){
  semd_PTR allocatedSemd; 
  if (semdFree_h == NULL) {
    return NULL;
  }
  /* one on free list */
  else if (semdFree_h->s_next == NULL) {
    allocatedSemd = semdFree_h;
    semdFree_h = NULL;
    cleanSemd(allocatedSemd);
    allocatedSemd->s_semAdd = semAdd;
    return allocatedSemd;
  }
  else { /* more than one on free list */
    allocatedSemd = semdFree_h;
    semdFree_h = semdFree_h->s_next;
    cleanSemd(allocatedSemd);
    allocatedSemd->s_semAdd = semAdd;
    return allocatedSemd;
  }
}

/***************************************************************/

#endif
