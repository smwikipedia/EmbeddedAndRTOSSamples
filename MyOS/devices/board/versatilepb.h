#ifndef VERSATILEPB_H
#define VERSATILEPB_H

#include "pl190.h"
#include "pl011.h"
#include "pl110.h"
#include "sp804.h"
#include "display.h"



/*
offsets of 4 UARTs on versatilepb board.
*/
#define ARM_VERSATILE_PL011_UART0 0x101F1000 // check the memory map in versatilepb spec
#define ARM_VERSATILE_PL011_UART1 0x101F2000
#define ARM_VERSATILE_PL011_UART2 0x101F3000
#define ARM_VERSATILE_PL011_UART3 0x10009000 // slightly different from UART0/1/2
extern UART uart[4];

/*
VIC for versatilepb 
*/
//check the versatilepb and pl190 specs
#define PICIRQSTATUS_ADDR 0x10140000 // IRQ status register
#define PICINTENABLE_ADDR 0x10140010 // Interrupt enable register

#define VIC_STATUS (*(u32*)PICIRQSTATUS_ADDR)
#define VIC_INTENABLE (*(u32*)PICINTENABLE_ADDR)

//check the versatilepb spec for interrupt source bits arrangement
#define UART0_IRQ_VIC_BIT (1<<12)
#define UART1_IRQ_VIC_BIT (1<<13)

//timer
#define TIMER01_IRQ_VIC_BIT (1<<4)
#define TIMER23_IRQ_VIC_BIT (1<<5)
extern volatile TIMER timer[4]; //4 timers; 2 per unit; at 0x00 and 0x20

#endif
