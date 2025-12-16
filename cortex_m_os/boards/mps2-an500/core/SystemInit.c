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
  NVIC_SetPriority (NVIC_UART_0_RX_IRQN, NVIC_PRIORITY_UART_0);
  NVIC_SetPriority (NVIC_UART_0_TX_IRQN, NVIC_PRIORITY_UART_0);
  NVIC_SetPriority (NVIC_UART_1_RX_IRQN, NVIC_PRIORITY_UART_0);
  NVIC_SetPriority (NVIC_UART_1_TX_IRQN, NVIC_PRIORITY_UART_0);
  NVIC_SetPriority (NVIC_UART_2_RX_IRQN, NVIC_PRIORITY_UART_0);
  NVIC_SetPriority (NVIC_UART_2_TX_IRQN, NVIC_PRIORITY_UART_0);
  NVIC_SetPriority (UART_0_1_2_OVERRUN_IRQN, NVIC_PRIORITY_UART_0);

  // Enable the interrupts for UART0/1 TX and RX.
  NVIC_EnableIRQ (NVIC_UART_0_RX_IRQN);
  NVIC_EnableIRQ (NVIC_UART_0_TX_IRQN);
  NVIC_EnableIRQ (NVIC_UART_1_RX_IRQN);
  NVIC_EnableIRQ (NVIC_UART_1_TX_IRQN);
  NVIC_EnableIRQ (NVIC_UART_2_RX_IRQN);
  NVIC_EnableIRQ (NVIC_UART_2_TX_IRQN);
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


  /*
  The CMSIS core_cm7.h defines only one API for SysTick: SysTick_Config().
  It defaults to use the processor clock.

  How to determine the processor clock?
  A clock source goes through the PLL to get the SYSCLK.
  And SYSCLK goes through pre-scalers to get HCLK, i.e. processor clock.
  HCLK drives the Cortex-M processor core, the AHB bus, and usually high-speed peripherals.

  On a complex commercial MCU (like an STM32), SYSCLK is often the output of the main PLL and is then
  prescaled to derive HCLK (e.g., HCLK = SYSCLK / 1, HCLK = SYSCLK / 2, etc.).

  On simpler systems, especially FPGA-based ones like the MPS2-AN500 where a single, dominant clock domain is used
  for the entire system-on-chip (SoC) instantiated within the FPGA, SYSCLK often IS the HCLK (or they are the same frequency with a prescaler of 1).


  From the DAI0500B_cortex_m7_on_v2m_mps2.pdf, $4 Clocks,
  the SYSCLK is 25MHz. That is 25000000 Hz.

  To generate SysTick interrupt every 10ms, the RELOAD value (ticks) should be:
  10ms*25MHz = 10*10^(-3) * 25*10^6 = 25*10^4 = 250000

  To generate SysTick interrupt every 1s, the value is: 25000000
  But the SysTick reload value is greater than what the RELOAD register can hold: 0xFFFFFF = 16777215, that is 16777215/25000000=0.67s

  If the SysTick is not frequent enough, task switch may be not in time. System may be less responsive.
  If the SysTick is too frequent, the system will also be less responsive due to interrupt overhead.
  To solve this dilemma, we have to find the sweet spot by experiment and measeuring.

  As I tried on QEMU mps2-an500 board, 10ms is better than 1ms when compared to the wall clock.
  So I set the SysTick interrupt interval to 10ms/250000 ticks.
  And I move the UART output work out of ISR into the main.
  */
  SysTick_Config (250000);
}
