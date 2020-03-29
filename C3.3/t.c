#include "types.h"
#include "versatilepb_pl190_vic.h"
#include "uart.h"

extern UART uart[4];
extern void uart_handler(UART *up);

extern void uprints(UART *up, u8 *s);
extern void ugets(UART *up, char *s);

void main()
{
    u8 line[128];
    UART *up;
    VIC_INTENABLE |= UART0_IRQ_VIC_BIT;
    VIC_INTENABLE |= UART1_IRQ_VIC_BIT;

    uart_init();
    up= &uart[0];

    while(1)
    {
        uprints(up, "Enter a line from UART:\n\r");
        ugets(up, line);
        uprints(up, "You have entered below line from UART:\n\r");
        uprints(up, line);
    }
}

void copy_vectors()
{
    extern u32 vectors_start, vectors_end;
    u32 *vectors_src = &vectors_start;
    u32 *vectors_dst = (u32 *)0;
    while(vectors_src < &vectors_end)
    {
        *vectors_dst++ = *vectors_src++;
    }
}

void IRQ_handler()
{
    u32 vicstatus = VIC_STATUS;

    //UART 0
    if(vicstatus & UART0_IRQ_VIC_BIT)
    {
        uart_handler(&uart[0]);
    }

    //UART 1
    if(vicstatus & UART1_IRQ_VIC_BIT)
    {
        uart_handler(&uart[1]);
    }

}