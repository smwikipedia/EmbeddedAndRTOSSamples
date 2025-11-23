#include <user_define_cm7.h>


// need to declare it with "__attribute__ ((noreturn))", otherwise UsageFault;

void nvic_setup(void)
{
    // Enable the 2 interrupts for UART0 TX and RX.
    NVIC_EnableIRQ (UART0_RX_IRQn);
    NVIC_EnableIRQ (UART0_TX_IRQn);

}
