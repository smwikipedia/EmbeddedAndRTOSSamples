#ifndef PROC_H
#define PROC_H

#include "types.h"

#define NPROC 5
#define SSIZE 1024

// PROC status
#define FREE 1
#define READY 2
#define SLEEP 3

//PROC priority, smaller value means lower priority
#define PRIORITY_0 0
#define PRIORITY_1 1

typedef struct proc
{
    struct proc *next;
    u32 *ksp;
    u32 pid;
    u32 status;
    u32 event;
    u32 priority;
    u32 kstack[SSIZE];
} PROC;

u32 kfork(u32 func, u32 priority);
void kexit();


#endif