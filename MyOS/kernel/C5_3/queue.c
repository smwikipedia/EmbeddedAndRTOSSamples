#include "types.h"
#include "proc.h"
#include "display.h"

/*
Return the first element from the list.
Return NULL if the list is empty.
The parameter "list" is ** because we may need to modify it. So pass by pointer.
*/
PROC *get_proc(PROC **list)
{
    if(*list == NULL)
    {
        return NULL;
    }

    PROC * first = *list;
    *list = first->next; // *list will automatically change to NULL if there's only 1 element remaining.
    return first;    
}

/*
Append the p at the end of the list.
The parameter "list" is ** because we may need to modify it. So pass by pointer.
*list always points to the first element of the list.
*/
u32 put_proc(PROC **list, PROC *p)
{
    PROC *last = NULL;
    if(*list == NULL)
    {
        *list = p;
    }
    else
    {
        last = *list;
        while(last->next != NULL)
        {
            last = last->next;
        }
        last->next = p;
    }

    p->next = NULL;
    return RET_SUCCESS;
}

/*
Enter p into queue by priority.
If same priority, follow FIFO order.
*/
u32 enqueue(PROC **queue, PROC *p)
{
    PROC *current = NULL;
    PROC *previous = NULL;
    if(*queue == NULL)
    {
        *queue = p;
        p->next = NULL;
    }
    else
    {
        current = *queue;
        while(current != NULL && current->priority >= p->priority)
        {
            previous = current;
            current = current->next;
        }
        if(current == NULL)
        {
            p->next = NULL;
            previous->next = p;
        }
        else
        {
            p->next = current;
            if(previous != NULL)
            {
                previous->next = p;
            }
            else //if(current == *queue)
            {
                *queue = p;
            }
        }
    }

    return RET_SUCCESS;
}

/*
Remove and return the first element from the queue.
The enqueue() will ensure that the first element is always the proper one by priority.
So dequeue() doesn't need to consider the priority.

If empty, i.e. queue is NULL, then return NULL.
The parameter "queue" is ** because we may need to modify it. So pass by pointer.
*/
PROC *dequeue(PROC **queue)
{
    if(*queue == NULL)
    {
        return NULL;
    }

    PROC * first = *queue;
    *queue = first->next; // *queue will automatically change to NULL if there's only 1 element remaining.
    return first;    
}

/*
Print list elements of the ready queue or free list.
*/
u32 printList(u8 *desc, PROC *p)
{
    kprintf("[%s]:\n", desc);
    PROC *last = p;
    while(last != NULL)
    {
        kprintf("[%d, %d]", last->pid, last->priority); // [pid, priority]
        last = last->next;
    }
    kprintf("\n");

    return RET_SUCCESS;
}