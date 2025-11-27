#include "user_define_cm7.h"

void nvic_setup (void)
{

    /*
    For mps2-an500,
    CMSDK APB subsystem @ 0x40000000 0x4000FFFF
    UART0 @ 0x40004000-0x40004FFF
    UART0 CTRL reg @ offset 0x008 -> 0x40004008
    UART0 CTRL reg @ offset 0x010 -> 0x40004010
    UART0 CTRL RX enable bit @ bit 3
    */

    /*
    Step 1: Configure the UART IP

    According to: DDI0479D_m_class_processor_system_r1p1_trm.pdf
    You must program the baud rate divider register before enabling the UART.
    */
    *(uint32_t*)(0x40004010) = 1250;

    /*
    0 - TX enable
    1 - RX enable
    2 - TX interrupt enable
    3 - RX interrupt enable
    */
    *(uint8_t*)(0x40004008) = 0xF;

    /*
    Step 2: Configure the NVIC

    Set priority and enable interrupt in NVIC
    */
    NVIC_SetPriority (UART0_RX_IRQn, 4);
    // Enable the 2 interrupts for UART0 TX and RX.
    NVIC_EnableIRQ (UART0_RX_IRQn);
    NVIC_EnableIRQ (UART0_TX_IRQn);


    /*
    Step 3: Configure the special purpose regsiters

    Ref: DDI0403E_e_armv7m_arm.pdf
    PRIMASK The exception mask register, a 1-bit register.
    PRIMASK is set to 1 by the execution of the instruction CPSID i.
    PRIMASK is set to 0 by the execution of the instruction CPSIE i.

    Ref: DUI0646C_cortex_m7_dgug.pdf
    If set, the PRIMASK register prevents activation of all exceptions with
    configurable priority. Software uses the CPSIE I and CPSID I instructions to
    enable and disable interrupts. The CMSIS provides the following intrinsic
    functions for these instructions: void __disable_irq(void) // Disable
    Interrupts void __enable_irq(void) // Enable Interrupts
    */
    __enable_irq ();
}


void Uart_C_Handler_Rx (void) __attribute__ ((noreturn));
void Uart_C_Handler_Rx (void)
{
    while (1)
        ;
}

void Uart_C_Handler_Tx (void) __attribute__ ((noreturn));
void Uart_C_Handler_Tx (void)
{
    while (1)
        ;
}

void SystemInit (void)
{
    nvic_setup ();
}
