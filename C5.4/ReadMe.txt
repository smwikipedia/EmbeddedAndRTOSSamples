This sample allocated 3 tasks with kfork().
And demonstrated task switch.
The task context is preserved on stack.

This sample is not a preemptive multi-tasking.
Each task still give up its cpu time voluntarily when pending input.
Kernel doesn't preempt the task.

Build Notes:
After downloading the GNU ARM toolchain, locate the libgcc.a file in the toolchain folder.
Such as: arm-gnu-toolchain-13.3.rel1-x86_64-arm-none-eabi/lib/gcc/arm-none-eabi/13.3.1/libgcc.a
Copy the libgcc.a to the "libs" directory.
Then `make BUILD`

Debug Notes:
About redirecting QEMU UART via telnet, after lauching the QEMU with "make DEBUG",
use below command in another Linux terminal window to connect to the QEMU uart:

"telnet 127.0.0.1 1124"
