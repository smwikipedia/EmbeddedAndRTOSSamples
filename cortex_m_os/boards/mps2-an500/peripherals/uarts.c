/*
This is board-specific code. So it can leverage the full knowledge of mps2-an500 board.

Why this board layer when we already have the drivers for all kinds of peripherals?
The board layer exposes peripheral API based on the logical instance number of peripherals on a board.
The logical instance number is translated into the register block address for that instance.
And then lower layer peripheral drivers are called.

On top of the board layer are the kernel or subsys layers.
The board layer hides the hardware details and provide a logical interface to the upper layers.

*/

#include <stdint.h>
#include <board_api.h>
#include <user_define_cm7.h>
#include <mps2-an500.h>
#include <uart/cmsdk_apb_uart.h>

static void Uart_C_Handler_Rx_Instance (uint32_t instance);
static void Uart_C_Handler_Tx_Instance (uint32_t instance);
static void Uart_012_C_Handler_Overrun (void);

UART_CLASS uarts[5] = { 0 };

/*
Convert the UART instance number to its reg block address.
*/
static UART_CLASS* uart_get_instance (uint32_t instance)
{
  if (instance >= 5)
    {
      DEAD_LOOP;
    }
  return &uarts[instance];
}

static UART_REGS_CMSDK_APB* uart_get_regs (uint32_t instance)
{
  switch (instance)
    {
    case UART_0: return (UART_REGS_CMSDK_APB*)UART_0_BASE;
    case UART_1: return (UART_REGS_CMSDK_APB*)UART_1_BASE;
    case UART_2: return (UART_REGS_CMSDK_APB*)UART_2_BASE;
    case UART_3: return (UART_REGS_CMSDK_APB*)UART_3_BASE;
    case UART_4: return (UART_REGS_CMSDK_APB*)UART_4_BASE;
    default: DEAD_LOOP;
    }

  return 0;
}


uint32_t board_uart_get_total_instance_number (void)
{
  return MPS2_AN550_UART_MAX_NUM;
}


int32_t board_uart_init (uint32_t instance)
{
  UART_CLASS* p_uart = uart_get_instance (instance);
  p_uart->regs       = uart_get_regs (instance);
  uart_init_cmsdk_apb (p_uart->regs);
  return 0;
}

int32_t board_uart_tx_data (uint32_t instance, uint8_t* data, uint32_t count)
{
  // int32_t sent_count;

  // UART_REGS_CMSDK_APB *regs = uart_get_instance(instance);
  // sent_count = uart_tx_data_cmsdk_apb(regs, data, count);
  // return sent_count;
  return 0;
}


/*
This is board-specific code. So it has the knowledge of the board.
mps2-an500 board has 5 UARTs.
Check the NVIC to determine which UART is the interrupt source.

The DUI0646C spec says:
If multiple pending exceptions have the same priority, the pending
exception with the lowest exception number takes precedence.

With a Cortex-M NVIC, from AI:
- when two interrupts of the SAME priority are "happening" (i.e., triggered or pending),
only one bit will be set in the Interrupt Active Bit Register (IABR) at any given time for those specific interrupts.
- when two interrupts of DIFFERENT priorities are happening,
two bits will be set in the Interrupt Active Bit Register (IABR), specifically when preemption occurs.

Conclusion:
So it's simpler to assign ALL UART instances the SAME priority for TX, Rx and overrun.
This ensures the active bits of all Tx or Rx interrupts of all UARTs will be set one by one.
And they are serviced one by one.
*/
void Uart_C_Handler (void)
{
  // determine the instance
  // For mps2-an500, all 5 UARTs interrupt bits fall in IABR[0]
  // note: iabr captures the snapshot of this moment.
  // But until this handler exits, the UARTs bits in the NVIC->IABR[0] won't change because of the identical priorities for all UARTs.
  uint32_t iabr = NVIC->IABR[0];
  // DO NOT mix the handling of RX and TX in the same function.

  // Only one of the if conditions below will be entered for one call to the Uart_C_Handler().
  // Because all the UARTs' Tx/Rx interrupt has the same priority.
  // Only one interrupt will be triggered for ONE UART's Tx OR Rx interrupt.
  // To reduce time, return in each if block.
  if (iabr & UART_0_RX_INT_ACTIVE_BIT)
    {
      return Uart_C_Handler_Rx_Instance (UART_0);
    }

  if (iabr & UART_0_TX_INT_ACTIVE_BIT)
    {
      return Uart_C_Handler_Tx_Instance (UART_0);
    }

  if (iabr & UART_1_RX_INT_ACTIVE_BIT)
    {
      return Uart_C_Handler_Rx_Instance (UART_1);
    }

  if (iabr & UART_1_TX_INT_ACTIVE_BIT)
    {
      return Uart_C_Handler_Tx_Instance (UART_1);
    }

  if (iabr & UART_2_RX_INT_ACTIVE_BIT)
    {
      return Uart_C_Handler_Rx_Instance (UART_2);
    }

  if (iabr & UART_2_TX_INT_ACTIVE_BIT)
    {
      return Uart_C_Handler_Tx_Instance (UART_2);
    }

  if (iabr & UART_3_RX_INT_ACTIVE_BIT)
    {
      return Uart_C_Handler_Rx_Instance (UART_3);
    }

  if (iabr & UART_3_TX_INT_ACTIVE_BIT)
    {
      return Uart_C_Handler_Tx_Instance (UART_3);
    }

  if (iabr & UART_4_RX_INT_ACTIVE_BIT)
    {
      return Uart_C_Handler_Rx_Instance (UART_4);
    }

  if (iabr & UART_4_TX_INT_ACTIVE_BIT)
    {
      return Uart_C_Handler_Tx_Instance (UART_4);
    }

  // One of the three UARTs 0/1/2 get overrun
  if (iabr & UART_0_1_2_OVERRUN_IRQN)
    {
      return Uart_012_C_Handler_Overrun ();
    }
}

/*
 Refer to drivers/uart/cmsdk_apb_uart/README for how cmsdk_apb_uart works.

*/
static void Uart_C_Handler_Rx_Instance (uint32_t instance)
{
  uint8_t c;
  UART_CLASS* p_uart;
  int32_t read_count;
  int32_t send_count;
  uint8_t raw_buffer[16];
  UART_REGS_CMSDK_APB* regs;

  p_uart = uart_get_instance (instance);
  regs   = (UART_REGS_CMSDK_APB*)p_uart->regs;

  // keep reading until not full
  while (uart_irq_rx_ready_cmsdk_apb_uart (regs))
    {
      read_count = uart_fifo_read_cmsdk_apb_uart (regs, raw_buffer, sizeof (raw_buffer));
      while (read_count > 0)
        {
          // Below 2 lines are just for the echo back test.
          // Shoud remove it in real life.
          // We should send out data as-is. We must not convert CR to CRLF at here.
          // The conversion should be done by the client.
          // With picocom, you can use the "--omap crcrlf".
          // This is the output mapping feature that maps CR to CRLF.
          send_count = uart_fifo_fill_cmsdk_apb_uart (regs, raw_buffer, read_count);
          read_count -= send_count;
        }
    }
  // Since the Rx buffer is not full now, always write 1 to clear the RX buffer overrun state.
  // Even there's no buffer overrun.
  // Since no Zephyr RTOS UART API for this, so set the registers directly.
  regs->STATE = RX_BUFFER_OVERRUN_FLAG;

  // Write 1 to clear the RX interrupt status.
  regs->INTSTS_CLR = RX_INT_FLAG | RX_OVERRUN_INT_FLAG;
  return;
}


/*
 TX interrupt only happens when TX has finished.
*/
static void Uart_C_Handler_Tx_Instance (uint32_t instance)
{
  UART_CLASS* p_uart;
  UART_REGS_CMSDK_APB* regs;

  p_uart = uart_get_instance (instance);
  regs   = p_uart->regs;

  // Since the Tx buffer is not full now, always write 1 to clear the Tx buffer overrun state.
  // Even there's no buffer overrun.
  regs->STATE = TX_BUFFER_OVERRUN_FLAG;

  // Write 1 to clear the TX interrupt status.
  regs->INTSTS_CLR = TX_INT_FLAG | TX_OVERRUN_INT_FLAG;
}


static void Uart_012_C_Handler_Overrun_Instance (uint32_t instance)
{
  // To save instructions, always handle both Tx and Rx buffer overrun.
  // Don't tell which one it is.
  // There should be no harm.
  Uart_C_Handler_Rx_Instance (instance);
  Uart_C_Handler_Tx_Instance (instance);
}

/*
 The overrun interrupt doesn't tell which UART has overrun, we have to check them one by one.
 We check if it is Rx/Tx overrun.
 For Rx overrun, drain the data.
 For Tx overrun, busy wait.

 The mps2-an500 board only have overrun interrupt for UART0/1/2.
 None for 4/5.
*/
static void Uart_012_C_Handler_Overrun (void)
{
  Uart_012_C_Handler_Overrun_Instance (UART_0);
  Uart_012_C_Handler_Overrun_Instance (UART_1);
  Uart_012_C_Handler_Overrun_Instance (UART_2);
}
