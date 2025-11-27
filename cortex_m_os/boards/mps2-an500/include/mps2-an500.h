/*
 Defines the board-specific details.
 Such as address map, interrupt arrangement, etc.
*/
#ifndef MPS2_AN500_H
#define MPS2_AN500_H

#define CMSDK_APB_SUBSYS_BASE 0x40000000UL

/*
 UARTs
*/

#define MPS2_AN550_UART_MAX_NUM 5

#define UART_0 0
#define UART_0_RX_INT 0
#define UART_0_TX_INT 1
#define UART_0_BASE 0x40004000UL


#define UART_1 1
#define UART_1_RX_INT 2
#define UART_1_TX_INT 3

#define UART_2 2
#define UART_2_RX_INT 4
#define UART_2_TX_INT 5

#define UART_3 3
#define UART_3_RX_INT 18
#define UART_3_TX_INT 19

#define UART_4 4
#define UART_4_RX_INT 20
#define UART_4_TX_INT 21


#endif
