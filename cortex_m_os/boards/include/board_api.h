/*
This is the board level HAL for OS.
Every board should provide this generic interface to OS.
i.e. This is what ANY board looks like in the eyes of OS.
*/

#ifndef BOARD_API_H
#define BOARD_API_H

#include <stdint.h>

// Utilities
#define DEAD_LOOP \
  do              \
    {             \
      ;           \
    }             \
  while (1)


// UART

/*
Get the total number of available UART instances.
*/
uint32_t board_uart_get_total_instance_number (void);

/*
Init a UART instance.

If a board has more than one models of UART, which is possible,
this API should tell between them through the instance parameter.
And call the correct model-specific API to init it.

*/
int32_t board_uart_init (uint32_t instance);


int32_t board_uart_tx_data (uint32_t instance, uint8_t* data, uint32_t count);


#endif
