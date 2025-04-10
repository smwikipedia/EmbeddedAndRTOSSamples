#include "types.h"
#include "versatilepb.h"
#include "pl190.h"
#include "pl011.h"
#include "pl110.h"
#include "sp804.h"
#include "pl050.h"
#include "display.h"

/*
Below up is global because it is convenient to use elsewhere.
Should be refactored in future.
*/
UART *up;

void main()
{
    u8 *p;
    u8 line[128];

    board_init();
    
    up = &uart[0];
    p = &_binary___resource_images_image_bmp_start; // symbol generated by the linker based on the input file name
    u8 *pLine = NULL;
    //show_bmp(p, 0, 0); // show the image at the screen location (0,0), this may hide some early content in screen
    u8 c = ' ';

    //uprints(up, "[Debug Will Start]\n\r");
    uprintf(up, "%s\n", "[Debug Will Start]");
    //kprintf1("%c,%s,%d,%u,0x%x\n", 'A', "SM", -123, 321, 0xABCD);
    kprintf("%c,%s,%d,%u,0x%x\n", 'A', "SM", -123, 321, 0xABCD);
    kprintf("%c,%c,%c,%c,%c\n", 'A', 'B', 'C', 'D', 'E');
    kprintf("%c\n", 'A');
    kprintf("%s\n", "SM");
    kprintf("%d\n", -123);
    kprintf("%u\n", 321);
    kprintf("0x%x\n", 0xABCD);
    kprintf("0x%x\n", -0xCDEF);

    // for (u32 i = 0; i < 4; i++)
    // { // start all 4 timers
    //     timer_start(i);
    // }

    while (1)
    {
        kprintf("Enter a line from KBD:\n");
        kgets(line);
        uprintf(up, line);
        kprintf("Keyboard input: %s\n", line);
        // uprints(up, "[Input something through UART and then press Enter echo them back through UART:]\n\r");
        // ugets(up, line);
        // uprints(up, "[And you have entered below line from UART:]\n\r");
        // uprints(up, line);
    }
}

void copy_vectors()
{
    extern u32 vectors_start, vectors_end;
    u32 *vectors_src = &vectors_start;
    u32 *vectors_dst = (u32 *)0;
    while (vectors_src < &vectors_end)
    {
        *vectors_dst++ = *vectors_src++;
    }
}

void IRQ_handler()
{
    u32 vicstatus = VIC_STATUS;
    u32 sicstatus = SIC_STATUS;

    //UART 0
    if (vicstatus & UART0_IRQ_VIC_BIT)
    {
        uart_handler(&uart[0]);
    }

    //UART 1
    if (vicstatus & UART1_IRQ_VIC_BIT)
    {
        uart_handler(&uart[1]);
    }

    // VIC status BITs: timer0,1=4, uart0=13, uart1=14
    if (vicstatus & TIMER01_IRQ_VIC_BIT)
    { // bit4=1:timer0,1, handle timer 0 and 1 one by one
        // KC.Wang's book use TVALUE (Current Value Register) in below line to check which timer trigger the interrupt. It seems not reliable.
        // The Masked Interrupt Status Register is much better.
        // Ref: https://stackoverflow.com/questions/61575520/inconsistent-irq-frequency-with-sp804-dual-timer-module-on-qemu-arm-versatilepb
        if (*(timer[0].base + TMIS) == 1) // timer 0
            timer_handler(0);
        if (*(timer[1].base + TMIS) == 1) // timer 1
            timer_handler(1);
    }
    if (vicstatus & TIMER23_IRQ_VIC_BIT)
    {                                     // bit5=1:timer2,3, handle timer 2 and 3 one by one
        if (*(timer[2].base + TMIS) == 1) // timer 2
            timer_handler(2);
        if (*(timer[3].base + TMIS) == 1) // timer 3
            timer_handler(3);
    }

    //KBD
    if (vicstatus & (1 << 31))
    { // PIC.bit31= SIC interrupts
        if (sicstatus & (1 << 3))
        { // SIC.bit3 = KBD interrupt
            kbd_handler();
        }
    }
}

/*
If this one is used, comment out the "irq_handler:" in reset.S
*/
void __attribute__((interrupt)) irq_handler()
{
    IRQ_handler();
}
