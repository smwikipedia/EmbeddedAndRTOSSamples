#include "types.h"
#include "versatilepb.h"
#include "string.h"
#include "display.h"


#include "pl011.h"
#include "tf-m/uart_pl011_drv.h"
#include "pl110.h"
#include "sp804.h"
#include "pl050.h"

// global data structure instances for the device instances on the versatilepb board
volatile UART uart[MAX_UART_NUMBER]; // 4 UART structures
struct uart_pl011_dev_t pl011_dev[MAX_UART_NUMBER];
struct uart_pl011_dev_cfg_t pl011_cfg[MAX_UART_NUMBER]; // 4 UART structures for TF_M PL011 driver
struct uart_pl011_dev_data_t pl011_data[MAX_UART_NUMBER]; // 4 UART structures for TF_M PL011 driver

volatile TIMER timer[MAX_TIMER_NUMBER]; //4 timers; 2 per unit; at 0x00 and 0x20
volatile KBD kbd; // 1 kbd; KBD data structure

void uart_init()
{
    for(u32 i=0; i<MAX_UART_NUMBER; i++)
    {
        uart[i].p_pl011_dev = &pl011_dev[i];
        pl011_dev[i].cfg = &pl011_cfg[i];
        pl011_dev[i].data = &pl011_data[i];

        pl011_cfg[i].def_baudrate = DEFAULT_UART_BAUDRATE;
        pl011_cfg[i].def_wlen = UART_PL011_WLEN_8;
        pl011_cfg[i].def_parity = UART_PL011_PARITY_DISABLED;
        pl011_cfg[i].def_stopbit = UART_PL011_STOPBIT_1;
    }

    kprintf("uart_init() with tf_m driver\n");
    for(u32 i=0; i<MAX_UART_NUMBER; i++)
    {
        UART *up = &uart[i];
        up->n = i; //UART ID, and it is the index into the pl011_cfg/data[] array.
        if(i != 3)
        {// uart 0 ~ 2 are adjacent
            // uart_init_single(up, VERSATILEPB_PL011_UART0 + i * 0x1000);
            uart_init_single_tf_m(up, VERSATILEPB_PL011_UART0 + i * 0x1000);
        }
        else
        {// uart 3 is different
            // uart_init_single(up, VERSATILEPB_PL011_UART3);
            uart_init_single_tf_m(up, VERSATILEPB_PL011_UART3);
        }
    }
}

void timer_init()
{
    int i;
    TIMER *tp;
    kprintf("timer_init()\n");
    for (i = 0; i < MAX_TIMER_NUMBER; i++)
    {
        tp = &timer[i];
        switch(i)
        {
            case 0:
                timer_init_single(tp, VERSATILEPB_SP804_TIMER0);
                break;
            case 1:
                timer_init_single(tp, VERSATILEPB_SP804_TIMER1);
                break;
            case 2:
                timer_init_single(tp, VERSATILEPB_SP804_TIMER2);
                break;
            case 3:
                timer_init_single(tp, VERSATILEPB_SP804_TIMER3);
                break;
        }

        *(tp->base + TLOAD) = 0x0; // reset
        *(tp->base + TVALUE) = 0xFFFFFFFF;
        *(tp->base + TRIS) = 0x0;
        *(tp->base + TMIS) = 0x0;
        *(tp->base + TLOAD) = 0x100;
        // CntlReg=1110-0110=|En=1|Periodic=1|IntE=1|Rsvd=0|scal=01|1=32bit|0=wrap|=0xE6
        *(tp->base + TCNTL) = 0x66; // Bit 7 is started in timer_start() to enable the timer module, so the bit 7 should be set to 0 here.
        *(tp->base + TBGLOAD) = 0x1C00; // timer counter value
        tp->tick = tp->hh = tp->mm = tp->ss = 0; // initialize wall clock
        strcpy((u8 *)tp->clock, "00:00:00");
    }
}


void board_init()
{
    VIC_INTENABLE = 0;
    VIC_INTENABLE |= UART0_IRQ_VIC_BIT;
    VIC_INTENABLE |= UART1_IRQ_VIC_BIT;

    VIC_INTENABLE |= TIMER01_IRQ_VIC_BIT; // timer0,1 at VIC.bit4
    VIC_INTENABLE |= TIMER23_IRQ_VIC_BIT; // timer2,3 at VIC.bit5

    VIC_INTENABLE |= SIC_IRQ_VIC_BIT; // SIC to PIC.bit31
    /* enable KBD IRQ on SIC */
    SIC_INTENABLE = 0;
    SIC_INTENABLE |= KMI0_IRQ_SIC_BIT; // KBD int=SIC.bit3

    fbuf_init(VERSATILEPB_PL110_LCD_BASE, VERSATILEPB_OSC1);
    uart_init();
    timer_init();
    kbd_init(&kbd, VERSATILEPB_PL050_KBD);

}



