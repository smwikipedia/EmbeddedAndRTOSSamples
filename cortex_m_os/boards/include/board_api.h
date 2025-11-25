/*
This is the board level HAL for OS.
Every board should provide this generic interface to OS.
i.e. This is what ANY board looks like in the eyes of OS.
*/

#ifndef BOARD_API_H
#define BOARD_API_H

#include <stdint.h>


// UART

/*
Get the total number of available UART instances.
*/
uint32_t board_get_uart_instance_total_number (void);

/*
Init a UART instance.
*/
int32_t board_uart_init (uint32_t instance);


#endif
