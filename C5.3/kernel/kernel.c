#include "types.h"
#include "debug.h"
#include "versatilepb.h"
#include "pl190.h"
#include "pl011.h"
#include "pl110.h"
#include "sp804.h"
#include "pl050.h"
#include "display.h"
#include "proc.h"
#include "queue.h"

extern void tswitch();

PROC proc[NPROC];
PROC *running, *freeList, *readyQueue;
u32 procsize = sizeof(PROC);

u32 body()
{
    u8 c;
    //color = running->pid;
    kprintf("proc %d resume to body()\n", running->pid);
    while (1)
    {
        kprintf("proc %d in body() input a char [s|f|x] : ", running->pid);
        c = kgetc();
        kprintf("%c\n", c);
        switch (c)
        {
        case 's':
            tswitch();
            break;
        case 'f':
            kfork((u32)body, PRIORITY_1);
            break;
        case 'x':
            kexit();
            break;
        }
    }
}

u32 kernel_init()
{
    u32 i, j;
    PROC *p;
    kprintf("kernel_init()\n");    

    for (i=0; i<NPROC; i++)
    {
        p = &proc[i];
        p->pid = i;
        p->status = FREE;
        p->next = p + 1;
    }
    proc[NPROC-1].next = NULL;
    
    freeList = &proc[0]; // all PROCs in freeList
    readyQueue = NULL; // readyQueue empty

    kprintf("before start proc[0]\n");
    printList("freeList", freeList);
    printList("readyQueue", readyQueue);    
    
    // create P0 as initial running process
    // when kernel init, get_proc() will always return proc[0].
    // But we should use get_proc() instead of &proc[0] because we need to maintain the freeList.
    p = get_proc(&freeList);
    ASSERT(p == &proc[0]);
    p->priority = PRIORITY_0; // P0 has the lowest priority 0
    p->status = READY;
    running = p;

    kprintf("after start proc[0]\n");
    kprintf("running = %d\n", running->pid);
    printList("freeList", freeList);
    printList("readyQueue", readyQueue);
}

u32 scheduler()
{
    kprintf("proc %d in scheduler\n", running->pid);
    if (running->status == READY) // if the running is still ready, enqueue it for next slice.
        enqueue(&readyQueue, running);
    running = dequeue(&readyQueue);
    kprintf("next running = %d\n", running->pid);
    printList("freeList", freeList);
    printList("readyQueue", readyQueue);    
}

u32 main()
{
    u8 c;
    //fbuf_init(); // initialize LCD driver
    //kbd_init();  // initialize KBD driver

    board_init();    

    kprintf("Thank you! Professor K.C. Wang.\n");
    kernel_init();
    
    kfork((int)body, PRIORITY_1); // proc[0] create proc[1] into readyQueue, with priority 1

    while (1)
    {
        while (readyQueue == NULL)
        {
            kprintf("proc %d with priority %d in body() input a char [s|f] : ", running->pid, running->priority);
            c = kgetc();
            kprintf("%c\n", c);
            switch (c)
            {
            case 's':
                tswitch();
                break;
            case 'f':
                kfork((u32)body, PRIORITY_1);
                break;
            // if readyQueue is empty, then proc[0] serve as the IDLE task. It should never exit.
            // the enqueue method will ensure that proc[0] is always the last element for its priority.
            // case 'x':
            //     kexit();
            //     break;
            }
        }
        tswitch();
    }
}

void copy_vectors()
{
    extern u32 vectors_start, vectors_end;
    u32 *vectors_src = &vectors_start;
    u32 *vectors_dst = (u32 *)0;
    while (vectors_src < &vectors_end)
    {
        *vectors_dst++ = *vectors_src++;
    }
}


void IRQ_handler()
{
    u32 vicstatus = VIC_STATUS;
    u32 sicstatus = SIC_STATUS;

    //UART 0
    if (vicstatus & UART0_IRQ_VIC_BIT)
    {
        uart_handler(&uart[0]);
    }

    //UART 1
    if (vicstatus & UART1_IRQ_VIC_BIT)
    {
        uart_handler(&uart[1]);
    }

    // VIC status BITs: timer0,1=4, uart0=13, uart1=14
    if (vicstatus & TIMER01_IRQ_VIC_BIT)
    { // bit4=1:timer0,1, handle timer 0 and 1 one by one
        // KC.Wang's book use TVALUE (Current Value Register) in below line to check which timer trigger the interrupt. It seems not reliable.
        // The Masked Interrupt Status Register is much better.
        // Ref: https://stackoverflow.com/questions/61575520/inconsistent-irq-frequency-with-sp804-dual-timer-module-on-qemu-arm-versatilepb
        if (*(timer[0].base + TMIS) == 1) // timer 0
            timer_handler(0);
        if (*(timer[1].base + TMIS) == 1) // timer 1
            timer_handler(1);
    }
    if (vicstatus & TIMER23_IRQ_VIC_BIT)
    {                                     // bit5=1:timer2,3, handle timer 2 and 3 one by one
        if (*(timer[2].base + TMIS) == 1) // timer 2
            timer_handler(2);
        if (*(timer[3].base + TMIS) == 1) // timer 3
            timer_handler(3);
    }

    //KBD
    if (vicstatus & (1 << 31))
    { // PIC.bit31= SIC interrupts
        if (sicstatus & (1 << 3))
        { // SIC.bit3 = KBD interrupt
            kbd_handler();
        }
    }
}