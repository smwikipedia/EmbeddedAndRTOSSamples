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
#define PRIORITY_2 2

typedef struct proc
{
    struct proc *next;

    /*
    context fields
    */
    u32 *ksp;
    u32 r0;
    u32 r1;
    u32 r2;
    u32 r3;
    u32 r4;
    u32 r5;
    u32 r6;
    u32 r7;
    u32 r8;
    u32 r9;
    u32 r10;
    u32 r11;
    u32 r12;
    u32 lr;

    u32 pid;
    u32 status;
    u32 event;
    u32 priority;
    u32 kstack[SSIZE];
} PROC;

u32 kfork(u32 func, u32 priority);
void kexit();


#endif