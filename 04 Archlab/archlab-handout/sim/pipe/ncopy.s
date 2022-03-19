	.file	"ncopy.c"
	.text
	.globl	ncopy
	.type	ncopy, @function
ncopy:
.LFB23:
	.cfi_startproc
	endbr64
	leaq	-1(%rdx), %r11		# r11 = limit = len - 1
	movl	$0, %eax			# count = 0
	movl	$0, %r9d
	jmp	.L2
.L4:
	addq	$1, %rax
.L2:
	cmpq	%r11, %rax
	jge	.L6 				# if (0 >= limit)
	movq	%rax, %r10
	leaq	0(,%rax,8), %r8
	movq	(%rdi,%r8), %rcx
	movq	%rcx, (%rsi,%r8)
	testq	%rcx, %rcx
	jle	.L3
	addq	$1, %r9
.L3:
	leaq	8(,%r10,8), %r8
	movq	(%rdi,%r8), %rcx
	movq	%rcx, (%rsi,%r8)
	testq	%rcx, %rcx
	jle	.L4
	addq	$1, %r9
	jmp	.L4
.L8:
	leaq	0(,%rax,8), %r8
	movq	(%rdi,%r8), %rcx
	movq	%rcx, (%rsi,%r8)
	testq	%rcx, %rcx
	jle	.L7
	addq	$1, %r9
.L7:
	addq	$1, %rax
.L6:
	cmpq	%rdx, %rax  # if (rax < rdx)
	jl	.L8
	movq	%r9, %rax
	ret
	.cfi_endproc
.LFE23:
	.size	ncopy, .-ncopy
	.globl	main
	.type	main, @function
main:
.LFB24:
	.cfi_startproc
	endbr64
	movl	$0, %eax
	ret
	.cfi_endproc
.LFE24:
	.size	main, .-main
	.comm	dst,64,32
	.comm	src,64,32
	.ident	"GCC: (Ubuntu 9.4.0-1ubuntu1~20.04) 9.4.0"
	.section	.note.GNU-stack,"",@progbits
	.section	.note.gnu.property,"a"
	.align 8
	.long	 1f - 0f
	.long	 4f - 1f
	.long	 5
0:
	.string	 "GNU"
1:
	.align 8
	.long	 0xc0000002
	.long	 3f - 2f
2:
	.long	 0x3
3:
	.align 8
4:
