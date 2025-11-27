
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
    .cpu cortex-m7
    .fpu softvfp
    /*
    ".thumb" ensures labels defined within a thumb block are correctly marked as thumb entry points.
    i.e., their address will be represented as address | 1 when referenced by BX/BLX from ARM code.
    This performs the same action as ".code 16."
    The ".code" directive selects the instruction set being generated.
    The value 16 selects Thumb, with the value 32 selecting ARM.
    */
    .thumb


    // Stack
    .section .stack
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
    .text always designates the code section regardless of its actual name in the output format.
    .section lets the programmer specify any section name and not supported by a.out format.
    I choose to always use .section directive.
    Ref: https://stackoverflow.com/questions/52284829/difference-between-section-text-and-text-with-gas
    */
    .section .text.Default_Handler
    @.thumb
    /*
    Ref: https://www.sourceware.org/binutils/docs/as/ARM-Directives.html
    The ".thumb_func" directive specifies that the following symbol is the name of a Thumb encoded function.
    This information is necessary in order to allow the assembler and linker to generate correct code for interworking between Arm and Thumb instructions
    and should be used even if interworking is not going to be performed.
    The presence of this directive also implies .thumb.
    So we don't need the ".thumb" directive.
     */
    .thumb_func
    .align 1
    // "%function" is a type attribute of a symbol
    .type Default_Handler, %function
    .globl Default_Handler
Default_Handler:
    b  Default_Handler
    .size  Default_Handler, . - Default_Handler


    @Reset Handler
    .section .text.Reset_Handler
    .thumb_func
    .align 1
    .type Reset_Handler, %function
    .globl Reset_Handler
Reset_Handler:
    nop
    bl SystemInit
    bl _start
    .size  Reset_Handler, . - Reset_Handler


    .section .text.Peripheral_Handlers
    .thumb_func
    .align 1
    .type Uart_Handler, %function
    .globl Uart_Handler
Uart_Handler:
    b  Uart_C_Handler
    .size  Uart_Handler, . - Uart_Handler


    /*
    These are the weak handlers that has only dummy logic
    */
    .section .text.Weak_Handlers
    .thumb_func
    .align 1

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

    .weak      Uart0_Rx_Handler
    .thumb_set Uart0_Rx_Handler, Uart_Handler

    .weak      Uart0_Tx_Handler
    .thumb_set Uart0_Tx_Handler, Uart_Handler


    // "ax": allocatable, executable
    .section .isr_vector, "ax"
    .align 2
    .globl __isr_vector
    // Vector Table
__isr_vector:
    // The address 0 is the MSP. So place the __StackTop here.
    .long   __StackTop
    // The address 0x4 is where M4 looks for a reset handler. So place Reset_Handler here.
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
    .long Uart0_Rx_Handler  // exception 16, IRQ 0
    .long Uart0_Tx_Handler  // exception 17, IRQ 1
