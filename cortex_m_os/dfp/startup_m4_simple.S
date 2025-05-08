
/* Define size of stack. Size must be multiple of 4. */
#define __STARTUP_CONFIG_STACK_SIZE   0x1000

/* Define alignement of stack. Alignment will be 2 to the power of __STARTUP_CONFIG_STACK_ALIGNEMENT. Since calling convention requires that the stack is aligned to 8-bytes when a function is called, the minimum __STARTUP_CONFIG_STACK_ALIGNEMENT is therefore 3. */
#define __STARTUP_CONFIG_STACK_ALIGNEMENT 3

/* Define size of heap. Size must be multiple of 4. */
#define __STARTUP_CONFIG_HEAP_SIZE   0x1000

    .syntax unified
    .cpu cortex-m4
    .fpu softvfp
    .thumb

    // Stack
    .section .stack
    .align __STARTUP_CONFIG_STACK_ALIGNEMENT
    .equ   Stack_Size, __STARTUP_CONFIG_STACK_SIZE
    .globl __StackTop
    .globl __StackLimit
__StackLimit:
    .space Stack_Size
    .size __StackLimit, . - __StackLimit
__StackTop:
    .size __StackTop, . - __StackTop

    // Heap
    .section .heap
    .align 3
    .equ Heap_Size, __STARTUP_CONFIG_HEAP_SIZE
    .globl __HeapBase
    .globl __HeapLimit
__HeapBase:
    .if Heap_Size
    .space Heap_Size
    .endif
    .size __HeapBase, . - __HeapBase
__HeapLimit:
    .size __HeapLimit, . - __HeapLimit


    .section .isr_vector, "ax"
    .align 2
    .globl __isr_vector
    // Vector Table
__isr_vector:
    .long   __StackTop                  /* Top of Stack */
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

    // External Interrupts -- This part varis between SoCs
    // TBD


    /*
    .text always designates the code section regardless of its actual name in the output format.
    .section lets the programmer specify any section name and not supported by a.out format.
    I choose to always use .section directive.
    Ref: https://stackoverflow.com/questions/52284829/difference-between-section-text-and-text-with-gas
    */
     .section .text.Default_Handler
    .thumb
    .thumb_func
    .align 1
    .type Default_Handler, %function
Default_Handler:
Infinite_Loop:
    b  Infinite_Loop
    .size  Default_Handler, .-Default_Handler

    .section .text.Weak_Handlers
    .thumb
    .thumb_func
    .align 1

    .weak      NMI_Handler
    .thumb_set NMI_Handler,Default_Handler

    .weak      HardFault_Handler
    .thumb_set HardFault_Handler,Default_Handler

    .weak      MemManage_Handler
    .thumb_set MemManage_Handler,Default_Handler

    .weak      BusFault_Handler
    .thumb_set BusFault_Handler,Default_Handler

    .weak      UsageFault_Handler
    .thumb_set UsageFault_Handler,Default_Handler

    .weak      SVC_Handler
    .thumb_set SVC_Handler,Default_Handler

    .weak      DebugMon_Handler
    .thumb_set DebugMon_Handler,Default_Handler

    .weak      PendSV_Handler
    .thumb_set PendSV_Handler,Default_Handler

    .weak      SysTick_Handler
    .thumb_set SysTick_Handler,Default_Handler



    .section .text.Reset_Handler
    .thumb
    .thumb_func
    .align 1
    .globl Reset_Handler
    .type Reset_Handler, %function
Reset_Handler:
    bl SystemInit
    bl _start
