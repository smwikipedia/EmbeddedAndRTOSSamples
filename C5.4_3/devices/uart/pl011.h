#ifndef PL011_UART_H
#define PL011_UART_H
#include "types.h"
#include "tf-m/uart_pl011_drv.h"


/* 
UART base addresses
The purpose of below indirect macros is that, when porting to other boards,
we may only need change these base addresses.
*/
// #define PL011_UART0_BASE VERSATILEPB_PL011_UART0
// #define PL011_UART1_BASE VERSATILEPB_PL011_UART1
// #define PL011_UART2_BASE VERSATILEPB_PL011_UART2
// #define PL011_UART3_BASE VERSATILEPB_PL011_UART3



//UART register offsets, check the PL110 spec
#define UDR 0x00 // data 
#define UDS 0x04 // receive status or error clear
#define UFR 0x18 // flag, it reflects the working status of a UART instance
#define CNTL 0x2C // line control
#define IMSC 0x38 // interrupt mask set/clear, this enable(set)/disable(clear) a certain interrupt
#define MIS 0x40 // masked interrupt status, this reflect the status of a certain interrupt

//UART flag register (offset 0x18) values
#define TXFE 0x80 // fifo is disabled, bit 7 set when tx holding register is empty
#define TXFF 0x20 // fifo is disabled, bit 5 set when tx holding register is full
#define RXFE 0x10 // fifo is disabled, bit 4 set when rx holding register is empty
#define RXFF 0x40 // fifo is disabled, bit 6 set when rx holding register is full
#define BUSY 0x08 // bit 3 set when UART busy transmitting data

//RX TX bits in IMSC and MIS registers
#define RX_BIT (1<<4) // RX interrupt mask bit in IMSC register, also RX interrupt status bit in MIS register.
#define TX_BIT (1<<5) // TX interrupt mask bit in IMSC register, also TX interrupt status bit in MIS register.

//string buffer size
#define SBUFSIZE 50

/* Default UART baud rate */
#define DEFAULT_UART_BAUDRATE 115200

/*
outdata: how many chars in outbuf to be transmitted
outroom: how much room remaining in outbuf
outhead: the latest char buffered
outtail: the next char to be transmitted

logically, outhead is ahead of outtail

**************************************************  outbuf[]
              ^                     ^
              |                     |
(start)     outtail                outhead     (end)


**************************************************  inbuf[]
              ^                     ^
              |                     |
(start)     intail                inhead       (end)

*/
typedef volatile struct uart {
    u8 * base;    // base address of UART
    u32 n;          // uart id 0~3
    u8 inbuf[SBUFSIZE];
    u32 indata, inroom, inhead, intail;
    u8 outbuf[SBUFSIZE];
    u32 outdata, outroom, outhead, outtail;
    boolean wrap;
    volatile u32 txon;  // 1 = TX interrput is on, the UART is in transmitting state
    struct uart_pl011_dev_t *p_pl011_dev;
} UART;

//void uart_init();
void uart_init_single(UART *up, u32 uart_base);
void uart_init_single_tf_m(UART *up, u32 uart_base);
void uart_handler(UART *up);
void uprints(UART *up, u8 *s);
void ugets(UART *up, char *s);
void uprintf(UART *up, u8* fmt, ...);

#endif