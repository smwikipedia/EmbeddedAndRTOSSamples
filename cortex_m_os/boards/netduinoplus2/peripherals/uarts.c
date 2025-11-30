#include <stdint.h>
#include <board_api.h>
#include <netduinoplus2.h>
#include <uart/stm32f405_uart.h>

/*
Convert the UART instance number to its reg block address.
*/
static void* uart_get_reg_block (uint32_t instance)
{
  return 0;
}


uint32_t board_uart_get_total_instance_number (void)
{
  return NETDUINOPLUS2_UART_MAX_NUM;
}


int32_t board_uart_init (uint32_t instance)
{
  void* uart_reg_block = uart_get_reg_block (instance);
  uart_init_stm32f405 (uart_reg_block);
  return 0;
}
