#include <stdint.h>
#include <board_api.h>
#include <netduinoplus2.h>
#include <uart/stm32f405_uart.h>

/*
Convert the UART instance number to its reg block address.
*/
static void* get_uart_reg_block (uint32_t instance)
{
    return 0;
}


uint32_t board_get_uart_instance_total_number (void)
{
    return NETDUINOPLUS2_UART_MAX_NUM;
}


int32_t board_uart_init (uint32_t instance)
{
    void* uart_reg_block = get_uart_reg_block (instance);
    return 0;
}
