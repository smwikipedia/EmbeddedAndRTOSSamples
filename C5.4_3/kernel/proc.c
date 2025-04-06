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
        return RET_FAIL; // return RET_FAIL for FAIL
    }

    p->status = READY;
    p->priority = priority;

    /*
    Prepare the content of the kstack of the obtained proc.
    So it can resume to execute func().
    For task 0, which is the initial task and has already started,
    it doesn't need to prepare its stack content for fake resumption.
    */
    // for (i = 1; i < 15; i++)
    //     p->kstack[SSIZE - i] = 0;      // all "saved" regs = 0, here we save 14 registers because we save {r0-r12, lr} in tswitch function. 
    // p->kstack[SSIZE - 1] = func;       // resume execution address, the last one is the stack bottom (high addr) to hold the "lr", which is for the resume.
    // p->ksp = &(p->kstack[SSIZE - 14]); // set ksp to point to the stack top (low addr), 14 because we pretend to have saved {r0-r12, lr} in the stack.

    p->ksp = &(p->kstack[SSIZE - 1]);
    p->r0 = 0;
    p->r1 = 0;
    p->r2 = 0;
    p->r3 = 0;
    p->r4 = 0;
    p->r5 = 0;
    p->r6 = 0;
    p->r7 = 0;
    p->r8 = 0;
    p->r9 = 0;
    p->r10 = 0;
    p->r11 = 0;
    p->r12 = 0;
    p->lr = func;


    // p is ready, but it is not started yet
    enqueue(&readyQueue, p);

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
    tswitch(); // this is one of the places that task swich should be carried out.
}