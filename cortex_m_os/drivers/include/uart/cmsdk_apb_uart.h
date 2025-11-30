/*
 The interface to the cmsdk_apb_uart IP.
*/

#ifndef CMSDK_APB_UART_H
#define CMSDK_APB_UART_H

#include <stdint.h>

/*
 TODO: HAL for cmsdk_apb_uart
*/

#define RX_BUFFER_OVERRUN 0x8
#define TX_BUFFER_OVERRUN 0x4
#define RX_BUFFER_FULL 0x2
#define TX_BUFFER_FULL 0x1

#define RX_OVERRUN_INT_FLAG 0x8
#define TX_OVERRUN_INT_FLAG 0x4
#define RX_INT_FLAG 0x2
#define TX_INT_FLAG 0x1

typedef struct
{
  uint32_t DATA;           // lower 8 bits
  uint32_t STATE;          // lower 4 bits
  uint32_t CTRL;           // lower 7 bits
  uint32_t INTSTS_CLR;     // lower 4 bits
  uint32_t BAUDDIV;        //  lower 20 bits
  uint8_t unused_5[0xFBC]; // 0x14 ~ 0xFCF is not used.
  uint32_t PID4;           // lower 8 bits
  uint32_t PID5;           // lower 8 bits
  uint32_t PID6;           // lower 8 bits
  uint32_t PID7;           // lower 8 bits
  uint32_t PID0;           // lower 8 bits
  uint32_t PID1;           // lower 8 bits
  uint32_t PID2;           // lower 8 bits
  uint32_t PID3;           // lower 8 bits
  uint32_t CID0;           // lower 8 bits
  uint32_t CID1;           // lower 8 bits
  uint32_t CID2;           // lower 8 bits
  uint32_t CID3;           // lower 8 bits
} CMSDK_APB_UART_TYPE;


/*
 APIs for cmsdk_apb_uart
*/

int32_t uart_init_cmsdk_apb (void* regs);
uint8_t uart_rx_data_cmsdk_apb (void* regs);
int32_t uart_tx_data_cmsdk_apb (void* regs, uint8_t* data, uint32_t count);


#endif
