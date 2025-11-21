
#include <user_define_cm4.h>

void main(void) __attribute__ ((noreturn));
void main(void)
{
    while(1);
}

void _start(void)
{
    /*
    C/C++ initialiation,
    or your own application specific main(),
    or RTOS
    ...
    */

    main();
}
