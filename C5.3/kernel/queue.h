#include "types.h"
#include "proc.h"

PROC *get_proc(PROC **list);

u32 put_proc(PROC **list, PROC *p);

u32 enqueue(PROC **queue, PROC *p);

PROC *dequeue(PROC **queue);

u32 printList(u8 *desc, PROC *p);