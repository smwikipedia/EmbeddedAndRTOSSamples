#include "uart.h"
#include "types.h"

UART uart[4]; // 4 UART structures

void uart_init()
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
        *(up->base + IMSC) |= (RX_BIT | TX_BIT);  // enable TX and RX interrupts for UART
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
        /*
        Clear the MIS[TX] by disable IMSC[TX] interrupt.
        Otherwise, the MIS[TX] will never disappear and the execution will dead loop in the IRQ handing.
        */
        *(up->base + IMSC) = RX_BIT;
        up->txon = 0; // turn off txon flag
        return;
    }

    //below code never executes in the non-FIFO mode.
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
    // for the 1st char to ouput, the txon is 0.
    // in a non-FIFO mode, below condition block will never be entered.
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
    //u32 i = *(up->base + UFR); // why do this?
    while(*(up->base + UFR) & TXFF);// if the tx holding register is full, busy wait
    
    /*
    Write the char data into the data register.
    The write operation initiates the transmission.
    During the debug, I never see the UDR holding the data c.
    I guess because in the non-FIFO mode, the data is immediately transmitted
    and UDR returns to 0 immediately, or never changes.
    And I did see the client for this UART port on the other side showing the transmitted data immediately.
    But the MIS[TX] bit is not signaled yet!
    */
    *(up->base + UDR) = (u32)c;
    
    /*
    After the single char is transmitted as above,
    We enables the IMSC[TX] bit.
    At the same time, both the IMSC[TX] and MIS[TX] change to 1,
    which means a TX interrupt is raised *at this very moment*.
    Then IRQ_handler() -> uart_handler() -> do_tx()
    */
    up->txon = 1;// this line should precede the next line to ensure up-txon correctly reflect the status of TX interrupt.
    
    /*
    a TX interrupt is raised *at this very moment*! ->IRQ_handler() -> uart_handler() -> do_tx()
    So we can see, TX actually means the completion of a single-char transmission.
    */
    *(up->base + IMSC) |= (RX_BIT | TX_BIT);

}

void ugets(UART *up, char *s)
{
    while((*s = ugetc(up))!= '\r')
    {
        uputc(up, *s++); // echo back as user is typing so user can see what he has just input.
    }

    uputc(up, '\n'); //echo to a new line, otherwise, the line just echoed will be overwritten.
    uputc(up, '\r'); 

    *s++ = '\n'; // add line break to the newly collected line.
    *s++ = '\r';
    *s = 0;
}

void uprints(UART *up, u8 *s)
{
    while(*s)
    {
        uputc(up, *s++);
    }
}

