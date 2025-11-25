#include "user_define_cm7.h"

void nvic_setup (void)
{
    // Enable the 2 interrupts for UART0 TX and RX.
    NVIC_EnableIRQ (UART0_RX_IRQn);
    NVIC_EnableIRQ (UART0_TX_IRQn);
}

void SystemInit (void)
{
    nvic_setup ();
}
