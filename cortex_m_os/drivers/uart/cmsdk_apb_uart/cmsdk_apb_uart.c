#include <stdint.h>
#include <uart/cmsdk_apb_uart.h>

int32_t uart_init (void* regs)
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
