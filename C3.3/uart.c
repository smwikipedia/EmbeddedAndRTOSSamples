#include "uart.h"
#include "types.h"

UART uart[4]; // 4 UART structures

int uart_init()
{
    u32 i;
    UART *up;
    for(i=0; i<4; i++){
        up = &uart[i];
        if(i != 3)
        {// uart 0 ~ 2 are adjacent
            up->base = (char *)(ARM_VERSATILE_PL011_UART0 + i * 0x1000); 
        }
        else
        {// uart 3 is different
            up->base = (char *)(ARM_VERSATILE_PL011_UART3);                   
        }        
        *(up->base + CNTL) &= ~0x10; // disable UART FIFO
        *(up->base + IMSC) |= 0x30;  // enable TX and RX interrupts for UART
        up->n = i; //UART ID
        up->indata = up->inhead = up->intail = 0;
        up->inroom = SBUFSIZE;
        up->outdata = up->outhead = up->outtail = 0;
        up->outroom = SBUFSIZE;
        up->txon = 0;
    }
}

void do_rx(UART *up)
{
    char c;
    c = *(up->base + UDR);
    up->inbuf[up->inhead++] = c;
    up->inhead %= SBUFSIZE; //circular buffer
    up->indata++;
    up->inroom--;
    //how to echo back?    
}


void do_tx(UART *up)
{
    u8 c;
    if(up->outdata <= 0)
    {
        *(up->base + IMSC) = 0x10; // disable TX interrupt
        up->txon = 0; // turn off txon flag
        return;
    }
    c = up->outbuf[up->outtail++];
    up->outtail %= SBUFSIZE;
    *(up->base + UDR) = (u32)c; // write c to output data register
    up->outdata--;
    up->outroom++;
}

void uart_handler(UART *up)
{
    u8 mis = *(up->base + MIS); //read MIS register
    if(mis & RX_BIT)
    {
        do_rx(up);
    }
    else if(mis & TX_BIT)
    {
        do_tx(up);
    }
    else
    {
        while(1); // dead loop, something unexpected happened.
    }
}

u8 ugetc(UART *up)
{
    u8 c;
    while(up->indata <=0);
    c = up->inbuf[up->intail++];
    up->intail %= SBUFSIZE;
    // lock();
    up->indata--;
    up->inroom++;
    // unlock();
    return c;
}

void uputc(UART *up, u8 c)
{
    if(up->txon)
    {
        up->outbuf[up->outhead++] = c;
        up->outhead %= SBUFSIZE;
        //lock();
        up->outdata++;
        up->outroom--;
        //unlock();
        return;
    }
    u32 i = *(up->base + UFR); // why do this?
    while(*(up->base + UFR)& TXFF);
    *(up->base + UDR) = (u32)c;
    *(up->base + IMSC) |= 0x30;
    up->txon = 1;
}

void ugets(UART *up, char *s)
{
    while((*s = ugetc(up))!= '\r')
    {
        uputc(up, *s++);
    }
    *s = 0;
}

void uprints(UART *up, u8 *s)
{
    while(*s)
    {
        uputc(up, *s++);
    }
}

