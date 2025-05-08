/*
A simple but essential linker script for Cortex-M4.
Ref: https://stackoverflow.com/a/67592534/264052
*/

MEMORY
{
  ROM (rx) : ORIGIN = 0x00000000, LENGTH = 0x100000
  RAM (rwx) : ORIGIN = 0x20000000, LENGTH = 0x40000
}

ENTRY(Reset_Handler)

SECTIONS
{
    .text : {
        /*About KEEP(): https://stackoverflow.com/questions/9827157/what-does-keep-mean-in-a-linker-script */
        KEEP(*(.isr_vector))
        *(.text*)
        } > ROM
    .rodata : { *(.rodata*) } > ROM
    .bss : { *(.bss*) } > RAM
}
