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

1) Ring buffer is expected to overrun
When someone writes to the ring buffer, the ring buffer CAN and IS ALLOWED to overrun!
Such as when application writes data to uart via the sw ring buffer,
and when RX interrupt handler collects received data into the sw ring buffer.

2) The paradigm of shard data manipulation
In application code, disable the interrupt, then only application code can opearte on the shared data.
In the interrupt handler code, application code CANNOT interrupt the handler code, so only the handler code can operate on the shared data.
In both cases, ALWAYS ONLY ONE PARTY operates on the shared data.
This paradigm makes it easier to think.

3) Disable/Enable the interrupts at the NVIC level. Not the UART device.
As I tried, if I disable/enable interrupts in the UART device, QEMU UART won't work.
Maybe there's a bug in the QEMU implementaion: https://gitlab.com/qemu-project/qemu/-/issues/3244

Grok AI also suggests that, to handle the quick interrupt on/off scenarios, disable/enable the interrupt at the NVIC level instead.
This masks the interrupt at the interrupt controller while preserving the peripheral's assertion state.
So unmasking the NVIC will trigger an interrupt if the condition is still active.
This approach is standard best pracitce in ARM systems and avoids the peripheral-specific quirks.

So, I will do it with "nvic_switch_uart_tx_int()".
Not the "uart_irq_tx_disable_cmsdk_apb_uart()", etc.

*/

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

/*
Enable/Disable the RX interrupt at the NVIC level.
*/
static void nvic_switch_uart_rx_int (uint32_t instance, uint32_t on)
{
  switch (instance)
    {
    case UART_0:
      (on == 1) ? NVIC_EnableIRQ (NVIC_UART_0_RX_IRQN) : NVIC_DisableIRQ (NVIC_UART_0_RX_IRQN);
      break;
    case UART_1:
      (on == 1) ? NVIC_EnableIRQ (NVIC_UART_1_RX_IRQN) : NVIC_DisableIRQ (NVIC_UART_1_RX_IRQN);
      break;
    case UART_2:
      (on == 1) ? NVIC_EnableIRQ (NVIC_UART_2_RX_IRQN) : NVIC_DisableIRQ (NVIC_UART_2_RX_IRQN);
      break;
    case UART_3:
      (on == 1) ? NVIC_EnableIRQ (NVIC_UART_3_RX_IRQN) : NVIC_DisableIRQ (NVIC_UART_3_RX_IRQN);
      break;
    case UART_4:
      (on == 1) ? NVIC_EnableIRQ (NVIC_UART_4_RX_IRQN) : NVIC_DisableIRQ (NVIC_UART_4_RX_IRQN);
      break;
    }
}

/*
Enable/Disable the TX interrupt at the NVIC level.
*/
static void nvic_switch_uart_tx_int (uint32_t instance, uint32_t on)
{
  switch (instance)
    {
    case UART_0:
      (on == 1) ? NVIC_EnableIRQ (NVIC_UART_0_TX_IRQN) : NVIC_DisableIRQ (NVIC_UART_0_TX_IRQN);
      break;
    case UART_1:
      (on == 1) ? NVIC_EnableIRQ (NVIC_UART_1_TX_IRQN) : NVIC_DisableIRQ (NVIC_UART_1_TX_IRQN);
      break;
    case UART_2:
      (on == 1) ? NVIC_EnableIRQ (NVIC_UART_2_TX_IRQN) : NVIC_DisableIRQ (NVIC_UART_2_TX_IRQN);
      break;
    case UART_3:
      (on == 1) ? NVIC_EnableIRQ (NVIC_UART_3_TX_IRQN) : NVIC_DisableIRQ (NVIC_UART_3_TX_IRQN);
      break;
    case UART_4:
      (on == 1) ? NVIC_EnableIRQ (NVIC_UART_4_TX_IRQN) : NVIC_DisableIRQ (NVIC_UART_4_TX_IRQN);
      break;
    }
}


// extern void disable_interrupt ();
// extern void enable_interrupt ();

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
    case UART_0: return (UART_REGS_CMSDK_APB*)BOARD_UART_0_BASE;
    case UART_1: return (UART_REGS_CMSDK_APB*)BOARD_UART_1_BASE;
    case UART_2: return (UART_REGS_CMSDK_APB*)BOARD_UART_2_BASE;
    case UART_3: return (UART_REGS_CMSDK_APB*)BOARD_UART_3_BASE;
    case UART_4: return (UART_REGS_CMSDK_APB*)BOARD_UART_4_BASE;
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
  UART_CLASS* uart = uart_get_instance (instance);
  uart->regs       = uart_get_regs (instance);

  /*
  Some buffer chain experiemnt, seems excessive for now.
  */
  // uart->rx.fn_recv_prev = recv_prev;
  // uart->rx.fn_send_next = send_next;
  // uart->tx.fn_recv_prev = recv_prev;
  // uart->tx.fn_send_next = send_next;

  // init_dlist_head (&uart->rx.buffer_chain_link);
  // init_dlist_head (&uart->tx.buffer_chain_link);

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

  // Disable interrupt of UARTx, so only below code modifies the ring buffer shared data.
  // Only need to disable tx interrupt, because tx and rx buffer are seperated.
  nvic_switch_uart_tx_int (instance, 0);

  // Disable at the NVIC level. Not at the UART level. Just to avoid hardware quirks.
  // uart_irq_tx_disable_cmsdk_apb_uart(regs);

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

      // enable interrupt
      nvic_switch_uart_tx_int (instance, 1);
      // uart_irq_tx_enable_cmsdk_apb_uart(regs);
      return;
    }

  // buffered_tx flag is celar, which means sw buffer is empty.
  // in this case, we should write to the hw directly to kick off the tx.
  uart_fifo_fill_cmsdk_apb_uart (regs, &c, 1);

  // enable interrupt;
  nvic_switch_uart_tx_int (instance, 1);
  // uart_irq_tx_enable_cmsdk_apb_uart(regs);

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
  UART_REGS_CMSDK_APB* regs;

  uart = uart_get_instance (instance);
  regs = uart->regs;

  // Disable just the UART x interrupt
  // So only below code modifies the ring buffer shared data.
  // This make it easier to think.
  // Only need to disable tx interrupt, because tx and rx buffer are seperated.
  nvic_switch_uart_rx_int (instance, 0);
  // uart_irq_rx_disable_cmsdk_apb_uart(regs);

  // DO NOT directly read from hardware here.
  // Always read from sw buffer.
  // And let the driver fill the buffer.
  // Otherwise maybe contention between this code and the hardware driver.
  if (!(rb_empty (&uart->rx)))
    {
      *c = uart->rx.buffer[uart->rx.n_tail];
      // Because ring buffer is not empty, so advancing tail won't reach the head.
      RB_ADVANCE_POS (uart->rx.n_tail);

      // enable interrupt
      nvic_switch_uart_rx_int (instance, 1);
      // uart_irq_rx_enable_cmsdk_apb_uart(regs);
      // only one byte is read
      return 1;
    }

  // enable interrupt
  nvic_switch_uart_rx_int (instance, 1);
  // uart_irq_rx_enable_cmsdk_apb_uart(regs);

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
  if (iabr & NVIC_UART_0_RX_INT_ACTIVE_BIT)
    {
      return Uart_C_Handler_Rx_Instance (UART_0);
    }

  if (iabr & NVIC_UART_0_TX_INT_ACTIVE_BIT)
    {
      return Uart_C_Handler_Tx_Instance (UART_0);
    }

  if (iabr & NVIC_UART_1_RX_INT_ACTIVE_BIT)
    {
      return Uart_C_Handler_Rx_Instance (UART_1);
    }

  if (iabr & NVIC_UART_1_TX_INT_ACTIVE_BIT)
    {
      return Uart_C_Handler_Tx_Instance (UART_1);
    }

  if (iabr & NVIC_UART_2_RX_INT_ACTIVE_BIT)
    {
      return Uart_C_Handler_Rx_Instance (UART_2);
    }

  if (iabr & NVIC_UART_2_TX_INT_ACTIVE_BIT)
    {
      return Uart_C_Handler_Tx_Instance (UART_2);
    }

  if (iabr & NVIC_UART_3_RX_INT_ACTIVE_BIT)
    {
      return Uart_C_Handler_Rx_Instance (UART_3);
    }

  if (iabr & NVIC_UART_3_TX_INT_ACTIVE_BIT)
    {
      return Uart_C_Handler_Tx_Instance (UART_3);
    }

  if (iabr & NVIC_UART_4_RX_INT_ACTIVE_BIT)
    {
      return Uart_C_Handler_Rx_Instance (UART_4);
    }

  if (iabr & NVIC_UART_4_TX_INT_ACTIVE_BIT)
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
 The RX interrupt happens when data is ready to be consumed.
 So we undoubtedly MUST read 1 byte.

*/
static void Uart_C_Handler_Rx_Instance (uint32_t instance)
{
  UART_CLASS* uart;
  UART_REGS_CMSDK_APB* regs;
  uint8_t c;

  uart = uart_get_instance (instance);
  regs = (UART_REGS_CMSDK_APB*)uart->regs;

  /*
  Step 1. The first thing is to clear the interrupt flag for this time.
  */
  // The Rx buffer must be not full now.
  // Clear the RX interrupt status.
  uart_irq_rx_clear_cmsdk_apb (regs);
  // Clear the RX overrun interrupt if any.
  // According to DDI0479D, this also clears the RX buffer overrun bit in STATE implicitly.
  uart_irq_rx_overrun_clear_cmsdk_apb (regs);

  /*
  Step 2. Keep storing incoming data into sw buffer until no more data, the sw buffer CAN overrun!

  In the case of cmsdk_apb_uart, the hw rx/tx buffer are both only 1-byte.
  According to qemu/include/hw/char/cmsdk-apb-uart.h
  "This UART has no FIFO, only a 1-character buffer for each of Tx and Rx."
  So we simply read one char.

  At first I did something like this:

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
    }

  In the uart_irq_rx_ready_cmsdk_apb_uart(), I use the RX FULL flag to indicate there's data to read.
  But as I experimented on QEMU, the RX FULL means two chars arriving at the UART to fast.
  So I shound't use it that way. And the UART hw rx buffer is only 1-byte.
  */
  uart_fifo_read_cmsdk_apb_uart (regs, &c, 1);
  uart->rx.buffer[uart->rx.n_head] = c;
  if (rb_full (&uart->rx))
    {
      // overrun happens!
      RB_ADVANCE_POS (uart->rx.n_tail);
    }
  RB_ADVANCE_POS (uart->rx.n_head);
}


/*
 TX interrupt only happens when UART is ready to transmit more data.
 So we undoubtedly CAN transmit 1 byte if the sw buffer has something to tx.

*/
static void Uart_C_Handler_Tx_Instance (uint32_t instance)
{
  UART_CLASS* uart;
  UART_REGS_CMSDK_APB* regs;
  uint8_t c;

  uart = uart_get_instance (instance);
  regs = uart->regs;

  /*
  Step 1. The first thing is to clear the interrupt flag of this time.s
  */
  // Clear the TX interrupt status.
  uart_irq_tx_clear_cmsdk_apb (regs);
  // Clear the TX overrun interrupt status if any.
  // According to DDI0479D, this also clears the TX buffer overrun bit in STATE implicitly.
  uart_irq_tx_overrun_clear_cmsdk_apb (regs);

  // This is the ISR code, no application code can interrupt it. So no need to disable UART interrupt.

  /*
  Step 2.
  TX handler only takes data from ring buffer.
  Buffer empty, no data to transmit.
  */
  if (rb_empty (&uart->tx))
    {
      uart->buffered_tx = 0;
      return;
    }

  /*
  Step 3.
  SW buffer not empty. We got work to do.
  And being here means we CAN send 1 byte.
  Fill the hardware tx buffer until it cannot accept more data or sw ring-buffer is drained.
  */

  // HW has been kicked off to take data from the sw ring buffer.
  // As long as buffered_tx is 1, sw should cotinue write to sw ring buffer and don't need to kick off the transmitting again.
  uart->buffered_tx = 1;
  while (uart_irq_tx_ready_cmsdk_apb_uart (regs))
    {
      // If quit due to tx buffer full, the buffered_tx will remain set.
      c = uart->tx.buffer[uart->tx.n_tail];

      // send out the data
      // this will ensure another TX interrupt to happen
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
