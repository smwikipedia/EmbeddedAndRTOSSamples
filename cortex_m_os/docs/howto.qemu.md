#### How to map multiple UARTs to PTY?
-----

Define a char device with `-chardev pty,id=myuart0`.

Bind it to a QEMU serial port with `-serial chardev:myuart0`.

To map multiple UARTs, create multiple pairs of `-chardev pty,id=myuart0 -serial chardev:myuart0`.
The UARTs will be mapped one by one.

In below example,
myuart0 binds to first instance of the board UART.
myuart1 binds to the second instance of the board UART.

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

Then use TTY clients like picocom or minicom to connec to each of them:
```
picocom /dev/pts/7
picocom /dev/pts/8
```