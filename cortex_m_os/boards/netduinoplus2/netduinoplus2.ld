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
    .isr_vectors : {
        /*About KEEP(): https://stackoverflow.com/questions/9827157/what-does-keep-mean-in-a-linker-script */
        KEEP(*(.isr_vectors))
        } > ROM
    .text : {
        *(.text*)
        } > ROM
    .rodata : { *(.rodata*) } > ROM
    .bss : { *(.bss*) } > RAM

  /*
    I think the stack should be explicilty allocated in RAM.
    The MSP register value shows a value in ROM area if I didn't specify below.
    But it works on QEMU. ROM is read-only but stack must be writable. So it is strange.
    In nRF CMSIS Device Family Pack, the stack does go to RAM area.
    So maybe this is a QEMU bug. On a physical board it will not work.
  */
    .stack : { *(.stack*) } > RAM
    .heap : { *(.heap*) } > RAM
}
