#ifndef UART_COMMON_H
#define UART_COMMON_H

#include <stdint.h>

#define UART_BUFFER_LEN 32

typedef struct
{
  void* regs;

  /*
  These buffers are the communication channel between the UART user and the UART driver.
  */
  uint8_t rx_buffer[UART_BUFFER_LEN];
  uint8_t tx_buffer[UART_BUFFER_LEN];

  /*
    rx_head points to the next space to hold the data received.
    rx_tail points to the first data to return.
    rx_head is ahead of rx_tail.
  */
  uint8_t rx_head, rx_tail;
  /*
    tx_head points to the next space to hold the data to send.
    tx_tail points to the first data to send.
    tx_head is ahead of tx_tail.
  */
  uint8_t tx_head, tx_tail;


} UART_CLASS;

#endif
