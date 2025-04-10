#include "pl011.h"
#include "types.h"
#include "display.h"
#include "versatilepb.h"

// defined in ts.S
extern void lock();
extern void unlock();

extern void ksleep(u32 event);
extern void kwakeup(u32 event);

void uputc(UART *up, u8 c);

/*
Initialize a single UART.
Note that below code only works on QEMU ARM versatilepb board.
It seems QEMU automatically uses default value for baud rate resgiers.
So we don't do it here.
This is not correct for UART in real hardware.
*/
void uart_init_single(UART *up, u32 uart_base)
{
    u32 i;
    up->base = (u8*)uart_base;
    *(up->base + CNTL) &= ~0x10; // disable UART FIFO
    *(up->base + IMSC) |= (RX_BIT | TX_BIT);  // enable TX and RX interrupts for UART
    up->indata = up->inhead = up->intail = 0;
    up->inroom = SBUFSIZE;
    up->outdata = up->outhead = up->outtail = 0;
    up->outroom = SBUFSIZE;
    up->wrap = FALSE;
    up->txon = 0;

    for (i=0; i<SBUFSIZE; i++)
    {
        up->inbuf[i] = 0;
        up->outbuf[i] = 0;
    }
}

void uart_init_single_tf_m(UART *up, u32 uart_base)
{
    u32 i;
    struct uart_pl011_dev_t *pl011_dev = up->p_pl011_dev;

    up->base = (u8*)uart_base;
    pl011_dev->cfg->base = uart_base;
    
    uart_pl011_init(pl011_dev, VERSATILEPB_PL011_CLOCK);
    uart_pl011_enable_intr(pl011_dev, (RX_BIT | TX_BIT));
    uart_pl011_enable(pl011_dev);
    // *(up->base + CNTL) &= ~0x10; // disable UART FIFO
    // *(up->base + IMSC) |= (RX_BIT | TX_BIT);  // enable TX and RX interrupts for UART


    up->n = i; //UART ID
    up->indata = up->inhead = up->intail = 0;
    up->inroom = SBUFSIZE;
    up->outdata = up->outhead = up->outtail = 0;
    up->outroom = SBUFSIZE;
    up->wrap = FALSE;
    up->txon = 0;

    for (i=0; i<SBUFSIZE; i++)
    {
        up->inbuf[i] = 0;
        up->outbuf[i] = 0;
    }
}


// void uart_init()
// {
//     u32 i;
//     UART *up;
//     for(i=0; i<MAX_UART_NUMBER; i++){
//         up = &uart[i];
//         if(i != 3)
//         {// uart 0 ~ 2 are adjacent
//             up->base = (char *)(PL011_UART0_BASE + i * 0x1000); 
//         }
//         else
//         {// uart 3 is different
//             up->base = (char *)(PL011_UART3_BASE);                   
//         }        
//         *(up->base + CNTL) &= ~0x10; // disable UART FIFO
//         *(up->base + IMSC) |= (RX_BIT | TX_BIT);  // enable TX and RX interrupts for UART
//         up->n = i; //UART ID
//         up->indata = up->inhead = up->intail = 0;
//         up->inroom = SBUFSIZE;
//         up->outdata = up->outhead = up->outtail = 0;
//         up->outroom = SBUFSIZE;
//         up->txon = 0;
//     }
// }


void do_rx(UART *up)
{
    char c;
    c = *(up->base + UDR);

    up->inbuf[up->inhead++] = c;
    up->inhead %= SBUFSIZE; //circular buffer

    if (up->wrap) {
        up->intail = up->inhead;
    }

    if (up->inhead == up->intail) {
        up->wrap = TRUE;
    }

    if(up->inroom > 0) {
        up->indata++;           // a newly received char is buffered
        up->inroom--;           // a newly received char is buffered
    }

    uputc(up, c); // echo back to telnet client
    kprintf("%c", c); // echo back to LCD


    /*
    Rasie event for line completion
    Actually, I was thinking maybe it is inappropriate for kbd_handler() to wake up a task.
    But in this experiment, we have no other parties can do this but the kbd_handler.
    Maybe in future examples, we will have more interesting/realistic options.
    */
    if(c == '\r') // '\r' is sent as a new line char
    {
        kwakeup((u32)up);
    }

}

/*
According to the PL011 spec:
If the FIFOs are disabled (have a depth of one location) and there is no data
present in the transmitters single location, the transmit interrupt is asserted HIGH.
It is cleared by performing a single write to the transmit FIFO, or by clearing the
interrupt.
*/
void do_tx(UART *up)
{
    u8 c;
    /*
    The up->txon will only return to 0 when the up->outbuf is empty.
    */
    if (up->outdata <= 0)
    {
        /*
        Clear the MIS[TX] by clearing IMSC[TX] bit
        Otherwise, the MIS[TX] will never disappear and the execution will dead loop in the IRQ handling.
        It can also be viewed as some kind of acknoledgement of the single-char trasmission completion.
        */
        *(up->base + IMSC) = *(up->base + IMSC) & (~TX_BIT);
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

    Actually, most output devices raise interrupts *after* output is done.
    So the lesson is, we just need to be careful about the timing of the interrupt!

    */
    c = up->outbuf[up->outtail++];
    up->outtail %= SBUFSIZE;

    /*
    Write c to output data register, it will also clear the TX IRQ signal for this time according to the UART PL011 spec.
    After the new c get transmitted, a new TX IRQ will be rasised.
    And the new TX IRQ will trigger this do_tx() function *again*.
    Each triggering of the do_tx() function will get a char from the up->outbuf[] until the up->outdata reaches 0.
    Kind of like self-relaying.
    I just give it a first move by uputc(), then the UART just keeps running by itself unitl no data to transmit.
    */
    up->outdata--; // a buffered char is transmitted
    up->outroom++; // a buffered char is transmitted

    /*
    This line should be a last action.
    Because the metadata change like above 2 lines should be finished before the TX IRQ is triggered.
    */
    *(up->base + UDR) = (u32)c;
}

void uart_handler(UART *up)
{
    u8 mis = *(up->base + MIS); //read MIS register
    if (mis & RX_BIT)
    {
        do_rx(up);
    }
    else if (mis & TX_BIT)
    {
        do_tx(up);
    }
    else
    {
        // print error msg on screen, not uart.
        kprintf("Something unpected happened in uart_handler()\n");
        while (1)
            ; // dead loop, something unexpected happened.
    }
}

/*
do_rx() is responsible to collect incoming chars into up->inbuf[].
This function just consume chars from the up->inbuf[].
*/
u8 ugetc(UART *up)
{
    u8 c;
    // while (up->indata <= 0)
    //     ; // no data in buffer, just block!
    // c = up->inbuf[up->intail];
    // kprintf("-----%d\n", up->indata);

    // The while loop is critical, it ensures a valid c is obtained from the uart inbuf.
    // Similar to kgetc() in keyboard driver.
    while(1)
    {
        lock();
        if(up->indata <= 0)
        {
            unlock();
            ksleep((u32)up); // task swich happens here!!
        }
        else
        {
            c = up->inbuf[up->intail];
            up->intail++;
            up->intail %= SBUFSIZE;
            if (up->intail == up->inhead) {
                up->wrap = FALSE;
            }
            up->indata--; // a buffered char is handled
            // kprintf("%d,", up->indata);
            up->inroom++; // a buffered char is handled
            unlock();
            return c;
        }

    }

    /*
    When updating the control variables in the upper-half of an interrupt-based device driver,
    we must ensure all the updating actions to the control variables are finished atomically.
    That is, it must not be interrupted. Otherwise there can be inconsistence.
    So we call lock() to disalbe IRQ for now.

    But in an interrupt handler(the lower-half), we don't need to worry about contention with the upper-half.
    Because we are sure that the upper-half has been interrupted and is not running.

    However, there's another issue.
    In this sample, the UART works in single-char mode.
    If the actions in upper-half take too long to finish, there can be >1 chars arriving at the UART.
    But the IRQ is disabled during the upper-half processing.
    So the do_rx() will not be invoked by the UART to collect the incoming chars in time.
    So it is possible that some char will be missed.

    And that is why there is a hardware FIFO buffer in the UART.

    In short, we need 2 buffers, one in software and one in hardware,
    to smoothly couple the hardware and software.

    */

    // below code will never execute
    // lock();
    // up->intail++;
    // up->intail %= SBUFSIZE;
    // up->indata--; // a buffered char is handled
    // up->inroom++; // a buffered char is handled
    // unlock();
    // return c;
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
    if (up->txon)
    {
        up->outbuf[up->outhead] = c;
        lock();
        up->outhead++;
        up->outhead %= SBUFSIZE;
        up->outdata++; // a new char is buffered
        up->outroom--; // a new char is buffered
        unlock();
        return;
    }
    //u32 i = *(up->base + UFR); // why do this?
    while (*(up->base + UFR) & TXFF)
        ; // if the tx holding register is full, busy wait

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
    up->txon = 1; // this line should precede the next line to ensure up-txon correctly reflect the status of TX interrupt.

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
    while ((*s = ugetc(up)) != '\r')
    {
        // below echo back won't work because it's part of the task, it won't get executed until a '\n' is received.
        // echo back should be placed in the uart interrupt handler
        // uputc(up, *s); // echo back as user is typing so user can see what he has just input.
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
    while (*s)
    {
        uputc(up, *s++);
    }
}

u8 *uart_tab = "0123456789ABCDEF";
void urpx(UART *up, u32 x)
{
    if (x == 0)
    {
        return;
    }

    u8 c = '0';
    if (x > 0)
    {
        c = uart_tab[x % 16];
        urpx(up, x / 16);
    }
    uputc(up, c);
}

void uprintx(UART *up, i32 x)
{
    if (x == 0)
    {
        uputc(up, '0');
        return;
    }

    if (x < 0)
    {
        uputc(up, '-');
        x = -x;
    }

    urpx(up, x);
}

/*
uart recursive print unsigned
*/
void urpu(UART *up, u32 x)
{
    if (x == 0)
    {
        return;
    }
    u8 c = '0';
    if (x > 0)
    {
        c = uart_tab[x % 10];
        urpu(up, x / 10);
    }
    uputc(up, c);
}

void uprintu(UART *up, u32 x)
{
    if (x == 0)
    {
        uputc(up, '0');
    }
    else
    {
        urpu(up, x);
    }
}

void uprinti(UART *up, i32 x)
{
    if (x < 0)
    {
        uputc(up, '-');
        x = -x;
    }

    uprintu(up, x);
}

void uprintf(UART *up, u8 *fmt, ...)
{
    u32 *ip;
    u8 *cp;
    cp = fmt;
    ip = (u32 *)((u32)&fmt + sizeof(u8 *)); //(u32*)&fmt + 1;

    while (*cp)
    {
        if (*cp != '%')
        {
            uputc(up, *cp);
            if (*cp == '\n')
            {
                uputc(up, '\r');
            }
            cp++;
            continue;
        }
        cp++;
        switch (*cp)
        {
        case 'c':
            uputc(up, (u8)*ip);
            break;
        case 's':
            uprints(up, (u8 *)*ip);
            break;
        case 'd':
            uprinti(up, *ip);
            break;
        case 'u':
            uprintu(up, *ip);
            break;
        case 'x':
            uprintx(up, *ip);
            break;
        }
        cp++;
        ip++;
    }
}