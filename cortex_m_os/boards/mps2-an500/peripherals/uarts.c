#include <stdint.h>
#include <board_api.h>
#include <mps2-an500.h>
#include <uart/cmsdk_apb_uart.h>

/*
Convert the UART instance number to its reg block address.
*/
static void* get_uart_reg_block (uint32_t instance)
{
    switch (instance)
        {
        case 0: return (void*)UART_0_BASE;
        default: DEAD_LOOP;
        }
    return 0;
}


uint32_t board_get_uart_instance_total_number (void)
{
    return MPS2_AN550_UART_MAX_NUM;
}


int32_t board_uart_init (uint32_t instance)
{
    void* uart_regs = get_uart_reg_block (instance);
    uart_init (uart_regs);
    return 0;
}
