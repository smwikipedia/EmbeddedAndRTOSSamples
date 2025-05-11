/*
A simple but essential linker script for Cortex-M4.
Inspired by: https://stackoverflow.com/a/67592534/264052
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

  /*
    I think the stack should be explicilty allocated in RAM.
    The MSP register value shows a value in ROM area if I didn't specify below.
    But it works on QEMU. Strange.
    In nRF CMSIS Device Family Pack, the stack does go to RAM area.
    So maybe this is a QEMU bug and on a real board it won't work because ROM is not writable.
  */
    .stack : { *(.stack*) } > RAM
    .heap : { *(.heap*) } > RAM
}
