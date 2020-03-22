arm-none-eabi-as -mcpu=arm926ej-s ts.S -o build/ts.o
arm-none-eabi-gcc -mcpu=arm926ej-s -marm -g -c t.c -o build/t.o
arm-none-eabi-gcc -mcpu=arm926ej-s -marm -g -c uart.c -o build/uart.o

arm-none-eabi-ld build/ts.o build/t.o build/uart.o -T t.ld -o build/t.elf

arm-none-eabi-objcopy -O binary build/t.elf build/t.bin

qemu-system-arm -s -M versatilepb -cpu arm926 -kernel build/t.bin -nographic -serial telnet:localhost:1122,server -S