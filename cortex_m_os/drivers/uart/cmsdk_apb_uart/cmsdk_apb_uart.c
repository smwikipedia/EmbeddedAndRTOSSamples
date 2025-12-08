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

// uint8_t uart_rx_data_cmsdk_apb (UART_REGS_CMSDK_APB* regs)
// {
//   uint8_t c = regs->DATA & 0xFF;
//   return c;
// }

// int32_t uart_tx_data_cmsdk_apb (UART_REGS_CMSDK_APB* regs, uint8_t* data, uint32_t count)
// {
//   uint32_t sent_count;

//   sent_count = 0;

//   while (count > 0)
//     {
//       regs->DATA = data[sent_count];
//       count--;
//       sent_count++;
//     }

//   return sent_count;
// }

int32_t uart_tx_buffer_overrun_cmsdk_apb (const UART_REGS_CMSDK_APB* regs)
{
  if (regs->STATE & TX_BUFFER_OVERRUN_FLAG)
    {
      return 1;
    }

  return 0;
}

int32_t uart_rx_buffer_overrun_cmsdk_apb (const UART_REGS_CMSDK_APB* regs)
{
  if (regs->STATE & RX_BUFFER_OVERRUN_FLAG)
    {
      return 1;
    }

  return 0;
}

void uart_tx_buffer_overrun_clear_cmsdk_apb (UART_REGS_CMSDK_APB* const regs)
{
  // Write 1 to clear the TX overrun STATE.
  regs->STATE = TX_BUFFER_OVERRUN_FLAG;
}

void uart_rx_buffer_overrun_clear_cmsdk_apb (UART_REGS_CMSDK_APB* const regs)
{
  // Write 1 to clear the RX overrun STATE.
  regs->STATE = RX_BUFFER_OVERRUN_FLAG;
}

void uart_irq_tx_clear_cmsdk_apb (UART_REGS_CMSDK_APB* const regs)
{
  // Write 1 to clear the TX interrupt status.
  regs->INTSTS_CLR = TX_INT_FLAG;
}

void uart_irq_rx_clear_cmsdk_apb (UART_REGS_CMSDK_APB* const regs)
{
  // Write 1 to clear the RX interrupt status.
  regs->INTSTS_CLR = RX_INT_FLAG;
}

void uart_irq_tx_overrun_clear_cmsdk_apb (UART_REGS_CMSDK_APB* const regs)
{
  // Write 1 to clear the TX overrun interrupt status.
  regs->INTSTS_CLR = TX_OVERRUN_INT_FLAG;
}

void uart_irq_rx_overrun_clear_cmsdk_apb (UART_REGS_CMSDK_APB* const regs)
{
  // Write 1 to clear the RX overrun interrupt status.
  regs->INTSTS_CLR = RX_OVERRUN_INT_FLAG;
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
  uint32_t read_count;

  read_count = 0;
  while (uart_irq_rx_ready_cmsdk_apb_uart (regs) && read_count <= size)
    {
      rx_data[read_count] = regs->DATA;
      read_count++;
    }

  return read_count;
}


/*
Enable TX interrupt
*/
void uart_irq_tx_enable_cmsdk_apb_uart (UART_REGS_CMSDK_APB* const regs)
{
  uint8_t value = regs->CTRL;

  value |= BIT (TX_INT_ENABLE_BIT);
  regs->CTRL = value;
}

/*
Disable TX interrupt
*/
void uart_irq_tx_disable_cmsdk_apb_uart (UART_REGS_CMSDK_APB* const regs)
{
  uint8_t value = regs->CTRL;

  value &= BIT_NOT (TX_INT_ENABLE_BIT);
  regs->CTRL = value;
}


int32_t uart_irq_update_cmsdk_apb_uart (const UART_REGS_CMSDK_APB* regs)
{
  // Seems cmsdk_apb_uart has implicit acknowledgment.
  // So nothing to do.
  return 1;
}

int32_t uart_irq_rx_ready_cmsdk_apb_uart (const UART_REGS_CMSDK_APB* regs)
{
  if (regs->STATE & RX_BUFFER_FULL_FLAG)
    {
      return 1;
    }
  else
    {
      return 0;
    }
}

int32_t uart_irq_tx_ready_cmsdk_apb_uart (const UART_REGS_CMSDK_APB* regs)
{
  if (regs->STATE & TX_BUFFER_FULL_FLAG)
    {
      return 0;
    }
  else
    {
      return 1;
    }
}

int32_t uart_irq_tx_complete_cmsdk_apb_uart (const UART_REGS_CMSDK_APB* regs)
{
  // Unfortunately, from the cmsdk_apb_uart STATE register, I cannot tell when the TX buffer is empty.
  // I may use TX buffer ready as a best effort.\
  // But I choose to return -ENOTSUP.

  // return uart_irq_tx_ready_cmsdk_apb_uart(regs);
  return -ENOTSUP;
}

/*
Enable RX interrupt.
*/
void uart_irq_rx_enable_cmsdk_apb_uart (UART_REGS_CMSDK_APB* const regs)
{
  // Just disable rx interrupt, do not enable rx implicitly!
  // Stick to the semantic!
  uint8_t value = regs->CTRL;

  value |= BIT (RX_INT_ENABLE_BIT);
  regs->CTRL = value;
}

/*
Disable RX interrupt.
*/
void uart_irq_rx_disable_cmsdk_apb_uart (UART_REGS_CMSDK_APB* const regs)
{
  // Just disable rx interrupt, do not disable rx implicitly!
  // Stick to the semantic!
  uint8_t value;

  value = regs->CTRL;
  value &= BIT_NOT (RX_INT_ENABLE_BIT);
  regs->CTRL = value;
}
