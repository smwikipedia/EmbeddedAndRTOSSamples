#include <stdint.h>
#include <board_api.h>
#include <mps2-an500.h>
#include <uart/cmsdk_apb_uart.h>

/*
Convert the UART instance number to its reg block address.
*/
static void* get_uart_reg_block (uint32_t instance)
{
    return 0;
}


uint32_t board_get_uart_instance_total_number (void)
{
    return MPS2_AN550_UART_MAX_NUM;
}


int32_t board_uart_init (uint32_t instance)
{
    void* uart_reg_block = get_uart_reg_block (instance);
    return 0;
}
