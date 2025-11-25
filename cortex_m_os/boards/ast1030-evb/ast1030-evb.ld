/*
A simple but essential linker script for Cortex-M4.
Inspired by: https://stackoverflow.com/a/67592534/264052
*/

/*
 From here: https://docs.zephyrproject.org/latest/boards/aspeed/ast1030_evb/doc/index.html
 The ROM size is 1MB (0x100000), the RAM size is 768KB.

 From Zephyr RTOS file: boards/aspeed/ast1030_evb/ast1030_evb.dts
 The SRAM is like below. The format is <base, size>:
 &sram0 {
	reg = <0 DT_SIZE_K(448)>, <0x70000 DT_SIZE_K(320)>;
  };

 So I specify the RAM at 0x7000, length is 320K (0x50000).
 Otherwise, you will see Usage Fault at boot.
*/
MEMORY
{
  ROM (rx) : ORIGIN = 0x00000000, LENGTH = 0x100000
  RAM (rwx) : ORIGIN = 0x70000,   LENGTH = 0x50000
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
    But it works on QEMU. ROM is read-only but stack must be writable. So it is strange.
    In nRF CMSIS Device Family Pack, the stack does go to RAM area.
    So maybe this is a QEMU bug. On a physical board it will not work.
  */
    .stack : { *(.stack*) } > RAM
    .heap : { *(.heap*) } > RAM
}
