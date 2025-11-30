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

int32_t board_uart_tx_data (uint32_t instance, uint8_t* data, uint32_t count)
{
  // int32_t sent_count;

  // CMSDK_APB_UART_TYPE *regs = uart_get_reg_block(instance);
  // sent_count = uart_tx_data_cmsdk_apb(regs, data, count);
  // return sent_count;
  return 0;
}


void Uart_C_Handler_Rx (void)
{
  uint8_t c;
  CMSDK_APB_UART_TYPE* regs = uart_get_reg_block (0);
  uint8_t CR = '\r';
  uint8_t LF = '\n';

  // keep reading until not full
  while ((regs->STATE & RX_BUFFER_FULL) || (regs->STATE & RX_BUFFER_OVERRUN))
    {
      c = uart_rx_data_cmsdk_apb (regs);
      if (c == CR) {
        uart_tx_data_cmsdk_apb (regs, &LF, 1);
      }
      if (c != LF) {
        uart_tx_data_cmsdk_apb (regs, &c, 1);
      }
    }
  regs->STATE = RX_BUFFER_OVERRUN; // Write 1 to clear the RX buffer overrun state.
  regs->INTSTS_CLR = RX_INT_FLAG | RX_OVERRUN_INT_FLAG;
  // while (1)
  //   ;
}

// void Uart_C_Handler_Tx (void) __attribute__ ((noreturn));

/*
TX interrupt happens when all buffered data has been sent out.
*/
void Uart_C_Handler_Tx (void)
{
  CMSDK_APB_UART_TYPE* regs = uart_get_reg_block (0);
  regs->STATE = TX_BUFFER_OVERRUN; // Write 1 to clear the TX buffer overrun state.
  regs->INTSTS_CLR = TX_INT_FLAG | TX_OVERRUN_INT_FLAG;
  // while (1)
  //   ;
}
