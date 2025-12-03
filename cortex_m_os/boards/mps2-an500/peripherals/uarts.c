#include <stdint.h>
#include <board_api.h>
#include <user_define_cm7.h>
#include <mps2-an500.h>
#include <uart/cmsdk_apb_uart.h>

static void Uart_C_Handler_Rx_Instance (uint32_t instance);
static void Uart_C_Handler_Tx_Instance (uint32_t instance);
static void Uart_012_C_Handler_Overrun (void);

/*
Convert the UART instance number to its reg block address.
*/
static void* uart_get_reg_block (uint32_t instance)
{
  switch (instance)
    {
    case 0: return (void*)UART_0_BASE;
    case 1: return (void*)UART_1_BASE;
    case 2: return (void*)UART_2_BASE;
    case 3: return (void*)UART_3_BASE;
    case 4: return (void*)UART_4_BASE;
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
  void* uart_regs = uart_get_reg_block (instance);
  uart_init_cmsdk_apb (uart_regs);
  return 0;
}

int32_t board_uart_tx_data (uint32_t instance, uint8_t* data, uint32_t count)
{
  // int32_t sent_count;

  // CMSDK_APB_UART_TYPE *regs = uart_get_reg_block(instance);
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
  uint8_t CR = '\r';
  uint8_t LF = '\n';
  CMSDK_APB_UART_TYPE* regs;


  regs = uart_get_reg_block (instance);
  // keep reading until not full
  while (regs->STATE & RX_BUFFER_FULL_MASK)
    {
      c = uart_rx_data_cmsdk_apb (regs);
      if (c != LF)
        {
          uart_tx_data_cmsdk_apb (regs, &c, 1);
        }
      if (c == CR)
        {
          uart_tx_data_cmsdk_apb (regs, &LF, 1);
        }
    }

  // Since the Rx buffer is not full now, always write 1 to clear the RX buffer overrun state.
  // Even there's no buffer overrun.
  regs->STATE = RX_BUFFER_OVERRUN_MASK;

  // Write 1 to clear the RX interrupt status.
  regs->INTSTS_CLR = RX_INT_FLAG | RX_OVERRUN_INT_FLAG;
  return;
}


static void Uart_C_Handler_Tx_Instance (uint32_t instance)
{
  uint8_t c;
  uint8_t CR = '\r';
  uint8_t LF = '\n';
  CMSDK_APB_UART_TYPE* regs;

  regs = uart_get_reg_block (instance);
  while (regs->STATE & TX_BUFFER_FULL_MASK)
    {
      // busy wait for the UART to finish the transmission
      ;
    }

  // Since the Tx buffer is not full now, always write 1 to clear the Tx buffer overrun state.
  // Even there's no buffer overrun.
  regs->STATE = TX_BUFFER_OVERRUN_MASK;

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
