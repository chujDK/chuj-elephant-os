	.file	"main.c"
	.text
	.globl	_start
	.type	_start, @function
_start:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$16, %esp
	movl	$0, -4(%ebp)
.L2:
	addl	$1, -4(%ebp)
#APP
# 7 "main.c" 1
	movl $'K',%gs:240
# 0 "" 2
#NO_APP
	jmp	.L2
	.size	_start, .-_start
	.ident	"GCC: (Ubuntu 4.8.5-4ubuntu2) 4.8.5"
	.section	.note.GNU-stack,"",@progbits
