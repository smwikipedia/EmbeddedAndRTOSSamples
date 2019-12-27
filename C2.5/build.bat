@echo off
REM arm-none-eabi-gcc -S -mcpu=arm926ej-s t.c -o t1.S
REM arm-none-eabi-as t1.S -o t1.o
REM arm-none-eabi-as -mcpu=arm926ej-s  t1.S -o t5.o
@echo on

arm-none-eabi-as -mcpu=arm926ej-s ts.S -o build/ts.o

arm-none-eabi-gcc -mcpu=arm926ej-s -marm -g -c t.c -o build/t.o
arm-none-eabi-gcc -mcpu=arm926ej-s -mthumb -g -c t.c -o build/t_thumb.o

arm-none-eabi-gcc -mcpu=arm926ej-s -mthumb -S t.c -o t_thumb.S
arm-none-eabi-gcc -mcpu=arm926ej-s -marm -S t.c -o t_arm.S

arm-none-eabi-ld build/ts.o build/t.o -T t.ld -o build/t.elf

arm-none-eabi-objcopy -O binary build/t.elf build/t.bin