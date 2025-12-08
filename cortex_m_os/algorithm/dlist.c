#include <stdint.h>
#include <stdbool.h>
#include <data_structures.h>

/*
 dlist
*/
LIST_ENTRY* init_dlist_head (LIST_ENTRY* list_head)
{
  list_head->next = list_head;
  list_head->prev = list_head;
  return list_head;
}

bool is_dlist_empty (const LIST_ENTRY* list_head)
{
  return (bool)(list_head->next == list_head);
}
