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

void uart_dump(UART *up)
{
    struct _uart_pl011_reg_map_t* p_uart =
        (struct _uart_pl011_reg_map_t*)up->p_pl011_dev->cfg->base;
    kprintf("uart%d dump:\n", up->n);
    kprintf(" uartlcr_h=0x%x\n",  p_uart->uartlcr_h);
    kprintf(" uartifls=0x%x\n",  p_uart->uartifls);
}

/*
Initialize a single UART.
Note that below code only works on QEMU ARM versatilepb board.
It seems QEMU automatically uses default value for baud rate resgiers.
So we don't do it here.
This is not correct for UART in real hardware.
*/
void uart_init_single_tf_m(UART *up, u32 uart_base)
{
    u32 i;
    struct uart_pl011_dev_t *pl011_dev = up->p_pl011_dev;

    up->base = (u8*)uart_base;
    pl011_dev->cfg->base = uart_base;
    
    uart_pl011_init(pl011_dev, VERSATILEPB_PL011_CLOCK);

    /*
    The fifo mode is ignored in QEMU PL011 model.
    So you won't see FIFO effect even with below fifl level config.
    ref:
    https://electronics.stackexchange.com/questions/744012/pl011-uart-missing-receiving-data-in-fifo-mode
    https://balau82.wordpress.com/2010/02/28/hello-world-for-bare-metal-arm-using-qemu/
    */
    uart_pl011_set_rx_fifo_lvl(pl011_dev, UART_PL011_RX_FIFO_LVL_1_2);
    uart_pl011_set_tx_fifo_lvl(pl011_dev, UART_PL011_TX_FIFO_LVL_1_2);

    uart_pl011_enable_intr(pl011_dev, (RX_BIT | TX_BIT));
    uart_pl011_enable(pl011_dev);
    uart_dump(up);

    up->n = i; //UART ID
    up->indata = up->inhead = up->intail = 0;
    up->inroom = SBUFSIZE;
    up->outdata = up->outhead = up->outtail = 0;
    up->outroom = SBUFSIZE;
    // up->wrap = FALSE;
    up->txon = 0;

    for (i=0; i<SBUFSIZE; i++)
    {
        up->inbuf[i] = 0;
        up->outbuf[i] = 0;
    }
}

/*
According to the PL011 3.4.2 URATRXINTR:

The receive interrupt changes state when one of the following events occurs:
- If the FIFOs are enabled and the receive FIFO reaches the programmed trigger 
level. When this happens, the receive interrupt is asserted HIGH. The receive 
interrupt is cleared by reading data from the receive FIFO until it becomes less 
than the trigger level, or by clearing the interrupt.
- If the FIFOs are disabled (have a depth of one location) and data is received 
thereby filling the location, the receive interrupt is asserted HIGH. The receive 
interrupt is cleared by performing a single read of the receive FIFO, or by clearing 
the interrupt.


The xinu code said: (https://github.com/xinu-os/xinu/blob/master/device/uart-pl011/uartInterrupt.c)
If FIFOs are enabled, this happens when the amount of data in the receive FIFO is greater
than or equal to the programmed trigger level.  If FIFOs are
disabled, this happens when the Rx holding register was filled
with one byte. 

So the two descrptions above are a bit different.
Since QEMU PL011 ignores the FIFO mode, I cannot get a definite answer.
I will try on a real board and get back.

----------

This function:
Collect chars from hw fifo into the sw inbuf

Read bytes from the receive FIFO until it is empty again.  (If
FIFOs are disabled, the Rx holding register acts as a FIFO of
size 1, so the code still works.)

In the xinu implementation,
https://github.com/xinu-os/xinu/blob/master/device/uart-pl011/uartInterrupt.c
it keeps reading the char from UART data register (fifo or not), and put into the input buffer
if theres space in the input buffer.
If the input buffer is full, still read char from the UART but simply ignore it and keep some statistics.

In my implmemeation, for simplicity, just drain the FIFO and collect everything into the input buffer.
Allow overrun in the input buffer. And keep no statistics.

*/
void do_rx_tf_m(UART *up)
{
    char c;

    // just drain data from RX FIFO and put it into input buffer,
    // wake up a process when '\r', but the process is not scheduled until ISR finishes,
    // when FIFO is drained, the UARTRXINTR will be automatically cleared, we don't need to clear it.
    // when the FIFO is drained, the input buffer will faithfully contain what's received.
    do {
        // always put the read char into the input buffer
        // do not check for room, allow overrun in the input buffer
        uart_pl011_read(up->p_pl011_dev, &c);
        // kprintf("=1= inhead=%d, intail=%d, indata=%d, inroom=%d\n", up->inhead, up->intail, up->indata, up->inroom);
        up->inbuf[up->inhead++] = c;
        up->inhead %= SBUFSIZE; //circular buffer

        // this is the old indata
        if (up->indata < SBUFSIZE) {
            // inhead and intail must be different before accepting the new c
            up->indata++;
            up->inroom--;
        } else {
            up->indata = SBUFSIZE;
            up->inroom = 0;
            // intail is push forwad by inhead
            // and one char is overrun
            up->intail = up->inhead;
        }

        // kprintf("=2= inhead=%d, intail=%d, indata=%d, inroom=%d\n", up->inhead, up->intail, up->indata, up->inroom);
        // echo immediately to LCD, LCD doesn't involve interrupt
        kprintf("%c", c);

        /*
          do not echo back to UART at here.
          echo in the uputc().
          the logic will be simpler.
        */
        // uputc(up,c);  // This cause the endless dead loop!

        /*
         shouldn't break here, keep draining the FIFO
         the input buffer will faithfully contain what's received.
        */
        // if(c == '\r')
        // {// '\r' is sent as a new line char
        //     kwakeup((u32)up);
        //     // break;
        // }
    } while (uart_pl011_is_readable(up->p_pl011_dev));

    // since we are in the tx isr, there must be something in FIFO,
    // and now it must be in the inbuf,
    // so we can wakeup the waiting proc.
    kwakeup((u32)up);
}


/*
According to the PL011 spec:
The transmit interrupt changes state when one of the following events occurs:
- If the FIFOs are enabled and the transmit FIFO reaches the programmed trigger 
level. When this happens, the transmit interrupt is asserted HIGH. The transmit 
interrupt is cleared by writing data to the transmit FIFO until it becomes greater 
than the trigger level, or by clearing the interrupt.
- If the FIFOs are disabled (have a depth of one location) and there is no data 
present in the transmitters single location, the transmit interrupt is asserted HIGH. 
It is cleared by performing a single write to the transmit FIFO, or by clearing the 
interrupt.

To update the transmit FIFO you must:
- Write data to the transmit FIFO, either prior to enabling the UART and the 
interrupts, or after enabling the UART and interrupts.


The xinu code said: (https://github.com/xinu-os/xinu/blob/master/device/uart-pl011/uartInterrupt.c)
If FIFOs are enabled, this happens when the amount of data in the transmit FIFO is less than
or equal to the programmed trigger level.  If FIFOs are disabled,
this happens if the Tx holding register is empty.

So the two descrptions above are a bit different.
Since QEMU PL011 ignores the FIFO mode, I cannot get a definite answer.
I will try on a real board and get back.

*/
void do_tx_tf_m(UART *up)
{
    u8 c;

    // clear the TX interrupt explicilty, not just mask it with uart_pl011_disable_intr()
    /* Explicitly clear the transmit interrupt.  This is necessary
     * because there may not be enough bytes in the output buffer to
     * fill the FIFO greater than the transmit interrupt trigger level.
     * If FIFOs are disabled, this applies if there are 0 bytes to
     * transmit and therefore nothing to fill the Tx holding register
     * with.
     */
    uart_pl011_clear_intr(up->p_pl011_dev, UART_PL011_TX_INTR_MASK);
    if (up->outdata > 0) {
        // get data from output buffer and put it into the TX FIFO
        // until the output buffer is empty, or the TX FIFO is full
        do {
            c = up->outbuf[up->outtail++];
            up->outtail %= SBUFSIZE;
            up->outdata--;
            up->outroom++;
            // a buffered char is put into tx fifo
            uart_pl011_write(up->p_pl011_dev, c);
        } while(uart_pl011_is_writable(up->p_pl011_dev) && up->outdata > 0);
    } else {
        // nothing in output buffer
        // PL011 nees another kick from the upper half code to start the transmission
        up->txon = 0;
    }
}


void uart_handler(UART *up)
{
    // u8 mis = *(up->base + MIS); //read MIS register
    u8 mis = uart_pl011_get_masked_intr_status(up->p_pl011_dev);

    /* Receive interrupt is asserted.  If FIFOs are enabled, this
     * happens when the amount of data in the receive FIFO is greater
     * than or equal to the programmed trigger level.  If FIFOs are
     * disabled, this happens when the Rx holding register was filled
     * with one byte.  */
    if (mis & RX_BIT)
    {
        // kprintf("RX int!\n");
        do_rx_tf_m(up);
    }

    /*
    If the FIFO is in effect, you should see a rx timeout interrupt (RT_BIT).
    But fifo mode is ignored in QEMU PL011 model.
    So below code won't be reached on QEMU versatilepb.
    ref:
    https://electronics.stackexchange.com/questions/744012/pl011-uart-missing-receiving-data-in-fifo-mode
    https://balau82.wordpress.com/2010/02/28/hello-world-for-bare-metal-arm-using-qemu/
    */
    else if (mis & RT_BIT)
    {
        // kprintf("RT int!\n");
        // do_rx_tf_m(up);
    }

    /* Transmit interrupt is asserted.  If FIFOs are enabled, this
     * happens when the amount of data in the transmit FIFO is less than
     * or equal to the programmed trigger level.  If FIFOs are disabled,
     * this happens if the Tx holding register is empty.
     */
    else if (mis & TX_BIT)
    {
        // kprintf("TX int!\n");
        // do_tx(up);
        do_tx_tf_m(up);
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
do_rx_tf_m() is responsible to collect incoming chars from hardware RX FIFO into inbuf[].
This function just consume chars from the inbuf[].

When updating the control variables in the upper-half of an interrupt-based device driver,
we must ensure all the updating actions to the control variables are finished atomically.
That is, it must not be interrupted. Otherwise there can be inconsistence.
So we call lock() to disalbe IRQ for now.

But in an interrupt handler(the lower-half), we don't need to worry about contention with the upper-half.
Because we are sure that the upper-half has been interrupted and is not running.

However, there's another issue.
In this sample, the UART works in single-char mode.
If the actions in upper-half take too long to finish, there can be >1 chars arriving at the UART hardware.
But the IRQ is disabled during the upper-half processing.
So the isr do_rx_tf_m() will not be invoked by the UART to collect the incoming chars in time.
So it is possible that some char will be missed.
And that is why there is a hardware FIFO buffer in the UART.

In short, we need 2 buffers, one in software and one in hardware,
to smoothly couple the hardware and software.
*/
u8 ugetc(UART *up)
{
    u8 c;
    lock();
    if(up->indata == 0)
    {
        // kprintf("indata=%d, must be 0\n", up->indata);
        unlock();
        ksleep((u32)up); // task switch happens here!!
    }

    // if reached here, ISR must have picked at least one char from the UART hw fifo into the sw inbuf.
    // so up->indata must > 0
    // kprintf("=3= inhead=%d, intail=%d, indata=%d, inroom=%d\n", up->inhead, up->intail, up->indata, up->inroom);
    lock();
    c = up->inbuf[up->intail];
    up->intail++;
    up->intail %= SBUFSIZE;

    // because the indata must > 0, we can always do below
    // and inroom has been taken good care of the rx isr
    up->indata--;
    up->inroom++;
    unlock();

    // uart echo back should be placed here,
    // if placed in the do_rx_tf_m(), weird things will happen, such as deadloop.
    // putting it here makes things simple
    uputc(up,c);
    return c;
}


/*
Below code can write a char to 2 different destinaitions.
When output buffer is empty (txon==0), we write one char to PL011 direfctly.
When output buffer is not empty (txon==1), we write one char to output buffer.

According to PL011 TRM,
"Write data to the transmit FIFO, either prior to enabling the UART and the interrupts, 
or after enabling the UART and interrupts."

CPU is much faster than the PL011.
The isr do_tx_tf_m() will set txon=0 when the the software output ring buffer is empty. (NOTE! it is not the hardware tx FIFO)
When PL011 is sending a char, there may be many chars sent to uputc().
They will be put into the output ring buffer.

So the whole paradigm is:
We just kick start the PL011 transmission by writing the first byte to its data regiser,
and then the isr do_tx_tf_m() will automatically collect data from output buffer and transmit it.
If some time during this process, the output buffer becomes empty, the isr do_tx_tf_m() will
set the txon=0. And the upper half code, i.e. uputc() will kick start again by directly writing
to the PL011 data register.
The whole process is so delicate and fascinating, isn't it!

And to understand the whole process, txon is the key!
*/
void uputc(UART *up, u8 c)
{

    lock();
    // up->txon is shared between upper half and bottom half
    // so lock before use
    if (up->txon)
    {
        // always put new data into the ring buffer outbuf[]
        // for simplicity, allow overrun
        up->outbuf[up->outhead++] = c;
        up->outhead %= SBUFSIZE;

        // this is the old outdata
        if (up->outdata < SBUFSIZE) {
            // outhead and outtail must be different before stuffing in the new c
            // and we must be able to do below
            up->outdata++;
            up->outroom--;
        } else {
            up->outdata = SBUFSIZE;
            up->outroom = 0;
            // intail is push forwad by inhead
            // and one char is overrun
            // btw, this is very similar to the do_rx_tf_m()
            up->outtail = up->outhead;
        }
        unlock();
        return;
    }

    // txon==0, which means PL011 has drained the output buffer,
    // Now we need to give PL011 another kick to start another transmission.
    // this is a shared variable, so need lock.
    up->txon = 1;
    unlock();

    // Here's another kick start to the PL011.
    uart_pl011_write(up->p_pl011_dev, c);
}

void ugets(UART *up, char *s)
{
    // kprintf("1111\n");
    while ((*s = ugetc(up)) != '\r')
    {
        s++;
    }

    // stick to CR LF, that is \r\n
    // actually, we should support either \r\n or \n\r,
    // because geometrically, they are equivalent
    s++;
    *s++ = '\n'; // add \n to move to next line, \r only moves to the head of current line
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