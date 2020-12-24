This sample allocated 3 tasks with kfork().
And demonstrated task switch.
The task context is preserved in PROC struct, not on stack.

This sample is not a preemptive multi-tasking.
Each task still give up its cpu time voluntarily when pending input.
Kernel doesn't preempt the task.