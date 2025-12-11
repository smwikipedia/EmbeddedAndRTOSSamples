#ifndef UART_COMMON_H
#define UART_COMMON_H

#include "ring_buffer.h"
#include <stdint.h>
#include <data_structures.h>


typedef int32_t (*process_t) (RING_BUFFER* buffer, uint8_t c);

typedef struct _UART_CLASS {
  void* regs;

  // Indicating if hw is transmitting data from the buffer
  // Only hardware driver can set/clear it. Upper layer read it.
  uint32_t buffered_tx;

  /*
  These buffers are the communication channel shared between the UART user and the UART driver.
  Access to such shared resource must be exclusive.
  */
  RING_BUFFER rx;
  RING_BUFFER tx;


  /*
  Add data to ring buffer and process the ring buffer.
  Such as line discipline.
  */
  process_t fn_process;

} UART_CLASS;

#endif
