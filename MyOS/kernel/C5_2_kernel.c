#include "types.h"
#include "versatilepb.h"
#include "pl190.h"
#include "pl011.h"
#include "pl110.h"
#include "sp804.h"
#include "pl050.h"
#include "display.h"

extern void tswitch();

#define NPROC 5
#define SSIZE 1024
typedef struct proc
{
    struct proc *next;
    u32 *ksp;
    u32 pid;
    u32 kstack[SSIZE];
} PROC;

PROC proc[NPROC], *running;
u32 procsize = sizeof(PROC);

u32 body()
{
    u8 c;
    kprintf("proc %d resume to body()\n", running->pid);
    while (1)
    {
        kprintf("proc %d in body() input a u8 [s] : ", running->pid);
        c = kgetc();
        kprintf("%c\n", c);
        tswitch(); // in C5.2_reset.S
    }
}

u32 kernel_init()
{
    u32 i, j;
    PROC *p;
    kprintf("kernel_init()\n");

    /*
    In K.C. Wang's book, below loop variable i starts with 0.
    But it will not work. Because proc[0]'s stack has been used as the SVC stack.
    Though the "BL main" in the C5_2_reset.S won't affect the stack,
    the stack will be modified when kernel_init() is invoked by the main().
    For example, the local variable i, j in kernel_init() are both on this stack.
    So we cannot blindly modify the proc[0] stack with [SSIZE-j] because that will corrupt the local variables.
    In my experiment, I see j returns to 0 because of "p->kstack[SSIZE - j] = 0x0;".
    So the for loop of j will never exit.

    My solution for now is to abandon the proc[0].
    The cirular task list will only cover proc[1~4], while proc[0].next also points to proc[1].
    After proc[0] calls the tswitch(), the task switch will loop with proc[1~4].
    */

    for (i = 1; i < NPROC; i++)
    {
        p = &proc[i];
        p->pid = i;
        //p->status = READY;
        for (j = 0; j < 15; j++)           // initialize proc.kstack and saved ksp
            p->kstack[SSIZE - j] = 0x0;    // all saved regs = 0
        p->kstack[SSIZE - 1] = (u32)body;  // resume point = body
        p->ksp = &(p->kstack[SSIZE - 14]); // saved ksp, a full-down stack
        p->next = p + 1;                   // point to next PROC
    }

    p = &proc[0];
    p->pid = 0;
    p->next = &proc[1];

    proc[NPROC - 1].next = &proc[1]; // circular PROC list, skip proc[0] because I cannot prepare its stack for resuming.
    running = &proc[0];
}

u32 scheduler()
{
    kprintf("proc %d in scheduler\n", running->pid);
    running = running->next;
    kprintf("next running = %d\n", running->pid);
}

u32 main()
{
    u8 c;
    //fbuf_init(); // initialize LCD driver
    //kbd_init();  // initialize KBD driver

    board_init();    

    kprintf("Welcome to WANIX in Arm\n");
    kernel_init();
    //while (1) // this loop exists in KC Wang's book. but it is unnecessary based on my experiment.
    //{
    kprintf("P0 running input a key : ");
    c = kgetc();
    kprintf("%c\n", c);
    tswitch();
    //}
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