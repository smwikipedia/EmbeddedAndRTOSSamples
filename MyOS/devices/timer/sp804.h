#ifndef UART_H
#define UART_H

#include "types.h"

#define TLOAD 0x0
#define TVALUE 0x1
#define TCNTL 0x2
#define TINTCLR 0x3
#define TRIS 0x4
#define TMIS 0x5
#define TBGLOAD 0x6

typedef volatile struct timer
{
    u32 *base;            // timer's base address; as u32 pointer
    u32 tick, hh, mm, ss; // per timer data area
    u8 clock[16];
} TIMER;

void timer_start(u32 n);
void timer_handler(u32 n);
void timer_init();


#endif