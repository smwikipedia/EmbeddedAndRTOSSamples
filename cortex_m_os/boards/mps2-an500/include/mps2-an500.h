/*
 Defines the board-specific details.
 Such as address map, interrupt arrangement, etc.
*/
#ifndef MPS2_AN500_H
#define MPS2_AN500_H

#include <utils.h>

#define CMSDK_APB_SUBSYS_BASE 0x40000000UL

/*
 UARTs
 Refer to DAI0500B $5 for interrupt arrangment on mps2-an500 board.
*/

#define MPS2_AN550_UART_MAX_NUM 5

#define UART_0 0
#define UART_0_RX_IRQN 0
#define UART_0_TX_IRQN 1
#define UART_0_BASE 0x40004000UL
#define UART_0_RX_INT_ACTIVE_BIT BIT0
#define UART_0_TX_INT_ACTIVE_BIT BIT1

#define UART_1 1
#define UART_1_RX_IRQN 2
#define UART_1_TX_IRQN 3
#define UART_1_BASE 0x40005000UL
#define UART_1_RX_INT_ACTIVE_BIT BIT2
#define UART_1_TX_INT_ACTIVE_BIT BIT3

#define UART_2 2
#define UART_2_RX_IRQN 4
#define UART_2_TX_IRQN 5
#define UART_2_BASE 0x40006000UL
#define UART_2_RX_INT_ACTIVE_BIT BIT4
#define UART_2_TX_INT_ACTIVE_BIT BIT5

/*
There's no UART overflow interrupt for UART 3/4
Ref: DAI0500B $5 Interrupt Assignments
*/
#define UART_0_1_2_OVERRUN_IRQN 12

#define UART_3 3
#define UART_3_RX_IRQN 18
#define UART_3_TX_IRQN 19
#define UART_3_BASE 0x40007000UL
#define UART_3_RX_INT_ACTIVE_BIT BIT18
#define UART_3_TX_INT_ACTIVE_BIT BIT19

#define UART_4 4
#define UART_4_RX_IRQN 20
#define UART_4_TX_IRQN 21
#define UART_4_BASE 0x40009000UL
#define UART_4_RX_INT_ACTIVE_BIT BIT20
#define UART_4_TX_INT_ACTIVE_BIT BIT21


#define NVIC_PRIORITY_UART 4
#define NVIC_PRIORITY_UART_0 NVIC_PRIORITY_UART
#define NVIC_PRIORITY_UART_1 NVIC_PRIORITY_UART
#define NVIC_PRIORITY_UART_2 NVIC_PRIORITY_UART
#define NVIC_PRIORITY_UART_3 NVIC_PRIORITY_UART
#define NVIC_PRIORITY_UART_4 NVIC_PRIORITY_UART


#endif
