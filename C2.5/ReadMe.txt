Steps:
1. build the image
arm-none-eabi-gcc -mcpu=arm926ej-s -c t.c -o t.o
arm-none-eabi-as -mcpu=arm926ej-s  ts.S -o ts.o
arm-none-eabi-ld ts.o t.o -T t.ld -o t.elf  (the order of input files matters. ts.o must precede t.o)
arm-none-eabi-objcopy -O binary t.elf t.bin

2. run qemu
qemu-system-arm -M versatilepb -cpu arm926 -kernel t.bin -nographic -serial telnet:localhost:1234,server


Notes:
1. Do specify -mcpu for gcc and as.
2. Do specify -cpu when running qemu. 
3. For build t.o, you can also use below commands. They generate the same results.
   arm-none-eabi-gcc -S -mcpu=arm926ej-s t.c -o t.S
   
   arm-none-eabi-as t.S -o t.o
   Or:
   arm-none-eabi-as -mcpu=arm926ej-s  t.S -o t.o

3. arm926ej-s and arm926 are compatible.
4. When run qemu in step 2, if specify the telnet ip as "127.0.0.1", then with Putty or Tera Term, both "localhost" and "127.0.0.1" can work.
   If specify the telnet ip as "localhost", then with Putty or Tera Term, only "127.0.0.1" can work, even I put "127.0.0.1 localhost" in hosts file.

Running Result:

Enter lines from serial terminal 0
abcde
[You have input:] abcde
12345
[You have input:] 12345
end
[You have input:] end
Compute sum of array:
sum = 55
END OF RUN
