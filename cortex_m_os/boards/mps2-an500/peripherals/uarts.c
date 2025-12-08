/*
The board layer is a thin wrapper over the uart driver.
It translate the logical UART instnace number into UART instance.
And then invoke the corresonding UART driver.

Why this extra board layer when we already have drivers for all kinds of peripherals?
The board layer exposes peripherals by the logical instance number on a board.
The logical instance number is translated into the register block address for that instance.
And the lower layer peripheral drivers work with the instance register block address.

The board layer hides the hardware details and provide a logical interface to the upper layers.
If the board have more than one models of periphrals of the same type, such as 2 UART models,
the upper layers don't need to care about the hardware details. Just use UART by instance number.


Some key aspects about the code:

1) When someone writes to the ring buffer, the ring buffer CAN and IS ALLOWED to overrun!
Such as when application writs data to uart via the sw ring buffer,
and when RX interrupt handler strores received data into the sw ring buffer.

2) In application code, disable the interrupt, then only application code can opearte on the shared data.
In the interrupt handler code, application code CANNOT interrupt the handler code, so only the handler code can operate on the shared data.
In both cases, ALWAYS ONLY ONE PARTY operates on the shared data.
This paradigm makes it easier to think.


*/

#include "ring_buffer.h"
#include <stdint.h>
#include <board_api.h>
#include <user_define_cm7.h>
#include <mps2-an500.h>
#include <uart/uart_common.h>
#include <uart/cmsdk_apb_uart.h>
#include <utilities.h>
#include <data_structures.h>

static void Uart_C_Handler_Rx_Instance (uint32_t instance);
static void Uart_C_Handler_Tx_Instance (uint32_t instance);
static void Uart_012_C_Handler_Overrun (void);

UART_CLASS uarts[5] = { 0 };

extern void disable_interrupt ();
extern void enable_interrupt ();

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
  UART_CLASS* uart      = uart_get_instance (instance);
  uart->regs            = uart_get_regs (instance);
  uart->rx.fn_recv_prev = recv_prev;
  uart->rx.fn_send_next = send_next;
  uart->tx.fn_recv_prev = recv_prev;
  uart->tx.fn_send_next = send_next;

  init_dlist_head (&uart->rx.buffer_chain_link);
  init_dlist_head (&uart->tx.buffer_chain_link);

  uart_init_cmsdk_apb (uart->regs);

  return 0;
}

/*
  The UART software ring buffer ALWAYS accepts data.
  This is what ring buffer is meant for.
  Data overrun IS expected.
*/
void board_uart_write_byte (uint32_t instance, uint8_t c)
{
  int32_t sent_count;
  UART_CLASS* uart;
  UART_REGS_CMSDK_APB* regs;

  uart = uart_get_instance (instance);
  regs = uart->regs;

  // disable just the UART x interrupt, so only below code modifies the ring buffer shared data.
  disable_interrupt ();

  // DO NOT directly check hardware readiness here.
  // Instead check if hw is taking data from sw buffer.
  // And let the driver tell that via a shared variable.
  // Otherwise maybe contention between this code and the hardware driver.
  if (uart->buffered_tx)
    {
      // hw is taking data from buffer, so we should write to sw buffer instead of directly writing to hw.
      // the sw buffer CAN overrun!
      uart->tx.buffer[uart->tx.n_head] = c;
      if (rb_full (&uart->tx))
        {
          // overrun happens!
          RB_ADVANCE_POS (uart->tx.n_tail);
        }
      RB_ADVANCE_POS (uart->tx.n_head);
      enable_interrupt ();
      return;
    }

  // buffered_tx flag is celar, which means sw buffer is empty.
  // in this case, we should write to the hw directly to kick off the tx.
  uart_fifo_fill_cmsdk_apb_uart (regs, &c, 1);
  enable_interrupt ();

  return;
}


int32_t board_uart_write (uint32_t instance, uint8_t* data, uint32_t count)
{
  int32_t sent_count;
  UART_CLASS* uart;

  uart = uart_get_instance (instance);

  sent_count = 0;
  while (sent_count < count)
    {
      // This function doesn't block. It just throws data to sw ring buffer or hw.
      // DO EXPECT data loss due to ring buffer overrun.
      board_uart_write_byte (instance, data[sent_count]);
      sent_count++;
    }

  // seems sent_count always equal to count, what if any error happens?
  return sent_count;
}


/*
  ret: read count in byte
*/
int32_t board_uart_read_byte (uint32_t instance, uint8_t* c)
{
  int32_t read_count;
  UART_CLASS* uart;
  // UART_REGS_CMSDK_APB* regs;

  uart = uart_get_instance (instance);
  // regs = uart->regs;

  // Disable just the UART x interrupt
  // So only below code modifies the ring buffer shared data.
  // This make it easier to think.
  disable_interrupt ();

  // DO NOT directly read from hardware here.
  // Always read from sw buffer.
  // And let the driver fill the buffer.
  // Otherwise maybe contention between this code and the hardware driver.
  if (!(rb_empty (&uart->rx)))
    {
      *c = uart->rx.buffer[uart->rx.n_tail];
      // Because ring buffer is not empty, so advancing tail won't reach the head.
      RB_ADVANCE_POS (uart->rx.n_tail);
      enable_interrupt ();
      // only one byte is read
      return 1;
    }

  enable_interrupt ();
  return 0;
}

/*
Read all the readable data from the sw ring buffer.
Until exceeds buffer size or sw ring buffer is empty.
*/
int32_t board_uart_read (uint32_t instance, uint8_t* buffer, uint32_t size)
{
  uint8_t c;
  int32_t read_count_one_time;
  uint32_t read_count_total;

  read_count_total = 0;

  do
    {
      read_count_one_time = board_uart_read_byte (instance, &c);
      if (read_count_one_time == 0)
        {
          break;
        }
      buffer[read_count_total] = c;
      read_count_total++;
    }
  while (read_count_one_time == 1 && read_count_total < size);

  return read_count_total;
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
  UART_CLASS* uart;
  UART_REGS_CMSDK_APB* regs;
  uint8_t c;

  uart = uart_get_instance (instance);
  regs = (UART_REGS_CMSDK_APB*)uart->regs;

  // Keep storing incoming data into sw buffer until no more data,
  // the sw buffer CAN overrun!
  while (uart_irq_rx_ready_cmsdk_apb_uart (regs))
    {
      uart_fifo_read_cmsdk_apb_uart (regs, &c, 1);
      uart->rx.buffer[uart->rx.n_head] = c;
      if (rb_full (&uart->rx))
        {
          // overrun happens!
          RB_ADVANCE_POS (uart->rx.n_tail);
        }
        RB_ADVANCE_POS (uart->rx.n_head);
      // just for echo back test
      // board_uart_write (instance, &c, 1);
    }
  // Since the Rx buffer is not full now, always write 1 to clear the RX buffer overrun state.
  // Even there's no buffer overrun.
  // Since no Zephyr RTOS UART API for this, so set the registers directly.
  uart_rx_buffer_overrun_clear_cmsdk_apb (regs);

  // Write 1 to clear the RX interrupt status.
  uart_irq_rx_clear_cmsdk_apb (regs);
  uart_irq_rx_overrun_clear_cmsdk_apb (regs);
  return;
}


/*
 TX interrupt only happens when UART is ready to transmit more data.
*/
static void Uart_C_Handler_Tx_Instance (uint32_t instance)
{
  UART_CLASS* uart;
  UART_REGS_CMSDK_APB* regs;
  uint8_t c;

  uart = uart_get_instance (instance);
  regs = uart->regs;

  // TX interrupt means UART is ready to transmit more data.
  // So clear tx and tx overrun interrupts.
  uart_irq_tx_clear_cmsdk_apb (regs);
  uart_irq_tx_overrun_clear_cmsdk_apb (regs);

  // This is the ISR code, no application code can interrupt it. So no need to disable UART interrupt.

  if (uart_tx_buffer_overrun_cmsdk_apb (regs))
    {
      // if the tx buffer is overrun, do not take more data from the sw ring buffer.
      // let the hw to continue tx.
      uart->buffered_tx = 1;
      return;
    }

  /*
  TX handler only takes data from ring buffer.
  Buffer empty, do nothing.
  */
  if (rb_empty (&uart->tx))
    {
      uart->buffered_tx = 0;
      return;
    }

  /*
  SW buffer not empty and hw tx buffer is not overrun. We got work to do.
  Fill the hardware tx buffer until it cannot accept more data or sw ring-buffer is drained.
  */

  // indicating buffered tx is in progress
  uart->buffered_tx = 1;
  while (uart_irq_tx_ready_cmsdk_apb_uart (regs))
    {
      c = uart->tx.buffer[uart->tx.n_tail];

      // send out the data
      uart_fifo_fill_cmsdk_apb_uart (regs, &c, 1);

      // advance the tail position
      RB_ADVANCE_POS (uart->tx.n_tail);

      // ring buffer drained, no more data to tx
      if (rb_empty (&uart->tx))
        {
          uart->buffered_tx = 0;
          break;
        }
    }
}


static void Uart_012_C_Handler_Overrun_Instance (uint32_t instance)
{
  // To save instructions, always handle both Tx and Rx buffer overrun.
  // Don't tell which kind of overrun it is.
  // There should be no harm.
  Uart_C_Handler_Rx_Instance (instance);
  Uart_C_Handler_Tx_Instance (instance);
}

/*
 There's only 1 bit in NVIC for overrun of UART 0/1/2.
 We cannot tell which UART has overrun, we have to check them one by one.
 For Rx overrun, collect data from hw into sw ring buffer until hw rx buffer is not full. Though there may be no application to consume it yet.
 For Tx overrun, transmit data until hw tx buffer is not full.

 The mps2-an500 board only have overrun interrupt for UART0/1/2. None for 4/5.
 So we only handle 0/1/2.
*/
static void Uart_012_C_Handler_Overrun (void)
{
  Uart_012_C_Handler_Overrun_Instance (UART_0);
  Uart_012_C_Handler_Overrun_Instance (UART_1);
  Uart_012_C_Handler_Overrun_Instance (UART_2);
}
