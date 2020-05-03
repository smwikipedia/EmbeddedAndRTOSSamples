#include "versatilepb.h"
#include "pl011.h"
#include "string.h"

/*
UART init
*/
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

/*
LCD frame buffer init
*/
u32 fbuf_init()
{
    fb = (u32 *)(0x200000); //frame buffer starts at 2M, decided by the board spec

    InitializeFontContext12x16();

    /* for 640x480 VGA */
    // *(volatile u32 *)(0x1000001c) = 0x00002C77;
    // *(volatile u32 *)(0x10120000) = 0x3F1F3F9C;
    // *(volatile u32 *)(0x10120004) = 0x090B61DF;
    // *(volatile u32 *)(0x10120008) = 0x067F1800;
    // gDisplayContext.screen_width = 640;
    // gDisplayContext.screen_height = 480;

    /* for 800x600 SVGA */
    *(volatile u32 *)(0x1000001c) = 0x00002CAC;
    *(volatile u32 *)(0x10120000) = 0x1313A4C4;
    *(volatile u32 *)(0x10120004) = 0x0505F6F7;
    *(volatile u32 *)(0x10120008) = 0x071F1800;
    gDisplayContext.screen_width = 800;
    gDisplayContext.screen_height = 600;


    gDisplayContext.cursor_row = 1;
    gDisplayContext.cursor_col = 1;
    gDisplayContext.max_col = gDisplayContext.screen_width / (gDisplayContext.font_display_width + gDisplayContext.h_font_space);
    gDisplayContext.max_row = gDisplayContext.screen_height / (gDisplayContext.font_display_height + gDisplayContext.v_font_space);
    gDisplayContext.cursor = '_';

    *(volatile u32 *)(0x10120010) = 0x200000; //fbuf
    *(volatile u32 *)(0x10120018) = 0x82B;
}

/*
4 SP804 timers
*/
volatile TIMER timer[4]; //4 timers; 2 per unit; at 0x00 and 0x20

void timer_init()
{
    int i;
    TIMER *tp;
    kprintf("timer_init()\n");
    for (i = 0; i < 4; i++)
    {
        tp = &timer[i];
        if (i == 0)
            tp->base = (u32 *)0x101E2000;
        if (i == 1)
            tp->base = (u32 *)0x101E2020;
        if (i == 2)
            tp->base = (u32 *)0x101E3000;
        if (i == 3)
            tp->base = (u32 *)0x101E3020;
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