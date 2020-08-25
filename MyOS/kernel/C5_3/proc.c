#include "types.h"
#include "proc.h"
#include "queue.h"
#include "display.h"

extern PROC *running, *freeList, *readyQueue;
extern void tswitch();

// kfork() creates a new process to exceute func with priority
u32 kfork(u32 func, u32 priority)
{
    u32 i;
    PROC *p = get_proc(&freeList);

    if (p == NULL)
    {
        kprintf("no more PROC, kfork failed\n");
        return -1; // return -1 for FAIL
    }

    p->status = READY;
    p->priority = priority;

    // set kstack for proc resume to execute func()
    for (i = 1; i < 15; i++)
        p->kstack[SSIZE - i] = 0;      // all "saved" regs = 0
    p->kstack[SSIZE - 1] = func;       // resume execution address
    p->ksp = &(p->kstack[SSIZE - 14]); // saved ksp

    enqueue(&readyQueue, p);           // enter p into readyQueue

    kprintf("%d kforked a new proc %d\n", running->pid, p->pid);
    printList("freeList", freeList);
    printList("readyQueue", readyQueue);
    return p->pid;
}

void kexit() // called by process to terminate
{
    kprintf("proc %d kexit\n", running->pid);
    running->status = FREE;
    put_proc(&freeList, running);
    printList("freeList", freeList);
    printList("readyQueue", readyQueue);    
    
    // give up CPU
    // though the saving part of the tswitch will still work on the exiting proc,
    // it is meaningless for a proc that have called kexit().
    tswitch();
}