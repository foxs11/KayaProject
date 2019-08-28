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

static pcb_PTR semd_h;
static pcb_PTR semdFree_h;

extern int insertBlocked (int *semAdd, pcb_PTR p){sammy}
extern pcb_PTR removeBlocked (int *semAdd){chris}
extern pcb_PTR outBlocked (pcb_PTR p){sammy}
extern pcb_PTR headBlocked (int *semAdd){chris}
extern void initASL (){chris}

/***************************************************************/

#endif
