This sample allocated 3 tasks with kfork().
And demonstrated task switch.
The task context is preserved in PROC struct, not on stack.

This sample is not a preemptive multi-tasking.
Each task still give up its cpu time voluntarily when pending input.
Kernel doesn't preempt the task.

This sample demonstrated the sleep/wakeup paradigm for process synchronization.
Any 2 parts of the system can be coupled together with this paradigm.
The sleep/wakeup operation must be implemented properly, such as atomic.


TODO:
- the while loop seems unnecessary in kgetc(); // NO, it is very critical!
- the SBUFSIZE for uart seems too small; // enlarged
- the echo back in ugets()/kgets() still not working.