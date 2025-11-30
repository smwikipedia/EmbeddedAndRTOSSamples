#include <stdint.h>
#include <ast1030-evb.h>
#include <board_api.h>
#include <uart/ns16550.h>

/*
Convert the UART instance number to its reg block address.
*/
static void* uart_get_reg_block (uint32_t instance)
{
  return 0;
}


uint32_t board_uart_get_total_instance_number (void)
{
  return AST1030_EVB_UART_MAX_NUM;
}


int32_t board_uart_init (uint32_t instance)
{
  void* uart_reg_block = uart_get_reg_block (instance);
  uart_init_ns16550 (uart_reg_block);
  return 0;
}
