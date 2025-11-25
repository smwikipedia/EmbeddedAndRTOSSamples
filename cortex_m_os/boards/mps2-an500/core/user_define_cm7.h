// user_define_cm7 first defines some macros to configure the core_cm7.h
// then include the core_cm7.h
// follow the CMSIS/Utilities/ARM_Example.h

#ifdef __cplusplus
extern "C"
{
#endif

    /* =========================================================================================================================== */
    /* ================                                Interrupt Number Definition ================ */
    /* =========================================================================================================================== */

    typedef enum
    {
        /* =======================================  ARM Cortex-M3 Specific Interrupt Numbers  ======================================== */
        Reset_IRQn = -15, /*!< -15  Reset Vector, invoked on Power up and warm reset                     */
        NonMaskableInt_IRQn =
        -14, /*!< -14  Non maskable Interrupt, cannot be stopped or preempted */
        HardFault_IRQn = -13, /*!< -13  Hard Fault, all classes of Fault */
        MemoryManagement_IRQn = -12, /*!< -12  Memory Management, MPU mismatch, including Access Violation
                                          and No Match */
        BusFault_IRQn = -11,   /*!< -11  Bus Fault, Pre-Fetch-, Memory Access
                                  Fault, other address/memory   related Fault   */
        UsageFault_IRQn = -10, /*!< -10  Usage Fault, i.e. Undef Instruction, Illegal State Transition        */
        SVCall_IRQn = -5, /*!< -5 System Service Call via SVC instruction */
        DebugMonitor_IRQn = -4, /*!< -4 Debug Monitor */
        PendSV_IRQn       = -2, /*!< -2 Pendable request for system service */
        SysTick_IRQn      = -1, /*!< -1 System Tick Timer */
        /* ========================================  ARM_Example Specific Interrupt Numbers  ========================================= */
        UART0_RX_IRQn = 0, /*!< 0  UART 0 receive interrupt */
        UART0_TX_IRQn = 1, /*!< 0  UART 0 transmit interrupt */

    } IRQn_Type;


/* =========================================================================================================================== */
/* ================                           Processor and Core Peripheral Section ================ */
/* =========================================================================================================================== */

/* ===========================  Configuration of the ARM Cortex-M3 Processor and Core Peripherals  =========================== */
#define __CM3_REV \
    0x0100U /*!< CM3 Core Revision */
#define __NVIC_PRIO_BITS \
    3 /*!< Number of Bits used for Priority Levels */
#define __Vendor_SysTickConfig \
    0 /*!< Set to 1 if different SysTick Config is used */
#define __MPU_PRESENT \
    1 /*!< MPU present or not */
#define __FPU_PRESENT \
    0 /*!< FPU present or not */

#include <core_cm7.h>

#ifdef __cplusplus
}
#endif
