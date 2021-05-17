	.file	"main.c"
	.text
	.globl	_start
	.type	_start, @function
_start:
	pushl	%ebp
	movl	%esp, %ebp
.L2:
	jmp	.L2
	.size	_start, .-_start
	.ident	"GCC: (Ubuntu 4.8.5-4ubuntu2) 4.8.5"
	.section	.note.GNU-stack,"",@progbits
