
#include <stdint.h>
#include <board_api.h>
int32_t main (void) __attribute__ ((noreturn));
int32_t main (void)
{
  uint8_t data[20];
  int32_t read_count;

  /*
  Echo test: read from one uart and echo to to another.
  */
  while (1)
    {
      // read from uart 0
      read_count = board_uart_read (0, data, sizeof (data));
      if (read_count > 0)
        {
          // : to uart 1
          // You can change it to 0, 1 because we only expose UART0/1 in the mp22-an500.mk.
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
