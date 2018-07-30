	.file	"fib.c"
	.text
	.globl	__app_roi_begin
	.type	__app_roi_begin, @function
__app_roi_begin:
.LFB5:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	nop
	popq	%rbp
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE5:
	.size	__app_roi_begin, .-__app_roi_begin
	.globl	__app_roi_end
	.type	__app_roi_end, @function
__app_roi_end:
.LFB6:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	nop
	popq	%rbp
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE6:
	.size	__app_roi_end, .-__app_roi_end
	.globl	timespec_to_ms
	.type	timespec_to_ms, @function
timespec_to_ms:
.LFB7:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	movq	%rdi, -8(%rbp)
	movq	-8(%rbp), %rax
	movq	(%rax), %rax
	cvtsi2sdq	%rax, %xmm0
	movsd	.LC0(%rip), %xmm1
	mulsd	%xmm0, %xmm1
	movq	-8(%rbp), %rax
	movq	8(%rax), %rax
	cvtsi2sdq	%rax, %xmm0
	movsd	.LC1(%rip), %xmm2
	divsd	%xmm2, %xmm0
	addsd	%xmm1, %xmm0
	popq	%rbp
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE7:
	.size	timespec_to_ms, .-timespec_to_ms
	.globl	fib_continue
	.type	fib_continue, @function
fib_continue:
.LFB8:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	movq	%rdi, -8(%rbp)
	movq	%rsi, -16(%rbp)
	movq	%rdx, -24(%rbp)
	movq	-8(%rbp), %rax
	movl	(%rax), %ecx
	movq	-16(%rbp), %rax
	movl	(%rax), %edx
	movq	-24(%rbp), %rax
	movq	(%rax), %rax
	addl	%ecx, %edx
	movl	%edx, (%rax)
	nop
	popq	%rbp
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE8:
	.size	fib_continue, .-fib_continue
	.globl	fib
	.type	fib, @function
fib:
.LFB9:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$48, %rsp
	movl	%edi, -36(%rbp)
	movq	%rsi, -48(%rbp)
	movq	%fs:40, %rax
	movq	%rax, -8(%rbp)
	xorl	%eax, %eax
	movq	-48(%rbp), %rax
	movq	%rax, -16(%rbp)
	cmpl	$1, -36(%rbp)
	jg	.L7
	movl	$0, %eax
	call	__app_roi_begin
	movq	-48(%rbp), %rax
	movl	-36(%rbp), %edx
	movl	%edx, (%rax)
	movl	$0, %eax
	call	__app_roi_end
	jmp	.L6
.L7:
	movl	-36(%rbp), %eax
	leal	-1(%rax), %edx
	leaq	-24(%rbp), %rax
	movq	%rax, %rsi
	movl	%edx, %edi
	call	fib
	movl	-36(%rbp), %eax
	leal	-2(%rax), %edx
	leaq	-20(%rbp), %rax
	movq	%rax, %rsi
	movl	%edx, %edi
	call	fib
	leaq	-16(%rbp), %rdx
	leaq	-20(%rbp), %rcx
	leaq	-24(%rbp), %rax
	movq	%rcx, %rsi
	movq	%rax, %rdi
	call	fib_continue
	nop
.L6:
	movq	-8(%rbp), %rax
	xorq	%fs:40, %rax
	je	.L10
	call	__stack_chk_fail
.L10:
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE9:
	.size	fib, .-fib
	.section	.rodata
.LC2:
	.string	"Usage: fib [workers]"
	.align 8
.LC4:
	.string	"Calculated in %.3f ns using %d workers.\n"
.LC5:
	.string	"Fibonacci number #%d is %d.\n"
	.text
	.globl	main
	.type	main, @function
main:
.LFB10:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$112, %rsp
	movl	%edi, -84(%rbp)
	movq	%rsi, -96(%rbp)
	movq	%fs:40, %rax
	movq	%rax, -8(%rbp)
	xorl	%eax, %eax
	movl	$15, -64(%rbp)
	cmpl	$1, -84(%rbp)
	jle	.L12
	movq	-96(%rbp), %rax
	addq	$8, %rax
	movq	(%rax), %rax
	movq	%rax, %rdi
	call	atoi
	testl	%eax, %eax
	jg	.L12
	movl	$.LC2, %edi
	call	puts
	movl	$1, %eax
	jmp	.L16
.L12:
	leaq	-48(%rbp), %rax
	movq	%rax, %rsi
	movl	$1, %edi
	call	clock_gettime
	movl	$0, -68(%rbp)
	jmp	.L14
.L15:
	leaq	-72(%rbp), %rdx
	movl	-64(%rbp), %eax
	movq	%rdx, %rsi
	movl	%eax, %edi
	call	fib
	addl	$1, -68(%rbp)
.L14:
	cmpl	$99999, -68(%rbp)
	jle	.L15
	leaq	-32(%rbp), %rax
	movq	%rax, %rsi
	movl	$1, %edi
	call	clock_gettime
	leaq	-32(%rbp), %rax
	movq	%rax, %rdi
	call	timespec_to_ms
	movsd	%xmm0, -104(%rbp)
	leaq	-48(%rbp), %rax
	movq	%rax, %rdi
	call	timespec_to_ms
	movsd	-104(%rbp), %xmm2
	subsd	%xmm0, %xmm2
	movapd	%xmm2, %xmm0
	movsd	%xmm0, -56(%rbp)
	movsd	-56(%rbp), %xmm0
	movsd	.LC3(%rip), %xmm1
	divsd	%xmm1, %xmm0
	movsd	.LC1(%rip), %xmm1
	mulsd	%xmm1, %xmm0
	cvtsd2ss	%xmm0, %xmm3
	movss	%xmm3, -60(%rbp)
	cvtss2sd	-60(%rbp), %xmm0
	movl	$1, %esi
	movl	$.LC4, %edi
	movl	$1, %eax
	call	printf
	movl	-72(%rbp), %edx
	movl	-64(%rbp), %eax
	movl	%eax, %esi
	movl	$.LC5, %edi
	movl	$0, %eax
	call	printf
	movl	$0, %eax
.L16:
	movq	-8(%rbp), %rcx
	xorq	%fs:40, %rcx
	je	.L17
	call	__stack_chk_fail
.L17:
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE10:
	.size	main, .-main
	.section	.rodata
	.align 8
.LC0:
	.long	0
	.long	1083129856
	.align 8
.LC1:
	.long	0
	.long	1093567616
	.align 8
.LC3:
	.long	0
	.long	1090021888
	.ident	"GCC: (Ubuntu 7.3.0-21ubuntu1~16.04) 7.3.0"
	.section	.note.GNU-stack,"",@progbits
