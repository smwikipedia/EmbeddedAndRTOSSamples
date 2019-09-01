	.cpu arm926ej-s
	.eabi_attribute 20, 1
	.eabi_attribute 21, 1
	.eabi_attribute 23, 3
	.eabi_attribute 24, 1
	.eabi_attribute 25, 1
	.eabi_attribute 26, 1
	.eabi_attribute 30, 6
	.eabi_attribute 34, 0
	.eabi_attribute 18, 4
	.file	"t.c"
	.text
	.global	v
	.data
	.align	2
	.type	v, %object
	.size	v, 40
v:
	.word	1
	.word	2
	.word	3
	.word	4
	.word	5
	.word	6
	.word	7
	.word	8
	.word	9
	.word	10
	.comm	sum,4,4
	.text
	.align	2
	.global	strcmp
	.syntax unified
	.arm
	.fpu softvfp
	.type	strcmp, %function
strcmp:
	@ args = 0, pretend = 0, frame = 8
	@ frame_needed = 1, uses_anonymous_args = 0
	@ link register save eliminated.
	str	fp, [sp, #-4]!
	add	fp, sp, #0
	sub	sp, sp, #12
	str	r0, [fp, #-8]
	str	r1, [fp, #-12]
	nop
.L3:
	ldr	r3, [fp, #-8]
	add	r2, r3, #1
	str	r2, [fp, #-8]
	ldrb	r2, [r3]	@ zero_extendqisi2
	ldr	r3, [fp, #-12]
	add	r1, r3, #1
	str	r1, [fp, #-12]
	ldrb	r3, [r3]	@ zero_extendqisi2
	cmp	r2, r3
	bne	.L2
	ldr	r3, [fp, #-8]
	ldrb	r3, [r3]	@ zero_extendqisi2
	cmp	r3, #0
	beq	.L2
	ldr	r3, [fp, #-12]
	ldrb	r3, [r3]	@ zero_extendqisi2
	cmp	r3, #0
	bne	.L3
.L2:
	ldr	r3, [fp, #-8]
	ldrb	r3, [r3]	@ zero_extendqisi2
	cmp	r3, #0
	bne	.L4
	ldr	r3, [fp, #-12]
	ldrb	r3, [r3]	@ zero_extendqisi2
	cmp	r3, #0
	bne	.L4
	mov	r3, #0
	b	.L5
.L4:
	mov	r3, #1
.L5:
	mov	r0, r3
	add	sp, fp, #0
	@ sp needed
	ldr	fp, [sp], #4
	bx	lr
	.size	strcmp, .-strcmp
	.comm	uart,32,4
	.align	2
	.global	uart_init
	.syntax unified
	.arm
	.fpu softvfp
	.type	uart_init, %function
uart_init:
	@ args = 0, pretend = 0, frame = 8
	@ frame_needed = 1, uses_anonymous_args = 0
	@ link register save eliminated.
	str	fp, [sp, #-4]!
	add	fp, sp, #0
	sub	sp, sp, #12
	mov	r3, #0
	str	r3, [fp, #-8]
	b	.L7
.L8:
	ldr	r3, [fp, #-8]
	lsl	r3, r3, #3
	ldr	r2, .L9
	add	r3, r3, r2
	str	r3, [fp, #-12]
	ldr	r3, [fp, #-8]
	add	r3, r3, #65536
	add	r3, r3, #496
	lsl	r3, r3, #12
	mov	r2, r3
	ldr	r3, [fp, #-12]
	str	r2, [r3]
	ldr	r3, [fp, #-12]
	ldr	r2, [fp, #-8]
	str	r2, [r3, #4]
	ldr	r3, [fp, #-8]
	add	r3, r3, #1
	str	r3, [fp, #-8]
.L7:
	ldr	r3, [fp, #-8]
	cmp	r3, #3
	ble	.L8
	ldr	r3, .L9
	ldr	r2, .L9+4
	str	r2, [r3, #24]
	nop
	mov	r0, r3
	add	sp, fp, #0
	@ sp needed
	ldr	fp, [sp], #4
	bx	lr
.L10:
	.align	2
.L9:
	.word	uart
	.word	268472320
	.size	uart_init, .-uart_init
	.align	2
	.global	ugetc
	.syntax unified
	.arm
	.fpu softvfp
	.type	ugetc, %function
ugetc:
	@ args = 0, pretend = 0, frame = 8
	@ frame_needed = 1, uses_anonymous_args = 0
	@ link register save eliminated.
	str	fp, [sp, #-4]!
	add	fp, sp, #0
	sub	sp, sp, #12
	str	r0, [fp, #-8]
	nop
.L12:
	ldr	r3, [fp, #-8]
	ldr	r3, [r3]
	add	r3, r3, #24
	ldrb	r3, [r3]	@ zero_extendqisi2
	and	r3, r3, #16
	cmp	r3, #0
	bne	.L12
	ldr	r3, [fp, #-8]
	ldr	r3, [r3]
	ldrb	r3, [r3]	@ zero_extendqisi2
	mov	r0, r3
	add	sp, fp, #0
	@ sp needed
	ldr	fp, [sp], #4
	bx	lr
	.size	ugetc, .-ugetc
	.align	2
	.global	uputc
	.syntax unified
	.arm
	.fpu softvfp
	.type	uputc, %function
uputc:
	@ args = 0, pretend = 0, frame = 8
	@ frame_needed = 1, uses_anonymous_args = 0
	@ link register save eliminated.
	str	fp, [sp, #-4]!
	add	fp, sp, #0
	sub	sp, sp, #12
	str	r0, [fp, #-8]
	mov	r3, r1
	strb	r3, [fp, #-9]
	nop
.L15:
	ldr	r3, [fp, #-8]
	ldr	r3, [r3]
	add	r3, r3, #24
	ldrb	r3, [r3]	@ zero_extendqisi2
	and	r3, r3, #32
	cmp	r3, #0
	bne	.L15
	ldr	r3, [fp, #-8]
	ldr	r3, [r3]
	ldrb	r2, [fp, #-9]
	strb	r2, [r3]
	nop
	mov	r0, r3
	add	sp, fp, #0
	@ sp needed
	ldr	fp, [sp], #4
	bx	lr
	.size	uputc, .-uputc
	.align	2
	.global	upgets
	.syntax unified
	.arm
	.fpu softvfp
	.type	upgets, %function
upgets:
	@ args = 0, pretend = 0, frame = 8
	@ frame_needed = 1, uses_anonymous_args = 0
	push	{fp, lr}
	add	fp, sp, #4
	sub	sp, sp, #8
	str	r0, [fp, #-8]
	str	r1, [fp, #-12]
	b	.L17
.L18:
	ldr	r3, [fp, #-12]
	ldrb	r3, [r3]	@ zero_extendqisi2
	mov	r1, r3
	ldr	r0, [fp, #-8]
	bl	uputc
	ldr	r3, [fp, #-12]
	add	r3, r3, #1
	str	r3, [fp, #-12]
.L17:
	ldr	r0, [fp, #-8]
	bl	ugetc
	mov	r3, r0
	and	r2, r3, #255
	ldr	r3, [fp, #-12]
	strb	r2, [r3]
	ldr	r3, [fp, #-12]
	ldrb	r3, [r3]	@ zero_extendqisi2
	cmp	r3, #13
	bne	.L18
	ldr	r3, [fp, #-12]
	mov	r2, #0
	strb	r2, [r3]
	nop
	mov	r0, r3
	sub	sp, fp, #4
	@ sp needed
	pop	{fp, pc}
	.size	upgets, .-upgets
	.align	2
	.global	uprints
	.syntax unified
	.arm
	.fpu softvfp
	.type	uprints, %function
uprints:
	@ args = 0, pretend = 0, frame = 8
	@ frame_needed = 1, uses_anonymous_args = 0
	push	{fp, lr}
	add	fp, sp, #4
	sub	sp, sp, #8
	str	r0, [fp, #-8]
	str	r1, [fp, #-12]
	b	.L20
.L21:
	ldr	r3, [fp, #-12]
	add	r2, r3, #1
	str	r2, [fp, #-12]
	ldrb	r3, [r3]	@ zero_extendqisi2
	mov	r1, r3
	ldr	r0, [fp, #-8]
	bl	uputc
.L20:
	ldr	r3, [fp, #-12]
	ldrb	r3, [r3]	@ zero_extendqisi2
	cmp	r3, #0
	bne	.L21
	nop
	mov	r0, r3
	sub	sp, fp, #4
	@ sp needed
	pop	{fp, pc}
	.size	uprints, .-uprints
	.section	.rodata
	.align	2
.LC0:
	.ascii	"Enter lines from serial terminal 0\012\015\000"
	.align	2
.LC1:
	.ascii	"   \000"
	.align	2
.LC2:
	.ascii	"\012\015\000"
	.align	2
.LC3:
	.ascii	"end\000"
	.align	2
.LC4:
	.ascii	"Compute sum of array:\012\015\000"
	.align	2
.LC5:
	.ascii	"sum = \000"
	.align	2
.LC6:
	.ascii	"\012\015END OF RUN\012\015\000"
	.text
	.align	2
	.global	main
	.syntax unified
	.arm
	.fpu softvfp
	.type	main, %function
main:
	@ args = 0, pretend = 0, frame = 72
	@ frame_needed = 1, uses_anonymous_args = 0
	push	{fp, lr}
	add	fp, sp, #4
	sub	sp, sp, #72
	bl	uart_init
	ldr	r3, .L31
	str	r3, [fp, #-12]
	ldr	r1, .L31+4
	ldr	r0, [fp, #-12]
	bl	uprints
.L25:
	sub	r3, fp, #76
	mov	r1, r3
	ldr	r0, [fp, #-12]
	bl	upgets
	ldr	r1, .L31+8
	ldr	r0, [fp, #-12]
	bl	uprints
	sub	r3, fp, #76
	mov	r1, r3
	ldr	r0, [fp, #-12]
	bl	uprints
	ldr	r1, .L31+12
	ldr	r0, [fp, #-12]
	bl	uprints
	sub	r3, fp, #76
	ldr	r1, .L31+16
	mov	r0, r3
	bl	strcmp
	mov	r3, r0
	cmp	r3, #0
	beq	.L30
	b	.L25
.L30:
	nop
	ldr	r1, .L31+20
	ldr	r0, [fp, #-12]
	bl	uprints
	ldr	r3, .L31+24
	mov	r2, #0
	str	r2, [r3]
	mov	r3, #0
	str	r3, [fp, #-8]
	b	.L26
.L27:
	ldr	r2, .L31+28
	ldr	r3, [fp, #-8]
	ldr	r2, [r2, r3, lsl #2]
	ldr	r3, .L31+24
	ldr	r3, [r3]
	add	r3, r2, r3
	ldr	r2, .L31+24
	str	r3, [r2]
	ldr	r3, [fp, #-8]
	add	r3, r3, #1
	str	r3, [fp, #-8]
.L26:
	ldr	r3, [fp, #-8]
	cmp	r3, #9
	ble	.L27
	ldr	r1, .L31+32
	ldr	r0, [fp, #-12]
	bl	uprints
	ldr	r3, .L31+24
	ldr	r1, [r3]
	ldr	r0, .L31+36
	smull	r2, r3, r1, r0
	asr	r2, r3, #2
	asr	r3, r1, #31
	sub	r3, r2, r3
	and	r3, r3, #255
	add	r3, r3, #48
	and	r3, r3, #255
	mov	r1, r3
	ldr	r0, [fp, #-12]
	bl	uputc
	ldr	r3, .L31+24
	ldr	r1, [r3]
	ldr	r0, .L31+36
	smull	r2, r3, r1, r0
	asr	r2, r3, #2
	asr	r3, r1, #31
	sub	r2, r2, r3
	mov	r3, r2
	lsl	r3, r3, #2
	add	r3, r3, r2
	lsl	r3, r3, #1
	sub	r2, r1, r3
	and	r3, r2, #255
	add	r3, r3, #48
	and	r3, r3, #255
	mov	r1, r3
	ldr	r0, [fp, #-12]
	bl	uputc
	ldr	r1, .L31+40
	ldr	r0, [fp, #-12]
	bl	uprints
	mov	r3, #0
	mov	r0, r3
	sub	sp, fp, #4
	@ sp needed
	pop	{fp, pc}
.L32:
	.align	2
.L31:
	.word	uart
	.word	.LC0
	.word	.LC1
	.word	.LC2
	.word	.LC3
	.word	.LC4
	.word	sum
	.word	v
	.word	.LC5
	.word	1717986919
	.word	.LC6
	.size	main, .-main
	.ident	"GCC: (GNU Tools for Arm Embedded Processors 7-2018-q2-update) 7.3.1 20180622 (release) [ARM/embedded-7-branch revision 261907]"
