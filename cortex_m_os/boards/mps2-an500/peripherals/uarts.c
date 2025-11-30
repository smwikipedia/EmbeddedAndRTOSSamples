#include <stdint.h>
#include <board_api.h>
#include <mps2-an500.h>
#include <uart/cmsdk_apb_uart.h>

/*
Convert the UART instance number to its reg block address.
*/
static void* uart_get_reg_block (uint32_t instance)
{
  switch (instance)
    {
    case 0: return (void*)UART_0_BASE;
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
