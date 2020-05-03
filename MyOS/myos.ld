ENTRY(reset_handler) /* OK to remove, seems no use */
SECTIONS
{
	. = 0x10000; /* loading address */
	.text : {*(.text)}
	.data : {*(.data)}
	.bss : {*(.bss)}
	.data : {*(image.o)}

	. = ALIGN(8);
	. = . + 0x1000; /* 4KB of SVC stack */
    svc_stack_top = .;
	. = . + 0x1000; /* 4KB of IRQ stack */
    irq_stack_top = .;
}
