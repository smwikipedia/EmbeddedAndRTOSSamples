/*
Refer to : CMSIS5/Device/ARM/ARMCM7/Source/GCC/startup_ARMCM7.S
*/


/* Define size of stack. Size must be multiple of 4. */
#define __STARTUP_CONFIG_STACK_SIZE   0x1000

/*
Define alignement of stack. Alignment will be 2 to the power of __STARTUP_CONFIG_STACK_ALIGNEMENT.
Since calling convention requires that the stack is aligned to 8-bytes when a function is called,
the minimum __STARTUP_CONFIG_STACK_ALIGNEMENT is therefore 3.
*/
#define __STARTUP_CONFIG_STACK_ALIGNEMENT 3

/* Define size of heap. Size must be multiple of 4. */
#define __STARTUP_CONFIG_HEAP_SIZE   0x1000

    // collectively define the characteristics of the generated machine code
    .syntax unified
    @ No need to specify both .arch and .cpu.
    @ .cpu is usually the more precise and preferred directive.
    @ .arch armv7-m
    .cpu cortex-m7
    .fpu softvfp


    // Stack
    .section .stack
    /*
    For ARM, the .align number is the number of low-order zero bits the location counter must have after advancement.
    For example ‘.align 3’ advances the location counter until it is a multiple of 8.
    If the location counter is already a multiple of 8, no change is needed.
     */
    .align __STARTUP_CONFIG_STACK_ALIGNEMENT
    // generate an absolute symbol Stack_Size: no address, has a value, and no memory consumption
    .equ   Stack_Size, __STARTUP_CONFIG_STACK_SIZE
    .globl __StackTop
    .globl __StackLimit
__StackLimit:
    // stack grows from high address top->low address limit
    .space Stack_Size
    // the symbol __StackLimit has the address of stack bottom at low address, and associated with a value of the stack size
    .size __StackLimit, . - __StackLimit
__StackTop:
    // the symbol __StackTop has the address of stack top at high address, and associated with a value 0.
    .size __StackTop, . - __StackTop

    // Heap
    .section .heap
    // align to 2^3 = 8 bytes.
    .align 3
    // generate an absolute symbol Heap_Size: no address, has a value, and no memory consumption
    .equ Heap_Size, __STARTUP_CONFIG_HEAP_SIZE
    .globl __HeapBase
    .globl __HeapLimit
__HeapBase:
    // heap grows from low address base -> high address limit
    .space Heap_Size
    // __HeapBase has the address of heap base, and associated with a value of the heap size
    .size __HeapBase, . - __HeapBase
__HeapLimit:
    // __HeapLimit has the address of heap limit, and associated with a value of 0
    .size __HeapLimit, . - __HeapLimit


    /*
    ".thumb" ensures labels defined within a thumb block are correctly marked as thumb entry points.
    i.e., their address will be represented as address | 1 when referenced by BX/BLX from ARM code.
    This performs the same action as ".code 16."
    The ".code" directive selects the instruction set being generated.
    The value 16 selects Thumb, with the value 32 selecting ARM.
    The .thumb directive in ARM unified assembly language has file-local scope and applies to all subsequent instructions until explicitly changed by another directive.
    */
    .thumb
    /*
    .text always designates the code section regardless of its actual name in the output format.
    .section lets the programmer specify any section name and not supported by a.out format.
    I choose to always use .section directive.
    Ref: https://stackoverflow.com/questions/52284829/difference-between-section-text-and-text-with-gas
    */
    .section .text
    .align 2

    /*
    I met an annoying issue when debugging the reset code with GDB server started with "-S".
    And it seems to be related to the .type directive (https://sourceware.org/binutils/docs/as.html#Type)
    A related question: https://stackoverflow.com/questions/79831305/why-is-gdb-showing-the-wrong-function-name

    (1) with the ".type <name>, %function"
    I have to put the Reset_Handler as the first function in the .text section.
    Otherwise the GDB will show a wrong function name when stopping at the first instuction in the Reset_Handler.
    The wrong function is whatever function immediately placed before the Reset_Handler and with a .type directive.

    In CMSIS 5, the startup_ARMCM7.S puts the Reset_Handler as the first function in the .code section.
    If I move it to somewhere else, the exactly same issue will happen.
    If I remove the ".type Reset_Handler, %function", the function name can be displayed correctly no matter where I
    put the Reset_Handler.

    (2) with "".func"
    I can put Reset_Handler anywhere. And the GDB shows the correct name for instuctions in Reset_Handler.

    I consulted AI, and it says this:
    "In essence, .type <name>, %function is the fundamental and generally preferred directive for declaring a symbol as a function for
    the linker and debugger in modern ARM assembly. .func is more of a legacy or supplementary directive that provides a structural block
    and might trigger some implicit behaviors related to debugging."

    And it gives me below example.
    But this example also has the issue in (1)

    my_function:
    .globl my_function
    .type my_function, %function  @ Defines type for linker/debugger
    .func                         @ Marks function start for block structure / older tools
    .thumb_func                   @ Essential for inter-working (can be implicit with .type %function)
    .fnstart                      @ Begins detailed CFI for debugger

    @ ... function code ...

    .fnend                        @ Ends detailed CFI
    .endfunc                      @ Ends function block


    (3) And the GNU as options --gstabs and --gdwarf are also related.
    According to AI,
    --gstabs is old and limited, it is rare in modern ARM context.
    --gdwarf is modern and powerful, it is standard for modern ARM development.

    (4) Conclusion
    Because of (1), I keep using the .type directive and put Reset_Handler as the first function in .text section.
    Because of (3), I choose --gdwarf-5 in the mps2-an500.mk file.
     */
Reset_Handler:
    .func Reset_Handler
    .thumb_func
    .type Reset_Handler, %function
    .globl Reset_Handler
    nop
    bl SystemInit
    bl _start
    .size  Reset_Handler, . - Reset_Handler
    .endfunc

    /*
    Ref: https://www.sourceware.org/binutils/docs/as/ARM-Directives.html
    The ".thumb_func" directive specifies that the following symbol is the name of a Thumb encoded function.
    This information is necessary in order to allow the assembler and linker to generate correct code for interworking between Arm and Thumb instructions
    and should be used even if interworking is not going to be performed.
    The presence of this directive also implies .thumb.
    So we don't need the ".thumb" directive.
     */
    // "%function" is a type attribute of a symbol
Default_Handler:
    .func Default_Handler
    .thumb_func
    .type Default_Handler, %function
    .globl Default_Handler
    b  Default_Handler
    .size  Default_Handler, . - Default_Handler
    .endfunc

    // Peripheral Handlers
Uart_Handler:
    .func Uart_Handler
    .thumb_func
    .type Uart_Handler, %function
    .globl Uart_Handler
    @ save the EXC_RETURN value which is automatically held in LR
    push {lr}
    bl  Uart_C_Handler
    @ restore the EXC_RETURN into PC, which triggers the exception return sequence
    pop {pc}
    .size  Uart_Handler, . - Uart_Handler
    .endfunc

    /*
    These are the weak handlers that has only dummy logic
    */

    /*
    .thumb_set performs the equivalent of a .set directive in that it creates a symbol which is an alias for another symbol (possibly not yet defined).
    .thumb_set is similar to set, but additionally set the LSB of an symbol value to 1 to mark the address as thumb mode.
    This directive also has the added property in that it marks the aliased symbol as being a thumb function entry point, in the same way that the .thumb_func directive does.
    https://www.sourceware.org/binutils/docs/as/ARM-Directives.html
    */
    .weak      NMI_Handler
    .thumb_set NMI_Handler, Default_Handler

    .weak      HardFault_Handler
    .thumb_set HardFault_Handler, Default_Handler

    .weak      MemManage_Handler
    .thumb_set MemManage_Handler, Default_Handler

    .weak      BusFault_Handler
    .thumb_set BusFault_Handler, Default_Handler

    .weak      UsageFault_Handler
    .thumb_set UsageFault_Handler, Default_Handler

    .weak      SVC_Handler
    .thumb_set SVC_Handler, Default_Handler

    .weak      DebugMon_Handler
    .thumb_set DebugMon_Handler, Default_Handler

    .weak      PendSV_Handler
    .thumb_set PendSV_Handler, Default_Handler

    .weak      SysTick_Handler
    .thumb_set SysTick_Handler, Default_Handler


disable_interrupt:
    .func disable_interrupt
    .thumb_func
    .type disable_interrupt, %function
    .globl disable_interrupt
    // TBD
    bx lr
    .endfunc

enable_interrupt:
    .func enable_interrupt
    .thumb_func
    .type enable_interrupt, %function
    .globl enable_interrupt
    // TBD
    bx lr
    .endfunc



    // "ax": allocatable, executable
    .section .isr_vectors, "ax"
    .align 2
    .globl __isr_vectors
    .globl __isr_vectors_end
    .globl __isr_vectors_size
    // Vector Table
__isr_vectors:
    .long   __StackTop
    .long  Reset_Handler
    .long  NMI_Handler
    .long  HardFault_Handler
    .long  MemManage_Handler
    .long  BusFault_Handler
    .long  UsageFault_Handler
    .long  0
    .long  0
    .long  0
    .long  0
    .long  SVC_Handler
    .long  DebugMon_Handler
    .long  0
    .long  PendSV_Handler
    .long  SysTick_Handler

    // External Interrupts -- This part varies between SoCs
    // TBD
    // check the user_define_cm7.h for the interrupt numbers
    .long Uart_Handler  // exception 16, IRQ 0, UART0 RX
    .long Uart_Handler  // exception 17, IRQ 1, UART0 TX
    .long Uart_Handler  // exception 18, IRQ 2, UART1 RX
    .long Uart_Handler  // exception 19, IRQ 3, UART1 TX
    .long Uart_Handler  // exception 20, IRQ 4, UART2 RX
    .long Uart_Handler  // exception 21, IRQ 5, UART2 TX

    /*
    The DUIO0646C spec says 0~239. Total 240.
    The DDI0489F CortexM7 TRM also says "The NVIC supports up to 240 interrupts each with up to 256 levels of priority."
    CMSIS 5 startup_ARMCM7.S says IRQn are from 0 ~ 224. Total 225. That's 240 minus 15 system exceptions.
    I think the "interrupts" in DDI0489F should be "exceptions".

    */
    .space   (219 * 4)                          /* Interrupts 6 .. 224 are left out, that's 219 */
__isr_vectors_end:
    .equ     __isr_vectors_size, __isr_vectors_end - __isr_vectors
    .size    __isr_vectors, . - __isr_vectors

    .end
