/*
 SysTick is a timer that generates the SysTick interrupt.


 This driver provides more APIs with reference to the Zephyr RTOS Timer APIs.
 https://docs.zephyrproject.org/latest/doxygen/html/group__timer__apis.html

 How SysTick timer works.
 https://www.youtube.com/watch?v=aLCUDv_fgoU


 - SysTick is the heartbeat of most Cortex-M RTOS systems. It's configured and owned by the OS.
 - Applications generally should NOT have direct APIs to turn SysTick on or off. Doing so bypasses the OS and causes instability.
 - The exception is for low-power management. In these cases, the OS provides higher-level power management APIs (e.g., osKernelSuspend())
   which internally handle the disabling and re-enabling of SysTick as part of a larger power state transition.
 - For bare-metal systems, the developer has full control to use and manage SysTick as desired.

*/

#include <board_api.h>

uint32_t g_sys_uptime_ms     = 0;
volatile uint32_t elapsed_1s = 0;


void SysTick_C_Handler (void)
{
  // Move the UART output work out of ISR into the main to reduce the ISR overhead.
  g_sys_uptime_ms += 10;
  if (g_sys_uptime_ms % 1000 == 0) {
    elapsed_1s = 1;
  }
}
