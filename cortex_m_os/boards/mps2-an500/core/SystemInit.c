#include <board_api.h>
#include <mps2-an500.h>
#include <user_define_cm7.h>


int32_t setup_periperals (void)
{
  board_uart_init (UART_0);
  board_uart_init (UART_1);
  board_uart_init (UART_2);

  return 0;
}

void setup_nvic (void)
{
  /*
  Set priority and enable interrupt in NVIC
  NVIC interrupts are prioritized by updating an 8-bit field within a 32-bit register (each register supporting
  four interrupts). Priorities are maintained according to the Armv7-M prioritization scheme. See Exception
  priorities and preemption on page B1-526.

  See DDI0403E ARMv7M ARM $B.1.5.4
  The number of supported priority values is an IMPLEMENTATION DEFINED power of two in the range 8 to 256, and
  the minimum supported priority value is always 0. All priority value fields are 8-bits, and if an implementation
  supports fewer than 256 priority levels then low-order bits of these fields are RAZ.

  According to CMSIS5 Device/ARM/ARMCM7/Include/ARMCM7.h, CM7 implements __NVIC_PRIO_BITS=3 bits for priority.

  Refer: DDI0403E Cortex-M ARM
  With a Cortex-M NVIC,
  - when two interrupts of the SAME priority are "happening" (i.e., triggered or pending),
  only one bit will be set in the Interrupt Active Bit Register (IABR) at any given time for those specific interrupts.
  - when two interrupts of DIFFERENT priorities are happening,
  two bits will be set in the Interrupt Active Bit Register (IABR), specifically when preemption occurs.

  So it's simpler to assign all UART instances the same priority.
  So that I don't need to consult the Interrupt Priority Register to decide
  which UART is the trigger.

  */
  NVIC_SetPriority (UART_0_RX_IRQN, NVIC_PRIORITY_UART_0);
  NVIC_SetPriority (UART_0_TX_IRQN, NVIC_PRIORITY_UART_0);
  NVIC_SetPriority (UART_1_RX_IRQN, NVIC_PRIORITY_UART_0);
  NVIC_SetPriority (UART_1_TX_IRQN, NVIC_PRIORITY_UART_0);
  NVIC_SetPriority (UART_2_RX_IRQN, NVIC_PRIORITY_UART_0);
  NVIC_SetPriority (UART_2_TX_IRQN, NVIC_PRIORITY_UART_0);
  NVIC_SetPriority (UART_0_1_2_OVERRUN_IRQN, NVIC_PRIORITY_UART_0);

  // Enable the interrupts for UART0/1 TX and RX.
  NVIC_EnableIRQ (UART_0_RX_IRQN);
  NVIC_EnableIRQ (UART_0_TX_IRQN);
  NVIC_EnableIRQ (UART_1_RX_IRQN);
  NVIC_EnableIRQ (UART_1_TX_IRQN);
  NVIC_EnableIRQ (UART_2_RX_IRQN);
  NVIC_EnableIRQ (UART_2_TX_IRQN);
  NVIC_EnableIRQ (UART_0_1_2_OVERRUN_IRQN);
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
