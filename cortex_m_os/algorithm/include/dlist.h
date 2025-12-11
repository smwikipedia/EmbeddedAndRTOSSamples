#ifndef ALGO_DLIST_H
#define ALGO_DLIST_H

#include <stdbool.h>

typedef struct _LIST_ENTRY LIST_ENTRY;

typedef struct _LIST_ENTRY {
  LIST_ENTRY* next;
  LIST_ENTRY* prev;
} LIST_ENTRY;

LIST_ENTRY* init_dlist_head (LIST_ENTRY* list_head);
bool is_dlist_empty (const LIST_ENTRY* list_head);

#endif
