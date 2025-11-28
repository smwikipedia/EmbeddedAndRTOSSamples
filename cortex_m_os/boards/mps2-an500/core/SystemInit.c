#include "board_api.h"
#include "user_define_cm7.h"


int32_t setup_periperals (void)
{
  board_uart_init (0);

  return 0;
}

void setup_nvic (void)
{
  /*
   Set priority and enable interrupt in NVIC
  */
  NVIC_SetPriority (UART0_RX_IRQn, 4);
  // Enable the 2 interrupts for UART0 TX and RX.
  NVIC_EnableIRQ (UART0_RX_IRQn);
  NVIC_EnableIRQ (UART0_TX_IRQn);
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
  setup_periperals ();
  setup_nvic ();

  /*
  Configure the special purpose regsiters

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
