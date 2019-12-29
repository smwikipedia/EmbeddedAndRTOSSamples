#ifndef UART_H
#define UART_H

#include "types.h"

//UART register offsets
#define UDR 0x00
#define UDS 0x04
#define UFR 0x18
#define CNTL 0x2C
#define IMSC 0x38
#define MIS 0x40

//UART status flags
#define TXFE 0x80
#define TXFF 0x20
#define RXFE 0x10
#define RXFF 0x40
#define BUSY 0x08

#define ARM_VERSATILE_PL011_UART0 0x101F1000
#define ARM_VERSATILE_PL011_UART3 0x10009000

#define RX_BIT (1<<4)
#define TX_BIT (1<<5)

//string buffer size
#define SBUFSIZE 128

typedef volatile struct uart {
    u8 * base;    // base address of UART
    u32 n;          // uart id 0~3
    u8 inbuf[SBUFSIZE];
    u32 indata, inroom, inhead, intail;
    u8 outbuf[SBUFSIZE];
    u32 outdata, outroom, outhead, outtail;
    volatile u32 txon;  // 1 = TX interrput is on
} UART;

void uart_init();


#endif