
#include <stdint.h>
#include <board_api.h>

// This globl variable will cause linker warning.
// warning: ./build/os.elf has a LOAD segment with RWX permissions
static uint8_t msg[] = "hello, SysTick!\r\n";

extern volatile uint32_t elapsed_1s;
extern uint32_t g_sys_uptime_ms;

int32_t main (void) __attribute__ ((noreturn));

void delay_5s (void)
{
  uint32_t n = 0;
  while (n < 5) {
    if (elapsed_1s) {
      n++;
      elapsed_1s = 0;
    }
  }
}


int32_t main (void)
{
  uint8_t data[20];
  int32_t read_count;

  /*
  Echo test: read from one uart and echo to to another.
  */
  delay_5s ();
  while (1) {
    // write to uart 0 every 1s
    if (elapsed_1s == 1) {
      elapsed_1s = 0;
      board_uart_write (0, msg, sizeof (msg));
    }


    // read from uart 0
    read_count = board_uart_read (0, data, sizeof (data));
    if (read_count > 0) {
      // Only send to UART 0/1 because we only expose UART0/1 in the mps2-an500.mk file.
      board_uart_write (0, data, read_count);
      board_uart_write (1, data, read_count);
    }
  }
}

void _start (void)
{
  /*
  C/C++ initialiation,
  or your own application specific main(),
  or RTOS
  ...
  */

  main ();
}
