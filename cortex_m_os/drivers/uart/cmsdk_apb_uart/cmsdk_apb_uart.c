/*
QEMU source for this UART model:
qemu/include/hw/char/cmsdk-apb-uart.h
qemu/hw/char/cmsdk-apb-uart.c

According to QEMU source: qemu/include/hw/char/cmsdk-apb-uart.h
This UART has no FIFO, only a 1-character buffer for each of Tx and Rx.



*/

#include "drivers_common.h"
#include <error_no.h>
#include <stdint.h>
#include <uart/cmsdk_apb_uart.h>
#include <utils.h>

int32_t uart_init_cmsdk_apb (UART_REGS_CMSDK_APB* regs)
{
  /*
  According to: DDI0479D_m_class_processor_system_r1p1_trm.pdf
  9600 -> 1250
  You must program the baud rate divider register before enabling the UART.
  */
  regs->BAUDDIV = BAUDDIV_9600;

  /*
  0 - TX enable
  1 - RX enable
  2 - TX interrupt enable
  3 - RX interrupt enable
  4 - TX buffer overrun enable
  5 - RX buffer overrun enable
  */
  regs->CTRL = (BIT (TX_ENABLE_BIT) |
  BIT (RX_ENABLE_BIT) |
  BIT (TX_INT_ENABLE_BIT) |
  BIT (RX_INT_ENABLE_BIT) |
  BIT (TX_OVERRUN_INT_ENABLE_BIT) |
  BIT (RX_OVERRUN_INT_ENABLE_BIT));

  return 0;
}

int32_t uart_tx_buffer_overrun_cmsdk_apb (const UART_REGS_CMSDK_APB* regs)
{
  if (regs->STATE & BIT (TX_BUFFER_OVERRUN_BIT))
    {
      return 1;
    }

  return 0;
}

int32_t uart_rx_buffer_overrun_cmsdk_apb (const UART_REGS_CMSDK_APB* regs)
{
  if (regs->STATE & BIT (RX_BUFFER_OVERRUN_BIT))
    {
      return 1;
    }

  return 0;
}

void uart_tx_buffer_overrun_clear_cmsdk_apb (UART_REGS_CMSDK_APB* const regs)
{
  // Write 1 to clear the TX overrun STATE.
  regs->STATE = (BIT (TX_BUFFER_OVERRUN_BIT) & STATE_MASK);
}

void uart_rx_buffer_overrun_clear_cmsdk_apb (UART_REGS_CMSDK_APB* const regs)
{
  // Write 1 to clear the RX overrun STATE.
  regs->STATE = (BIT (RX_BUFFER_OVERRUN_BIT) & STATE_MASK);
}

void uart_irq_tx_clear_cmsdk_apb (UART_REGS_CMSDK_APB* const regs)
{
  // Write 1 to clear the TX interrupt status.
  regs->INTSTS_CLR = (BIT (TX_INT_BIT) & INT_STATUS_CLEAR_MASK);
}

void uart_irq_rx_clear_cmsdk_apb (UART_REGS_CMSDK_APB* const regs)
{
  // Write 1 to clear the RX interrupt status.
  regs->INTSTS_CLR = (BIT (RX_INT_BIT) & INT_STATUS_CLEAR_MASK);
}

void uart_irq_tx_overrun_clear_cmsdk_apb (UART_REGS_CMSDK_APB* const regs)
{
  // Write 1 to clear the TX overrun interrupt status.
  regs->INTSTS_CLR = (BIT (TX_OVERRUN_INT_BIT) & INT_STATUS_CLEAR_MASK);
}

void uart_irq_rx_overrun_clear_cmsdk_apb (UART_REGS_CMSDK_APB* const regs)
{
  // Write 1 to clear the RX overrun interrupt status.
  regs->INTSTS_CLR = (BIT (RX_OVERRUN_INT_BIT) & INT_STATUS_CLEAR_MASK);
}

void uart_tx_enable_cmsdk_apb_uart (UART_REGS_CMSDK_APB* const regs)
{
  regs->CTRL |= (BIT (TX_ENABLE_BIT) & CTRL_MASK);
}

void uart_tx_disable_cmsdk_apb_uart (UART_REGS_CMSDK_APB* const regs)
{
  regs->CTRL &= (BIT_NOT (TX_ENABLE_BIT) & CTRL_MASK);

  // this function disable tx function, not tx interrupt,
  // clear any pending tx interrut
  regs->INTSTS_CLR = BIT (TX_INT_BIT);
}

void uart_rx_enable_cmsdk_apb_uart (UART_REGS_CMSDK_APB* const regs)
{
  regs->CTRL |= (BIT (RX_ENABLE_BIT) & CTRL_MASK);
}

void uart_rx_disable_cmsdk_apb_uart (UART_REGS_CMSDK_APB* const regs)
{
  regs->CTRL &= (BIT_NOT (RX_ENABLE_BIT) & CTRL_MASK);

  // this function disable rx function, not rx interrupt,
  // also clear any pending rx interrut
  regs->INTSTS_CLR = BIT (RX_INT_BIT);
}


int32_t uart_rx_buffer_full_cmsdk_apb_uart (UART_REGS_CMSDK_APB* const regs)
{
  if (regs->STATE & BIT (RX_BUFFER_FULL_BIT))
    {
      return 1;
    }
  else
    {
      return 0;
    }
}

int32_t uart_tx_buffer_full_cmsdk_apb_uart (UART_REGS_CMSDK_APB* const regs)
{
  if (regs->STATE & BIT (TX_BUFFER_FULL_BIT))
    {
      return 1;
    }
  else
    {
      return 0;
    }
}


/*
Zephyr RTOS UART interrupt-driven APIs
*/

int32_t uart_fifo_fill_cmsdk_apb_uart (UART_REGS_CMSDK_APB* const regs, const uint8_t* tx_data, int32_t size)
{
  uint32_t sent_count;

  sent_count = 0;
  while (uart_irq_tx_ready_cmsdk_apb_uart (regs) && size > 0)
    {
      regs->DATA = tx_data[sent_count];
      size--;
      sent_count++;
    }

  return sent_count;
}


int32_t uart_fifo_read_cmsdk_apb_uart (const UART_REGS_CMSDK_APB* regs, uint8_t* rx_data, const int32_t size)
{
  /*
  cmsdk_apb_uart only has 1-byte hw rx buffer.
  */
  if (size < 1)
    {
      return 0;
    }
  rx_data[0] = regs->DATA;
  return 1;
}


/*
Enable TX interrupt
*/
void uart_irq_tx_enable_cmsdk_apb_uart (UART_REGS_CMSDK_APB* const regs)
{
  // regs->CTRL |= BIT (TX_INT_ENABLE_BIT | TX_ENABLE_BIT);
  regs->CTRL |= (BIT (TX_INT_ENABLE_BIT) & CTRL_MASK);
}

/*
Disable TX interrupt
*/
void uart_irq_tx_disable_cmsdk_apb_uart (UART_REGS_CMSDK_APB* const regs)
{
  // regs->CTRL &= BIT_NOT (TX_INT_ENABLE_BIT | TX_ENABLE_BIT);
  regs->CTRL &= (BIT_NOT (TX_INT_ENABLE_BIT) & CTRL_MASK);

  // clear any pending tx interrut
  regs->INTSTS_CLR = BIT (TX_INT_BIT);
}


int32_t uart_irq_update_cmsdk_apb_uart (const UART_REGS_CMSDK_APB* regs)
{
  // Seems cmsdk_apb_uart has implicit acknowledgment.
  // So nothing to do.
  return 1;
}

int32_t uart_irq_rx_ready_cmsdk_apb_uart (const UART_REGS_CMSDK_APB* regs)
{
  /*
  cmsdk_apb_uart tx buffer and rx buffer are both 1-byte.
  The rx full bit in the STATE regsiter mean data arriving too fast.
  They cannot be used to determine if rx/tx are ready.
  We can only use the RX flag in the INT_STS_CLR register.
  But this only works when the RX status are not cleared.

  So I did this.
  return !!(regs->INTSTS_CLR & BIT (RX_INT_BIT));

  But Zephyr RTOS is using the RX FULL to indicate rx ready.
  Ref: zephyr/drivers/serial/uart_cmsdk_apb.c, uart_cmsdk_apb_irq_rx_ready().

  I am following zephyr's practice.
  */

  return !!(regs->STATE & BIT (RX_BUFFER_FULL_BIT));
}


int32_t uart_irq_is_pending_cmsdk_apb_uart (const UART_REGS_CMSDK_APB* regs)
{
  return (uart_irq_rx_ready_cmsdk_apb_uart (regs) || uart_irq_tx_ready_cmsdk_apb_uart (regs));
}


int32_t uart_irq_tx_ready_cmsdk_apb_uart (const UART_REGS_CMSDK_APB* regs)
{
  /*
  cmsdk_apb_uart tx buffer and rx buffer are both 1-byte.
  The tx full bit in the STATE regsiter mean data transmitting too fast.
  */
  return !(regs->STATE & BIT (TX_BUFFER_FULL_BIT));
}

int32_t uart_irq_tx_complete_cmsdk_apb_uart (const UART_REGS_CMSDK_APB* regs)
{
  // Unfortunately, from the cmsdk_apb_uart STATE register, I cannot tell when the TX buffer is empty.
  // I may use TX buffer ready as a best effort.
  // And Zephyr RTOS also does this.
  // See: zephyr/drivers/serial/uart_cmsdk_apb.c, uart_cmsdk_apb_irq_tx_complete()

  return uart_irq_tx_ready_cmsdk_apb_uart (regs);
}

/*
Enable RX interrupt.
*/
void uart_irq_rx_enable_cmsdk_apb_uart (UART_REGS_CMSDK_APB* const regs)
{
  // regs->CTRL |= BIT (RX_INT_ENABLE_BIT | RX_ENABLE_BIT);
  regs->CTRL |= (BIT (RX_INT_ENABLE_BIT) & CTRL_MASK);
}

/*
Disable RX interrupt.
*/
void uart_irq_rx_disable_cmsdk_apb_uart (UART_REGS_CMSDK_APB* const regs)
{
  // regs->CTRL &= BIT_NOT (RX_INT_ENABLE_BIT | RX_ENABLE_BIT);
  regs->CTRL &= (BIT_NOT (RX_INT_ENABLE_BIT) & CTRL_MASK);

  // clear any pending rx interrut
  regs->INTSTS_CLR = BIT (RX_INT_BIT);
}
