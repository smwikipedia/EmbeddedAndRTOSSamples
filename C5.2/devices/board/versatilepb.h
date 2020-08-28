#ifndef VERSATILEPB_H
#define VERSATILEPB_H


/*
Base addresses of 4 UARTs on versatilepb board.
*/
#define VERSATILEPB_PL011_UART0 0x101F1000 // check the memory map in versatilepb spec
#define VERSATILEPB_PL011_UART1 0x101F2000
#define VERSATILEPB_PL011_UART2 0x101F3000
#define VERSATILEPB_PL011_UART3 0x10009000 // slightly different from UART0/1/2
#define MAX_UART_NUMBER 4
#define MAX_TIMER_NUMBER 4


/*
VIC for versatilepb 
*/
//check the versatilepb and pl190 specs
#define PICIRQSTATUS_ADDR 0x10140000 // Primary Interrupt Controller (PIC) status regiseter
#define PICINTENABLE_ADDR 0x10140010 // PIC interrupt enable register

#define SICIRQSTATUS_ADDR 0x10003000 // Secondary Interrupt Controller (SIC) status regiseter
#define SICINTENABLE_ADDR 0x10003008 // SIC interrupt enable register

#define VIC_STATUS (*(u32*)PICIRQSTATUS_ADDR)
#define VIC_INTENABLE (*(u32*)PICINTENABLE_ADDR)

#define SIC_STATUS (*(u32*)SICIRQSTATUS_ADDR)
#define SIC_INTENABLE (*(u32*)SICINTENABLE_ADDR)

//check the versatilepb spec for interrupt source bits arrangement
#define UART0_IRQ_VIC_BIT (1<<12)
#define UART1_IRQ_VIC_BIT (1<<13)

//timer
#define VERSATILEPB_SP804_TIMER0 0x101E2000
#define VERSATILEPB_SP804_TIMER1 0x101E2020
#define VERSATILEPB_SP804_TIMER2 0x101E3000
#define VERSATILEPB_SP804_TIMER3 0x101E3020

#define TIMER01_IRQ_VIC_BIT (1<<4)
#define TIMER23_IRQ_VIC_BIT (1<<5)


//KBD
#define VERSATILEPB_PL050_KBD 0x10006000
#define SIC_IRQ_VIC_BIT (1<<31)
#define KMI0_IRQ_SIC_BIT (1<<3)
#define KMI1_IRQ_SIC_BIT (1<<4)

//LCD
#define VERSATILEPB_OSC1 0x1000001c
#define VERSATILEPB_PL110_LCD_BASE 0x10120000
#define VERSATILEPB_PL110_LCD_TIME_REG0 0x10120000
#define VERSATILEPB_PL110_LCD_TIME_REG1 0x10120004
#define VERSATILEPB_PL110_LCD_TIME_REG2 0x10120008

void board_init();
#endif
