/*
This version of driver follows the sleep/wakeup paradigm introduced in $5.6 Process Synchronization.
*/

#include "types.h"
#include "display.h"
#include "string.h"
#include "pl050.h"
//#include "pl011.h"
#include "versatilepb.h"
#include "debug.h"

// defined in ts.S
// synchoronize methods for the upper half of the interrupt-driven driver.
extern void lock();
extern void unlock();

extern void ksleep(u32 event);
extern void kwakeup(u32 event);

//volatile KBD kbd; // KBD data structure
//extern UART *up;

//below scan codes are for PC-AT set 2. Ref: https://en.wikipedia.org/wiki/Scancode
#define MAX_SCANCODE_NUM 128
#define LSHIFT 18
#define RSHIT 89
#define ENTER 90
#define KEY_RELEASE_PREFIX 240

boolean shift_on = FALSE;
boolean key_release = FALSE;

u8 PC_AT_VISABLE_CHARMAP_UNSHIFT[MAX_SCANCODE_NUM] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '`', 0,
    0, 0, 0, 0, 0, 'q', '1', 0, 0, 0, 'z', 's', 'a', 'w', '2', 0,
    0, 'c', 'x', 'd', 'e', '4', '3', 0, 0, ' ', 'v', 'f', 't', 'r', '5', 0,
    0, 'n', 'b', 'h', 'g', 'y', '6', 0, 0, 0, 'm', 'j', 'u', '7', '8', 0,
    0, ',', 'k', 'i', 'o', '0', '9', 0, 0, '.', '/', 'l', ';', 'p', '-', 0,
    0, 0, '\'', 0, '[', '=', 0, 0, 0, 0, 0, ']', 0, '\\', 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

u8 PC_AT_VISABLE_CHARMAP_SHIFT[MAX_SCANCODE_NUM] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '`', 0,
    0, 0, 0, 0, 0, 'Q', '!', 0, 0, 0, 'Z', 'S', 'A', 'W', '@', 0,
    0, 'C', 'X', 'D', 'E', '$', '#', 0, 0, ' ', 'V', 'F', 'T', 'R', '%', 0,
    0, 'N', 'B', 'H', 'G', 'Y', '^', 0, 0, 0, 'M', 'J', 'U', '&', '*', 0,
    0, '<', 'K', 'I', 'O', ')', '(', 0, 0, '>', '?', 'L', ':', 'P', '_', 0,
    0, 0, '"', 0, '{', '+', 0, 0, 0, 0, 0, '}', 0, '|', 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};


void kbd_init(KBD *kp, u32 PL050_KBD_BASE)
{
    kp->base = (u8 *)PL050_KBD_BASE;
    *(kp->base + KCNTL) = 0x14; // 00010100=INTenable, Enable
    *(kp->base + KCLK) = 8;     // PL051 manual says a value 0 to 15
    kp->data = 0;
    kp->room = MAX_KBD_CHAR_BUFFER_SIZE; // counters
    kp->head = kp->tail = 0;             // index to buffer
}

// void kbd_init()
// {
//     KBD *kp = &kbd;
//     kp->base = (u8 *)PL050_KBD_BASE;
//     *(kp->base + KCNTL) = 0x14; // 00010100=INTenable, Enable
//     *(kp->base + KCLK) = 8;     // PL051 manual says a value 0 to 15
//     kp->data = 0;
//     kp->room = MAX_KBD_CHAR_BUFFER_SIZE; // counters
//     kp->head = kp->tail = 0;             // index to buffer
// }

void kbd_handler() // KBD interrupt handler in C
{
    u8 scode, c;
    u32 i;
    KBD *kp = &kbd;
    //color = RED;                 // int color in vid.c file
    scode = *(kp->base + KDATA); // read scan code in data register

    // below 2 lines are used to get the scancode of the keyboard.
    //kprintf("kbd interrupt: scode = %d\n", scode);
    //return;

    if (key_release)
    {
        if (scode == LSHIFT || scode == RSHIT)
        {
            shift_on = FALSE;
        }
        key_release = FALSE;
        return;
    }

    if (scode == KEY_RELEASE_PREFIX)
    {
        key_release = TRUE;
        return;
    }

    if (scode == LSHIFT || scode == RSHIT)
    {
        shift_on = key_release ? FALSE : TRUE;
        return;
    }

    if (scode == ENTER)
    {
        c = '\n';
    }
    else
    {
        u8 *charmap = shift_on ? PC_AT_VISABLE_CHARMAP_SHIFT : PC_AT_VISABLE_CHARMAP_UNSHIFT;
        c = charmap[scode];
    }

    /* 
    head: the next place in buffer to place the input char
    tail: the next buffered char to retrieve
    */
    if(kp->head == kp->tail)
    {
        if(kp->data == MAX_KBD_CHAR_BUFFER_SIZE)
        {
            ASSERT(kp->room == 0);
            /*
            head catches up with tail from behind
            which means the buffer is full,
            the newly incoming c will overwrite the old tail
            push both head and tail forward
            data and room remain unchanged
            */            
            kp->buf[kp->head++] = c;
            kp->head %= MAX_KBD_CHAR_BUFFER_SIZE;  
            kp->tail++;            
            kp->tail %= MAX_KBD_CHAR_BUFFER_SIZE;
            // kp->data = MAX_KBD_CHAR_BUFFER_SIZE;
            // kp->room = 0;
        }
        else if (kp->data == 0)
        {
            /*
            tail catches up with head from behind
            which means the buffer is empty,
            just push the head forward, the tail remains still.
            */             
            kp->buf[kp->head++] = c;
            kp->head %= MAX_KBD_CHAR_BUFFER_SIZE; 
            kp->data++;
            kp->room--;
        }
        else
        {
            ASSERT(FALSE);
        }
    }
    else
    {
        kp->buf[kp->head++] = c;
        kp->head %= MAX_KBD_CHAR_BUFFER_SIZE;
        kp->data++;
        kp->room--;
    }

    //below echo is just for better illustration, in real life, I don't think a driver should do this.  
    kprintf("%c", c);

    //kprintf("%c[head:%d, tail:%d, data:%d,  room:%d]\n", c, kp->head, kp->tail, kp->data, kp->room); // echo to LCD
    //uprintf(up, "%c[head:%d, tail:%d, data:%d,  room:%d]\n", c, kp->head, kp->tail, kp->data, kp->room); // echo to UART
    //kprintf("%c", c); // echo to LCD

    /*
    Rasie event for line completion
    Actually, I was thinking maybe it is inappropriate for kbd_handler() to wake up a task.
    But in this experiment, we have no other things can do this but the kbd_handler.
    Maybe in future examples, we will have more interesting/realistic options.    
    */
    if(c == '\n')
    {
        kwakeup((u32)&kbd);
    }
}

/*
This is the upper-half of the interrupt-driven driver.
It needs the lock/unlock calls because it must be ensure the interrupt won't happen during is execution,
which may corrupt the control variables.
But the kbd_handler() doesn't need to call the lock/unlock methods because it can rest for sure that upper-half won't step in,
which means it is the only one who manipulates the control variables.

In short, the contention for control variables only exists for the upper-half, not the lower-half.


The kgets() and kgetc() just keep *chasing* the input buffer from current tail position.
The 

*/
u8 kgetc() // return a char from keyboard in sleep/wakeup paradigm
{
    u8 c;
    KBD *kp = &kbd;

    while(1)
    {
        lock();
        if(kp->data == 0)
        {
            unlock();
            ksleep((u32)&kbd);
        }
        else
        {
            c = kp->buf[kp->tail++];
            kp->tail %= MAX_KBD_CHAR_BUFFER_SIZE;
            kp->data--;
            kp->room = MAX_KBD_CHAR_BUFFER_SIZE - kp->data;
            unlock();
            return c;
        }
    }
}

u32 kgets(u8 s[]) // get a string from KBD
{
    u8 c;
    while ((c = kgetc()) != '\n')
    {
        *s++ = c;
    }
    *s = '\n';
    s++;
    *s = 0;
    return strlen(s);
}