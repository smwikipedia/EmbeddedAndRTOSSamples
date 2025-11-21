#include <user_define_cm7.h>


// need to declare it with "__attribute__ ((noreturn))", otherwise UsageFault;

void nvic_setup(void)
{
    NVIC_EnableIRQ (UART0_RX_IRQn);
    NVIC_EnableIRQ (UART0_TX_IRQn);
}
