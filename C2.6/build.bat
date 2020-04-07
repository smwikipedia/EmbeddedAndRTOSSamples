arm-none-eabi-objcopy -I binary -O elf32-littlearm -B arm image.bmp build/image.o

arm-none-eabi-as -mcpu=arm926ej-s ts.S -o build/ts.o
arm-none-eabi-gcc -mcpu=arm926ej-s -marm -g -c t.c -o build/t.o
arm-none-eabi-gcc -mcpu=arm926ej-s -marm -g -c uart.c -o build/uart.o

arm-none-eabi-gcc -mcpu=arm926ej-s -marm -g -c vid.c -o build/vid.o
arm-none-eabi-as -mcpu=arm926ej-s font.S -o build/font.o
arm-none-eabi-gcc -mcpu=arm926ej-s -marm -g -c char12x16.c -o build/char12x16.o

arm-none-eabi-ld build/ts.o build/t.o build/uart.o build/image.o build/vid.o build/font.o build/char12x16.o -T t.ld -o build/t.elf

arm-none-eabi-objcopy -O binary build/t.elf build/t.bin
