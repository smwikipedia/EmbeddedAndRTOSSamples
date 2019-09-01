ENTRY(start)
SECTIONS
{
	. = 0x10000;
	.text : {*(.text)}
	.data : {*(.data)}
	.bss : {*(.bss)}
	. = ALIGN(8);
	. = . +0x1000;
	stack_top = .;
}
