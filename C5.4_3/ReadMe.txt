This sample should behave exactly the same as C5.4_2.
The difference is I am trying to using the PL011 drvier from TF-M repo.

The TF-M PL011 driver is from:
https://github.com/TrustedFirmware-M/trusted-firmware-m/tree/main/platform/ext/target/arm/drivers/usart/pl011

My purpose is to less the burden of device driver development and focus on OS itself.

Bug:
1. when input into the UART and press enter quickly, the LCD display may dead loop and no chance for input.



Below is the ReadMe for C5.4_2 just for convenience.

This sample allocated 2 tasks with kfork().
The 2 tasks are of the same priority so they both have a chance to run.
The task context is preserved in PROC struct, not on stack.

This sample is not a preemptive multi-tasking.
Each task still give up its cpu time voluntarily if there's no input.
Kernel doesn't preempt the task.
The interrupt handler of a device will wake up the task waiting input from that device.

This sample demonstrated the sleep/wakeup paradigm for process synchronization.
Any 2 parts of the system can be coupled together with this paradigm.
The sleep/wakeup operation must be implemented properly, such as atomic.

Build Notes:
After downloading the GNU ARM toolchain, locate the libgcc.a file in the toolchain folder.
Such as: arm-gnu-toolchain-13.3.rel1-x86_64-arm-none-eabi/lib/gcc/arm-none-eabi/13.3.1/libgcc.a
Copy the libgcc.a to the "libs" directory.
Then `make BUILD`

Debug Notes:
About redirecting QEMU UART via telnet, after lauching the QEMU with "make DEBUG",
use below command in another Linux terminal window to connect to the QEMU uart:

"telnet 127.0.0.1 1124"

The QEMU gdbserver started with "-s" is NOT connectable with "target remote :1234" command in gdb client
until above telnet step is finished. -- Be advised!

Previously, due to the unfair task scheduling which is soley based on priority, and the incorrect
implemenation of echo back, the telnet UART window didn't show output properly.


Notes:
- the while loop seems unnecessary in kgetc(); // NO, it is very critical!
- the SBUFSIZE for uart seems too small; // enlarged
- the echo back in ugets()/kgets() still not working; // fixed
