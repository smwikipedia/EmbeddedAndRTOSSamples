/*
A simple but essential linker script for Cortex-M4.
Inspired by: https://stackoverflow.com/a/67592534/264052
*/

/*
 From here: https://www.scribd.com/document/705805574/DAI0500B-cortex-m7-on-v2m-mps2
 Use the PRAM range for RAM: 0x60000000~0x60FFFFFF PSRAM (16MB)
*/
MEMORY
{
  ROM (rx) : ORIGIN = 0x00000000, LENGTH = 0x400000
  RAM (rwx) : ORIGIN = 0x60000000,   LENGTH = 0x1000000
}

ENTRY(Reset_Handler)

SECTIONS
{
    .isr_vector : {
        /*About KEEP(): https://stackoverflow.com/questions/9827157/what-does-keep-mean-in-a-linker-script */
        KEEP(*(.isr_vector))
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
