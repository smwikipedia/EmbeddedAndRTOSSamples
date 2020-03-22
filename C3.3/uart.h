#ifndef UART_H
#define UART_H

#include "types.h"

//offsets of 4 UARTs on versatilepb board.
#define ARM_VERSATILE_PL011_UART0 0x101F1000 // check the memory map in versatilepb spec
#define ARM_VERSATILE_PL011_UART1 0x101F2000
#define ARM_VERSATILE_PL011_UART2 0x101F3000
#define ARM_VERSATILE_PL011_UART3 0x10009000 // slightly different from UART0/1/2

//UART register offsets, check the PL110 spec
#define UDR 0x00 // data 
#define UDS 0x04 // receive status or error clear
#define UFR 0x18 // flag, it reflects the working status of a UART instance
#define CNTL 0x2C // line control
#define IMSC 0x38 // interrupt mask set or clear, this disable/enable a certain interrupt
#define MIS 0x40 // masked interrupt status, this reflect the status of a certain interrupt

//UART flag register (offset 0x18) values
#define TXFE 0x80 // fifo is disabled, bit 7 set when tx holding register is empty
#define TXFF 0x20 // fifo is disabled, bit 6 set when tx holding register is full
#define RXFE 0x10 // fifo is disabled, bit 6 set when rx holding register is empty
#define RXFF 0x40 // fifo is disabled, bit 6 set when rx holding register is full
#define BUSY 0x08 // bit 3 set when UART busy transmitting data

//RX TX bits in IMSC and MIS registers
#define RX_BIT (1<<4) // RX interrupt mask bit in IMSC register, also RX interrupt status bit in MIS register.
#define TX_BIT (1<<5) // TX interrupt mask bit in IMSC register, also TX interrupt status bit in MIS register.

//string buffer size
#define SBUFSIZE 128

typedef volatile struct uart {
    u8 * base;    // base address of UART
    u32 n;          // uart id 0~3
    u8 inbuf[SBUFSIZE];
    u32 indata, inroom, inhead, intail;
    u8 outbuf[SBUFSIZE];
    u32 outdata, outroom, outhead, outtail;
    volatile u32 txon;  // 1 = TX interrput is on, the UART is in transmitting state
} UART;

void uart_init();


#endif