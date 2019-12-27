#ifndef VERSATILEPB_PL190_VIC_H
#define VERSATILEPB_PL190_VIC_H


#include "types.h"
//check the versatilepb and pl190 specs
#define PICIRQSTATUS_ADDR 0x10140000
#define PICINTENABLE_ADDR 0x10140010

#define VIC_STATUS (*(u32*)PICIRQSTATUS_ADDR)
#define VIC_INTENABLE (*(u32*)PICINTENABLE_ADDR)

#define UART0_IRQ_VIC_BIT (1<<12)
#define UART1_IRQ_VIC_BIT (1<<13)


#endif


