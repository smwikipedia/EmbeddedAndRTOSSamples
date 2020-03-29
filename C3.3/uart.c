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
    up->indata++; // a newly received char is buffered
    up->inroom--; // a newly received char is buffered

    /*
    This function doesn't do the echo back.
    It just faithfully collect incoming chars and put them into the up->inbuf[].
    */
}

/*

*/
void do_tx(UART *up)
{
    u8 c;
    /*
    The up->txon will only return to 0 when the up->outbuf is empty.
    */
    if(up->outdata <= 0)
    {
        /*
        Clear the MIS[TX] by disable IMSC[TX] interrupt.
        Otherwise, the MIS[TX] will never disappear and the execution will dead loop in the IRQ handling.
        It can also be viewed as some kind of acknoledgement of the single-char trasmission completion.        
        */
        *(up->base + IMSC) = RX_BIT;
        up->txon = 0; // turn off txon flag
        return;
    }

    /*
    If the UART experienced some delay, and the chars keep coming in for UART to transmit,
    the outbuf will hold the yet-to-transmit chars, and the up->txon will remain 1.
    And we just keep getting data from the up->outbuf[] and write the char to UDR.
    
    Is it possible that a char in UART hasn't got chance to be fully transmitted but overwritten 
    by a new char from the up-outbuf[]?
    No, we don't need to worry about that.
    Because according to the PL011 spec, the TX interrupt will be raised *after* the single-char is transmitted.
    So once we are here in do_tx(), we are safe to write another char into UDR.

    So the lesson is, we just need to be careful about the timing!

    */
    c = up->outbuf[up->outtail++];
    up->outtail %= SBUFSIZE;

    /*
    Write c to output data register, this will also clear the TX IRQ signal for this time according to the UART PL011 spec.
    After the new c get transmitted, a new TX IRQ will be rasised.
    So below code is definitely necessary for robustness.
    */
    *(up->base + UDR) = (u32)c;
    up->outdata--; // a buffered char is transmitted
    up->outroom++; // a buffered char is transmitted
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

/*
do_rx() is responsible to collect incoming chars into up->inbuf[].
This function just consume chars fro mthe up->inbuf[].
*/
u8 ugetc(UART *up)
{
    u8 c;
    while(up->indata <=0); // no data in buffer, just block!
    c = up->inbuf[up->intail++];
    up->intail %= SBUFSIZE;
    // lock();
    up->indata--; // a buffered char is handled
    up->inroom++; // a buffered char is handled
    // unlock();
    return c;
}

void uputc(UART *up, u8 c)
{
    /*
    For the 1st char to ouput, the txon is 0.
    During the UART transmission, the txon will be 1.
    If for some reason, the UART hardware encounters some delay, even for transmitting a single char,
    the cpu will still be running the uputs() -> uputc(), so the chars will just keep coming.
    then newly incoming chars will be stored into up->outbuf[], and the up->txon will never be set to 0 in the do_tx().
    So in this case, below code wil be executed.
    It's just a software buffer to tolerate some potential hardware delays.
    The buffer is the lubricant between software and hardware.
    */
    if(up->txon)
    {
        up->outbuf[up->outhead++] = c;
        up->outhead %= SBUFSIZE;
        //lock();
        up->outdata++; // a new char is buffered
        up->outroom--; // a new char is buffered
        //unlock();
        return;
    }
    //u32 i = *(up->base + UFR); // why do this?
    while(*(up->base + UFR) & TXFF);// if the tx holding register is full, busy wait
    
    /*
    The action sequence is:
    Step 1. Update this program's knowledge about the UART state by setting the up->txon=1 to indicate that UART is busy trasmitting. 
    Step 2. Enable the TX RX interrupt by "setting" the IMSC[TX][RX] bits.
    Step 3. This is uputc(), so we write a char to the UDR.
    
    A TX interrupt will be raised after the single-char is transmitted Step 3.
    Then IRQ_handler() -> uart_handler() -> do_tx()

    Ref UART PL011 spec：
    If the FIFOs are disabled (have a depth of one location) and there is no data present in the transmitters single location,
    the transmit interrupt is asserted HIGH. It is cleared by performing a single write to the transmit FIFO, or by clearing the
    interrupt

    */
    up->txon = 1;// this line should precede the next line to ensure up-txon correctly reflect the status of TX interrupt.
    
    *(up->base + IMSC) |= (RX_BIT | TX_BIT);

    /*
    Write the char data into the data register.
    The write operation initiates the transmission according to the UART PL011 spec.
    During the debug with qemu, I never see the UDR holding the data c.
    I guess because in the non-FIFO mode, the data is immediately transmitted
    and UDR returns to 0 immediately, or never changes.
    And I did see the client for this UART port on the other side showing the transmitted data immediately.
    After the UART hardware transmitted this single char, the MIS[TX] bit will be signaled with value 1!
    We just rely on the hardware...
    */
    *(up->base + UDR) = (u32)c;

}

void ugets(UART *up, char *s)
{
    while((*s = ugetc(up))!= '\r')
    {
        uputc(up, *s); // echo back as user is typing so user can see what he has just input.
        s++;
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

