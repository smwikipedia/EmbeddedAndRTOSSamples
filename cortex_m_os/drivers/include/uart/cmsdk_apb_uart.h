/*
 The interface to the cmsdk_apb_uart IP.

 The interface design use the Zephyr RTOS UART interrupt-driven UART API as a reference.
 Refer to below link for tha API semantic.
 https://docs.zephyrproject.org/latest/doxygen/html/group__uart__interrupt.html

 The function semantic comment is adopted from above link.


 The difference is, Zephyr API takes a "UART_REGS_CMSDK_APB* regs" as the parameter.
 Here I use the address of the device register block.

 For example:
 Zephyr: int uart_irq_update (const struct device * dev)
 cm_os: int32_t uart_irq_update_<uart_model_suffix> (const uint32_t instance)

 */

#ifndef CMSDK_APB_UART_H
#define CMSDK_APB_UART_H

#include <stdint.h>
#include <drivers_common.h>
#include <uart/uart_common.h>
/*
 TODO: HAL for cmsdk_apb_uart
*/

#define BAUDDIV_9600 1250

/*
  CTRL register
*/
#define TX_ENABLE_BIT 0
#define RX_ENABLE_BIT 1
#define TX_INT_ENABLE_BIT 2
#define RX_INT_ENABLE_BIT 3
#define TX_OVERRUN_INT_ENABLE_BIT 4
#define RX_OVERRUN_INT_ENABLE_BIT 5

/*
  STATE register
  The bits RX_BUFFER_FULL_FLAG and TX_BUFFER_FULL_FLAG reflect the live state of the hardware.
  While the RX/TX interrupt flag bits only refelct transient events.
  Should use the state bits to track the state and take actions accordingly.
  When action is done, change the interrupt bits accordingly.
*/
#define RX_BUFFER_OVERRUN_FLAG 0x8 // rw
#define TX_BUFFER_OVERRUN_FLAG 0x4 // rw
#define RX_BUFFER_FULL_FLAG 0x2    // ro - automatically, set when there's data, clear when no more data (QEMU behavior)
#define TX_BUFFER_FULL_FLAG 0x1    // ro - automatically, set when no space, clear when there's space (QEMU behavior)

// Interrupt State and Clear register
#define RX_OVERRUN_INT_FLAG 0x8 // rw
#define TX_OVERRUN_INT_FLAG 0x4 // rw
#define RX_INT_FLAG 0x2         // rw
#define TX_INT_FLAG 0x1         // rw

typedef struct
{
  uint32_t DATA;           // lower 8 bits
  uint32_t STATE;          // lower 4 bits
  uint32_t CTRL;           // lower 7 bits
  uint32_t INTSTS_CLR;     // lower 4 bits
  uint32_t BAUDDIV;        // lower 20 bits
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
} UART_REGS_CMSDK_APB;

/*
 APIs for cmsdk_apb_uart
 Not part of the Zephyr RTOS UART interrupt-driven UART API
*/

int32_t uart_init_cmsdk_apb (UART_REGS_CMSDK_APB* regs);
uint8_t uart_rx_data_cmsdk_apb (UART_REGS_CMSDK_APB* regs);
int32_t uart_tx_data_cmsdk_apb (UART_REGS_CMSDK_APB* regs, uint8_t* data, uint32_t count);

/*
  Check if the transfer buffer is overrun.

  Return values:
  0 - not overrun
  1 - overrun
*/
int32_t uart_tx_buffer_overrun_cmsdk_apb (const UART_REGS_CMSDK_APB* regs);

/*
 Zephyr RTOS UART interrupt-driven UART API
*/

typedef void (*uart_irq_callback_user_data_t) (const UART_REGS_CMSDK_APB* regs, void* user_data);

/*
  Fill FIFO with data.

  This function is expected to be called from UART interrupt handler (ISR), if uart_irq_tx_ready() returns true.
  Result of calling this function not from an ISR is undefined (hardware-dependent).
  Likewise, not calling this function from an ISR if uart_irq_tx_ready() returns true may lead to undefined behavior, e.g. infinite interrupt loops.
  It's mandatory to test return value of this function, as different hardware has different FIFO depth (oftentimes just 1).

  Parameters
  dev	UART device instance.
  tx_data	Data to transmit.
  size	Number of bytes to send.

  Returns
  Number of bytes sent.

  Return values
  -ENOSYS	if this function is not supported
  -ENOTSUP	If API is not enabled.
*/
int32_t uart_fifo_fill_cmsdk_apb_uart (UART_REGS_CMSDK_APB* const regs, const uint8_t* tx_data, int32_t size);
int32_t uart_fifo_fill_u16_cmsdk_apb_uart (const UART_REGS_CMSDK_APB* regs, const uint16_t* tx_data, int32_t size);


/*
  Read data from FIFO.

  This function is expected to be called from UART interrupt handler (ISR), if uart_irq_rx_ready() returns true.
  Result of calling this function not from an ISR is undefined (hardware-dependent).
  It's unspecified whether "RX ready" condition as returned by uart_irq_rx_ready() is level- or edge- triggered.
  That means that once uart_irq_rx_ready() is detected, uart_fifo_read() must be called until it reads all available data in the FIFO (i.e. until it returns less data than was requested).

  Parameters
  rx_data	Data container.
  size	Container size.

  Returns
  Number of bytes read.

  Return values
  -ENOSYS	If this function is not implemented.
  -ENOTSUP	If API is not enabled.
*/
int32_t uart_fifo_read_cmsdk_apb_uart (const UART_REGS_CMSDK_APB* regs, uint8_t* rx_data, const int32_t size);
int32_t uart_fifo_read_u16_cmsdk_apb_uart (const UART_REGS_CMSDK_APB* regs, uint16_t* rx_data, const int32_t size);
void uart_irq_tx_enable_cmsdk_apb_uart (const UART_REGS_CMSDK_APB* regs);
void uart_irq_tx_disable_cmsdk_apb_uart (const UART_REGS_CMSDK_APB* regs);

/*
  Check if UART TX buffer can accept bytes.

  Check if UART TX buffer can accept more bytes for transmission (i.e. uart_fifo_fill() will succeed and return non-zero).
  This function must be called in a UART interrupt handler, or its result is undefined.
  Before calling this function in the interrupt handler, uart_irq_update() must be called once per the handler invocation.

  Return values
  0           If device is not ready to write a new byte.
  >0          Minimum number of bytes that can be written in a single call to uart_fifo_fill.
              It may be possible to write more bytes, but the actual number written must be checked in the return code from uart_fifo_fill.
  -ENOSYS     If this function is not implemented.
  -ENOTSUP    If API is not enabled.
*/
int32_t uart_irq_tx_ready_cmsdk_apb_uart (const UART_REGS_CMSDK_APB* regs);

/*
 Enable RX interrupt.
*/
void uart_irq_rx_enable_cmsdk_apb_uart (UART_REGS_CMSDK_APB* const regs);

/*
 Disable RX interrupt.
*/
void uart_irq_rx_disable_cmsdk_apb_uart (UART_REGS_CMSDK_APB* const regs);

/*
Check if UART TX block finished transmission.

Check if any outgoing data buffered in UART TX block was fully transmitted and TX block is idle.
When this condition is true, UART device (or whole system) can be power off.
Note that this function is not useful to check if UART TX can accept more data, use uart_irq_tx_ready() for that.
This function must be called in a UART interrupt handler, or its result is undefined.
Before calling this function in the interrupt handler, uart_irq_update() must be called once per the handler invocation.
*/
int32_t uart_irq_tx_complete_cmsdk_apb_uart (const UART_REGS_CMSDK_APB* regs);


/*
Check if UART RX buffer has a received char.

Check if UART RX buffer has at least one pending character (i.e. uart_fifo_read() will succeed and return non-zero).
This function must be called in a UART interrupt handler, or its result is undefined.
Before calling this function in the interrupt handler, uart_irq_update() must be called once per the handler invocation.
It's unspecified whether condition as returned by this function is level- or edge- triggered (i.e. if this function returns true when RX FIFO is non-empty,
or when a new char was received since last call to it). See description of uart_fifo_read() for implication of this.

Return values
  1 If a received char is ready.
  0 If a received char is not ready.
  -ENOSYS If this function is not implemented.
  -ENOTSUP If API is not enabled.
*/
int32_t uart_irq_rx_ready_cmsdk_apb_uart (const UART_REGS_CMSDK_APB* regs);


void uart_irq_err_enable_cmsdk_apb_uart (const UART_REGS_CMSDK_APB* regs);
void uart_irq_err_disable_cmsdk_apb_uart (const UART_REGS_CMSDK_APB* regs);
int32_t uart_irq_is_pending_cmsdk_apb_uart (const UART_REGS_CMSDK_APB* regs);

/*
Start processing interrupts in ISR.

This function should be called the first thing in the ISR.
Calling uart_irq_rx_ready(), uart_irq_tx_ready(), uart_irq_tx_complete() allowed only after this.

The purpose of this function is:

- For devices with auto-acknowledge of interrupt status on register read to cache the value of this register (rx_ready, etc. then use this case).

- For devices with explicit acknowledgment of interrupts, to ack any pending interrupts and likewise to cache the original value.

- For devices with implicit acknowledgment, this function will be empty.
  But the ISR must perform the actions needs to ack the interrupts (usually, call uart_fifo_read() on rx_ready, and uart_fifo_fill() on tx_ready).

Return values
  1	On success.
  -ENOSYS	If this function is not implemented.
  -ENOTSUP	If API is not enabled.
*/
int32_t uart_irq_update_cmsdk_apb_uart (const UART_REGS_CMSDK_APB* regs);

int32_t uart_irq_callback_user_data_set_cmsdk_apb_uart (const UART_REGS_CMSDK_APB* regs, uart_irq_callback_user_data_t cb, void* user_data);
int32_t uart_irq_callback_set_cmsdk_apb_uart (const UART_REGS_CMSDK_APB* regs, uart_irq_callback_user_data_t cb);


#endif
