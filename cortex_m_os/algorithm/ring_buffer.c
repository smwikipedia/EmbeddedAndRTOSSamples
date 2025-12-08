#include "ring_buffer.h"
#include <stdint.h>
#include <stdbool.h>
#include <data_structures.h>
#include <utilities.h>


bool rb_empty (const RING_BUFFER* rb)
{
  return (rb->n_head == rb->n_tail);
}

bool rb_full (const RING_BUFFER* rb)
{
  return ((rb->n_head + 1) % RING_BUFFER_LENGTH == rb->n_tail);
}

/*
 Ring buffer chain
*/
int32_t send_next (LIST_ENTRY* curr_link)
{
  // RING_BUFFER* curr_buffer;
  // RING_BUFFER* next_buffer;

  // // no buffer chain
  // if (is_dlist_empty (curr_link))
  // {
  //   return 0;
  // }

  // curr_buffer = CS (curr_link, buffer_chain_link, RING_BUFFER);
  // next_buffer = CS (curr_link->next, buffer_chain_link, RING_BUFFER);

  // while (((curr_buffer->n_tail + 1) % RING_BUFFER_LENGTH) != curr_buffer->n_head)
  // {
  //   // TBD
  // }

  return 0;
}


int32_t recv_prev (LIST_ENTRY* curr_link)
{
  // RING_BUFFER* curr_buffer;
  // RING_BUFFER* prev_buffer;

  // // no buffer chain
  // if (is_dlist_empty (curr_link))
  //   {
  //     return 0;
  //   }

  // // previous buffer empty
  // if (rb_empty (prev_buffer))
  //   {
  //     return 0;
  //   }

  // if (1)
  //   {
  //   }

  // curr_buffer = CS (curr_link, buffer_chain_link, RING_BUFFER);
  // prev_buffer = CS (curr_link->prev, buffer_chain_link, RING_BUFFER);

  // // not always receive, do not overrun current buffer
  // while (RB_ADVANCE_POS (curr_buffer->n_head) != curr_buffer->n_tail)
  //   {
  //     curr_buffer->buffer[RB_ADVANCE_POS (curr_buffer->n_head)] = prev_buffer->buffer[RB_ADVANCE_POS (prev_buffer->n_tail)];

  //     if (rb_empty (prev_buffer))
  //       {
  //       }
  //   }
  return 0;
}
