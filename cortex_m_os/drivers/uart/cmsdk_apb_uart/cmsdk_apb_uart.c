#include <stdint.h>
#include <uart/cmsdk_apb_uart.h>

int32_t uart_init_cmsdk_apb (void* regs)
{
  CMSDK_APB_UART_TYPE* dev = (CMSDK_APB_UART_TYPE*)regs;

  /*
  According to: DDI0479D_m_class_processor_system_r1p1_trm.pdf
  You must program the baud rate divider register before enabling the UART.
  */
  dev->BAUDDIV = 1250;

  /*
  0 - TX enable
  1 - RX enable
  2 - TX interrupt enable
  3 - RX interrupt enable
  */
  dev->CTRL = 0xF;

  return 0;
}

uint8_t uart_rx_data_cmsdk_apb (void* regs)
{
  CMSDK_APB_UART_TYPE* dev = (CMSDK_APB_UART_TYPE*)regs;
  uint8_t c                = dev->DATA & 0xFF;
  return c;
}

int32_t uart_tx_data_cmsdk_apb (void* regs, uint8_t* data, uint32_t count)
{
  uint32_t sent_count;

  sent_count               = 0;
  CMSDK_APB_UART_TYPE* dev = (CMSDK_APB_UART_TYPE*)regs;
  while (count > 0)
    {
      dev->DATA = data[sent_count];
      count--;
      sent_count++;
    }

  return sent_count;
}
