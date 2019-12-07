REM arm-none-eabi-gcc -S -mcpu=arm926ej-s t.c -o t1.S
REM arm-none-eabi-as t1.S -o t1.o
REM arm-none-eabi-as -mcpu=arm926ej-s  t1.S -o t5.o

arm-none-eabi-as -mcpu=arm926ej-s  ts.S -o build/ts.o

arm-none-eabi-gcc -mcpu=arm926ej-s -c t.c -o build/t.o

arm-none-eabi-ld ts.o t.o -T t.ld -o build/t.elf

arm-none-eabi-objcopy -O binary build/t.elf build/t.bin