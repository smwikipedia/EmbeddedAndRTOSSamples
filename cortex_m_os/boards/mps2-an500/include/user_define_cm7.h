// user_define_cm7 first defines some macros to configure the core_cm7.h
// then include the core_cm7.h
// follow the CMSIS/Utilities/ARM_Example.h

#ifdef __cplusplus
extern "C" {
#endif

/* =========================================================================================================================== */
/* ================                                Interrupt Number Definition ================ */
/* =========================================================================================================================== */

typedef enum {
  /* =======================================  ARM Cortex-M3 Specific Interrupt Numbers  ======================================== */
  Reset_IRQn            = -15, /*!< -15  Reset Vector, invoked on Power up and warm reset */
  NonMaskableInt_IRQn   = -14, /*!< -14  Non maskable Interrupt, cannot be stopped or preempted */
  HardFault_IRQn        = -13, /*!< -13  Hard Fault, all classes of Fault */
  MemoryManagement_IRQn = -12, /*!< -12  Memory Management, MPU mismatch, including Access Violation and No Match */
  BusFault_IRQn         = -11, /*!< -11  Bus Fault, Pre-Fetch-, Memory Access Fault, other address/memory   related Fault   */
  UsageFault_IRQn       = -10, /*!< -10  Usage Fault, i.e. Undef Instruction, Illegal State Transition        */
  SVCall_IRQn           = -5,  /*!< -5 System Service Call via SVC instruction */
  DebugMonitor_IRQn     = -4,  /*!< -4 Debug Monitor */
  PendSV_IRQn           = -2,  /*!< -2 Pendable request for system service */
  SysTick_IRQn          = -1,  /*!< -1 System Tick Timer */

  /* ========================================  ARM_Example Specific Interrupt Numbers  ========================================= */
  UART0_RX_IRQn = 0, /*!< 0  UART 0 receive interrupt */
  UART0_TX_IRQn = 1, /*!< 0  UART 0 transmit interrupt */

} IRQn_Type;


/*
 --------  Configuration of Core Peripherals  --------
 Copied from CMSIS5 Device/ARM/ARMCM7/Include/ARMCM7.h
*/
#define __CM7_REV 0x0000U         /* Core revision r0p0 */
#define __MPU_PRESENT 1U          /* MPU present */
#define __VTOR_PRESENT 1U         /* VTOR present */
#define __NVIC_PRIO_BITS 3U       /* Number of Bits used for Priority Levels */
#define __Vendor_SysTickConfig 0U /* Set to 1 if different SysTick Config is used */
#define __FPU_PRESENT 0U          /* no FPU present */
#define __FPU_DP 0U               /* unused */
#define __ICACHE_PRESENT 1U       /* Instruction Cache present */
#define __DCACHE_PRESENT 1U       /* Data Cache present */
#define __DTCM_PRESENT 1U         /* Data Tightly Coupled Memory present */


#include <core_cm7.h>

#ifdef __cplusplus
}
#endif
