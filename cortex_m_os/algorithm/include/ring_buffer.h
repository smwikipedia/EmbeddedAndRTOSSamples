#ifndef ALGO_RING_BUFFER_H
#define ALGO_RING_BUFFER_H

#include <stdint.h>
#include <dlist.h>

// #define RING_BUFFER(name, len)                                 \
//   typedef struct                                               \
//   {                                                            \
//     uint8_t buffer[len];                                       \
//     /*                                                         \
//       head points to the next space to hold the data received. \
//       tail points to the first data to return.                 \
//       head is ahead of tail.                                   \
//     */                                                         \
//     uint32_t head, tail;                                       \
//   } RING_BUFFER_##name;

#define RING_BUFFER_LENGTH 50
#define RB_ADVANCE_POS(x) (x = (x + 1) % RING_BUFFER_LENGTH)


typedef int32_t (*send_next_t) (LIST_ENTRY* next);
typedef int32_t (*recv_prev_t) (LIST_ENTRY* prev);

extern int32_t send_next (LIST_ENTRY* next);
extern int32_t recv_prev (LIST_ENTRY* prev);

typedef struct _RING_BUFFER RING_BUFFER;
typedef struct _RING_BUFFER
{
  uint8_t buffer[RING_BUFFER_LENGTH];

  /*
  Pointers to chain multiple ring buffers together as a dlist.
  For future.
  */
  LIST_ENTRY buffer_chain_link;

  /*
  Functions to move data along the buffer chain and do necessary processing.
  */
  send_next_t fn_send_next;
  recv_prev_t fn_recv_prev;

  /*
  House-keeping data for ring-buffer.

  head = write position.
  tail = read position.

  Reserve one 1 space.
  Empty: head == tail
  Full: (head+1)%N == tail.  (N is capacity)

  Note that the directions are different for rx and tx.
  */
  uint32_t n_head, n_tail;
  uint32_t n_room, n_content;

} RING_BUFFER;


bool rb_empty (const RING_BUFFER* rb);
bool rb_full (const RING_BUFFER* rb);

#endif
