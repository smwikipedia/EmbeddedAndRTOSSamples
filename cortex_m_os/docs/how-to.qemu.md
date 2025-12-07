#### How to map multiple UARTs to PTY?
-----

Define a char device with `-chardev pty,id=myuart0`.

Bind it to a QEMU serial port with `-serial chardev:myuart0`.

To map multiple UARTs, create multiple pairs of `-chardev pty,id=myuart0 -serial chardev:myuart0`.
The UARTs will be mapped one by one.

In below example,
myuart0 binds to first instance of the board UART.
myuart1 binds to the second instance of the board UART.

And if you want to bind to N UARTs, you may need to create N pairs.
Or you can use `-serial none` to skip the one you don't need.

```
QEMU_CMD_DEBUG = $(QEMU_ARM) -s -S -M $(QEMU_BOARD_NAME) -cpu $(QEMU_CPU) -m 16 \
					-chardev pty,id=myuart0 -serial chardev:myuart0 \
					-chardev pty,id=myuart1 -serial chardev:myuart1 \
					-device loader,file=$(OS_BIN)
```


After launching the QEMU, you should see below output:

```
char device redirected to /dev/pts/7 (label myuart0)
char device redirected to /dev/pts/8 (label myuart1)
```

Then use TTY clients like picocom or minicom to connect to each of them:
```
picocom /dev/pts/7
picocom /dev/pts/8
```


#### How to map multiple UARTs to tcp?
-----
Similar to PTY mapping, create multiple `-serial tcp:127.0.0.1:1124`.

Example:
```
QEMU_CMD_DEBUG = $(QEMU_ARM) -s -S -M $(QEMU_BOARD_NAME) -cpu $(QEMU_CPU) -m 16 \
					-serial tcp:127.0.0.1:1124,server \
					-serial tcp:127.0.0.1:1125,server \
					-device loader,file=$(OS_BIN)
```

Note that you must use telnet to connect to 127.0.0.1:1124 before 127.0.0.1:1125 is connectable.
Because 1124 is specified before 1125.
And in GDB, you must connect to both 1124 and 1125 before you can run `target remote:1234` to connect
to the gdb server.

And to skip a certain UART instance, specify `-serial none`.



#### Difference between tcp and telnet mapping of QEMU UART
The telnet mapping:
-serial telnet:127.0.0.1:1124

The tcp mapping:
-serial tcp:127.0.0.1:1124

With telnet mapping, any key press will be sent to QEMU uart immediately. QEMU uart will only receive 1 char.
Telnet won't display the input char unless the QEMU uart echo it back.

With tcp mapping, the data is sent to QEMU uart only after "enter" is pressed in telnet app. QEMU uart will receive all the chars pluss the "\r\n".
Telnet app will display the input char itself. It doesn't need the QEMU uart to echo it back.
If QEMU uart echo it back, telnet will print it again.

It seems the telnet app applies some line discipline for the QEMU uart tcp mapping.
