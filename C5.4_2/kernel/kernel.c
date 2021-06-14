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

// in KC Wang's book, below 2 functions are named int_off, int_on
// but it has nothing to do with interrupt
// so I changed the names and implementation
extern u32 get_cpsr(void);
extern void set_cpsr(u32 cpsr);

void ksleep(u32 event);
void kwakeup(u32 event);

PROC proc[NPROC];
PROC *running, *freeList, *readyQueue, *sleepQueue;
u32 procsize = sizeof(PROC);

void printAll()
{
    printList("freeList", freeList);
    printList("readyQueue", readyQueue);
    printList("sleepQueue", sleepQueue);
}

// void body()
// {
//     u8 c;
//     //color = running->pid;
//     kprintf("proc %d resume to body()\n", running->pid);
//     while (1)
//     {
//         kprintf("proc %d in body() input a char [s|f|x] : ", running->pid);
//         c = kgetc();
//         kprintf("%c\n", c);
//         switch (c)
//         {
//         case 's':
//             tswitch();
//             break;
//         case 'f':
//             kfork((u32)body, PRIORITY_1);
//             break;
//         case 'x':
//             kexit();
//             break;
//         }
//     }
// }

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
    proc[NPROC-1].next = NULL; // fix the last one
    
    freeList = &proc[0]; // all PROCs in freeList initially
    readyQueue = NULL; // readyQueue empty
    sleepQueue = NULL; // sleepQueue empty

    //kprintf("before start proc[0]\n");
    //printAll();
    
    // create P0 as initial running process
    // when kernel init, get_proc() will always return proc[0].
    // But we should use get_proc() instead of &proc[0] because we need to maintain the freeList.
    p = get_proc(&freeList);
    ASSERT(p == &proc[0]);
    p->priority = PRIORITY_0; // P0 has the lowest priority 0
    p->status = READY; // proc[0] is always READY, it can always run when there's no other tasks to run
    running = p; // now the "running" is consistent with what we have done in "reset_handler"

    //kprintf("after start proc[0]\n");
    //kprintf("running = %d\n", running->pid);
    //printAll();
}

void scheduler()
{
    kprintf("proc %d in scheduler\n", running->pid);
    switch (running->status)
    {
        case READY: // if the running is still ready, enqueue it for next slice.
            // the proc[0] task is special that it is always READY. it is the very first task of the system.
            // it can always run when there's no other tasks to run.
            enqueue(&readyQueue, running);
            break;
        case SLEEP: // I don't know how K.C. Wang's book implement it. I guess there should be a sleepQueue.
            enqueue(&sleepQueue, running);
            break;
        default:
            ASSERT(FALSE);
            break; // won't reach here.
    }
        
    running = dequeue(&readyQueue); // there should be other tasks to run, otherwise, the proc[0] is always in the readyQueue to run.
    kprintf("next running = %d\n", running->pid);
    printAll();   
}

void timer_task()
{

}

void uart_task()
{

}

void kbd_task()
{
    u8 line[MAX_KBD_CHAR_BUFFER_SIZE];
    while(1)
    {
        kprintf("KBD task %d running\n", running->pid);        
        kprintf("KBD task %d sleep for a line from KBD...\n", running->pid);
        /*
        KC Wang's book has below line.
        But I think a task shouldn't go to sleep so explicitly.
        kgets() calls kgetc(), which will call ksleep() if no char in buffer.
        So I commented out below line.
        It still works fine.
        */
        //ksleep((u32)&kbd);

        kgets(line);
        kprintf("PROC %d get this: %s", running->pid, line);
    }
}


void ksleep(u32 event)
{
    // in KC Wang's book, below line is int_off()
    // but the sematic has nothing to do with interrupt, so I change the name
    u32 old_cpsr = get_cpsr();
    running->event = event;
    running->status = SLEEP;
    // One more task fall asleep, it's reasonable to do a task switch.
    tswitch(); // This is one of the time point to call tswitch(). Carefully chose the point.
    
    // in KC Wang's book, below line is int_on()
    // but the sematic has nothing to do with interrupt, so I change the name
    set_cpsr(old_cpsr);
}

/*
Wake up ALL PROCs in the sleep queue that are sleeping on the event.
Move ALL those PROCs from sleep queue to ready queue.
And maintain the sleep queue by priority.
*/
void kwakeup(u32 event)
{
    u32 old_cpsr = get_cpsr();
    
    PROC *current = NULL;
    PROC *previous = NULL;
    PROC *to_wakeup = NULL;
    current = sleepQueue;
    while(current != NULL)
    {
        if(current->status == SLEEP && current->event == event)
        {
            if(previous == NULL)
            {
                sleepQueue = current->next;
            }
            else
            {
                previous->next = current->next;
            }
            
            to_wakeup = current;
            to_wakeup->status = READY;
            current = current->next;
            enqueue(&readyQueue, to_wakeup);            
        }
        else
        {
            previous = current;
            current = current->next;
        }
    }
    
    set_cpsr(old_cpsr);
}

u32 main()
{
    //Since we have set SVC mode stack to proc[0]'s kstack,
    //main() will runs on that stack.
    //main() may be seen as the very first task of the system whose stack is set up by brutal force.

    u8 c;
    //fbuf_init(); // initialize LCD driver
    //kbd_init();  // initialize KBD driver

    board_init();    

    kprintf("Thank you! Professor K.C. Wang.\n");
    kernel_init();
    
    // proc[0] create proc[1] into readyQueue, with priority 1
    // proc[1] will never get char in this experiment
    kfork((u32)kbd_task, PRIORITY_1);
    // proc[0] create proc[2] into readyQueue, with priority 1
    // proc[2] will never get char in this experiment
    kfork((u32)kbd_task, PRIORITY_1);
    // proc[0] create proc[3] with a higher priority
    // proc[3] will ALWAYS get char in this experiment because of its high priority.
    kfork((u32)kbd_task, PRIORITY_2);

    while (1)
    {
        //we are on proc[0]'s stack
        if (readyQueue != NULL)
        {
            /*
            proc[0] will yield its execution.
            The stack will change to the new task's. Though it may still be the switch-out one.
            If there's no tasks to run, proc[0] is always READY to run. In that case, the stack will still be proc[0]'s.
            And in that case, the while loop will continue to run.
            If there's some other task to run, the while loop will cease to run.
            */
            tswitch(); // This is one of the tswitch() point, carefully choose the point.

            kprintf("\n.....");
        }
        // Still busy loop here...       
    }
}

/*
Copy interrupt/exception vectors to address 0.
This is a requirment of the ARM arch.
*/
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