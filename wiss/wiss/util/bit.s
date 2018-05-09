
/********************************************************/
/*                                                      */
/*               WiSS Storage System                    */
/*          Version SystemV-4.0, September 1990	        */
/*                                                      */
/*              COPYRIGHT (C) 1990                      */
/*                David J. DeWitt 		        */
/*               Madison, WI U.S.A.                     */
/*                                                      */
/*	         ALL RIGHTS RESERVED                    */
/*                                                      */
/********************************************************/


/*
    This is improved version of "bit.c" in "db:~wiss/src/util".
    The performance improvements are significant.
    All routines handle bitmap ends correctly, including sizes not
    divisible by 8.
    The size of the bit maps are only limited by available space.
    The algorithms for "countmap" and "nextset"/"nextclear" have been
    substituted by more efficient ones which process more than one bit
    at a time.
    The format for printing has been shortened.

    To profile these routines with "prof" or "gprof", remove the #'s
    at the beginning of the six lines after the entry points.
*/

	.globl	_printmap
	.globl	_clearmap
	.globl	_wsetmap
	.globl	_countmap
	.globl	_mapnotempty
	.globl	_setbit
	.globl	_clearbit
	.globl	_checkset
	.globl	_nextset
	.globl	_nextclear

	.text

	.align	1
_printmap:
	.word	0xc00
	.data
	.align	2
0:	.long	0
	.text
	moval	0b,r0
	jsb	mcount
	movl	4(ap),r11
	movzbl	(r11)+,-(sp)	# first byte word
	pushab	4f
	calls	$2,_printf
	ashl	$-3,8(ap),r10	# byte counter
	jbr	2f
1:	movzbl	(r11)+,-(sp)
	pushab	5f
	calls	$2,_printf
2:	decl	r10
	jgtr	1b
	bicl3	$~7,8(ap),r0	# bits in last byte
	jeql	3f
	extzv	$0,r0,(r11),-(sp)
	pushab	6f
	calls	$2,_printf
	ret
3:	pushab	7f
	calls	$1,_printf
	ret
	.data
4:	.ascii	"{%02x\0"
5:	.ascii	" %02x\0"
6:	.ascii	" %02x"
7:	.ascii	"}\0"
	.text

	.align	1
_clearmap:
	.word	0x800
	.data
	.align	2
0:	.long	0
	.text
	moval	0b,r0
	jsb	mcount
	movl	4(ap),r3
	ashl	$-18,8(ap),r11
	jbr	2f
1:	movc5	$0,0,$0x00,$0x8000,(r3)
2:	sobgeq	r11,1b
	extzv	$3,$15,8(ap),r0
	movc5	$0,0,$0x00,r0,(r3)
	bicl3	$~7,8(ap),r0
	insv	$0x00,$0,r0,(r3)
	ret

	.align	1
_wsetmap:
	.word	0
	.data
	.align	2
0:	.long	0
	.text
	moval	0b,r0
	jsb	mcount
	movl	4(ap),r3
	ashl	$-18,8(ap),r11
	jbr	2f
1:	movc5	$0,0,$0xff,$0x8000,(r3)
2:	sobgeq	r11,1b
	extzv	$3,$15,8(ap),r0
	movc5	$0,0,$0xff,r0,(r3)
	bicl3	$~7,8(ap),r0
	insv	$0xff,$0,r0,(r3)
	ret

	.align	1
_countmap:
	.word	0
	.data
	.align	2
0:	.long	0
	.text
	moval	0b,r0
	jsb	mcount
	movl	4(ap),r5	# pointer in bitmap
	ashl	$-3,8(ap),r4	# byte counter
	moval	0f,r3		# address of table
	clrl	r0		# accumulator for count
	jbr	2f
0:	.long	0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4
1:	movzbl	(r5)+,r2	# current byte
	ashl	$-4,r2,r1	# left half byte
	addl2	(r3)[r1],r0
	bicl2	$0xf0,r2	# right half byte
	addl2	(r3)[r2],r0
2:	sobgeq	r4,1b
	bicl3	$~7,8(ap),r2	# bits in last byte
	extzv	$0,r2,(r5),r2
	ashl	$-4,r2,r1	# left half byte
	addl2	(r3)[r1],r0
	bicl2	$0xf0,r2	# right half byte
	addl2	(r3)[r2],r0
	ret

	.align	1
_mapnotempty:
	.word	0
	.data
	.align	2
0:	.long	0
	.text
	moval	0b,r0
	jsb	mcount
	movl	4(ap),r1
	ashl	$-18,8(ap),r2
	jbr	2f
1:	skpc	$0x00,$0x8000,(r1)
	jneq	3f
2:	sobgeq	r2,1b
	extzv	$3,$15,8(ap),r0
	skpc	$0x00,r0,(r1)
	jneq	3f
	bicl3	$~7,8(ap),r0
	extzv	$0,r0,(r1),r0
3:	ret

	.align	1
_setbit:
	.word	0
	.data
	.align	2
0:	.long	0
	.text
	moval	0b,r0
	jsb	mcount
	insv	$1,8(ap),$1,*4(ap)
	ret

	.align	1
_clearbit:
	.word	0
	.data
	.align	2
0:	.long	0
	.text
	moval	0b,r0
	jsb	mcount
	insv	$0,8(ap),$1,*4(ap)
	ret

	.align	1
_checkset:
	.word	0
	.data
	.align	2
0:	.long	0
	.text
	moval	0b,r0
	jsb	mcount
	extzv	8(ap),$1,*4(ap),r0
	ret

	.align	1
_nextset:
	.word	0
	.data
	.align	2
0:	.long	0
	.text
	moval	0b,r0
	jsb	mcount
	/* put arguments into registers */
	movl	4(ap),r5
	movl	8(ap),r4
	addl3	$1,12(ap),r0
	/* calculate size of range */
	subl3	r0,r4,r1
	jleq	1f
	cmpl	r1,$32
	jgtr	3f
	/* use "ffs" once */
	ffs	r0,r1,(r5),r0
	jneq	2f
1:	mnegl	$1,r0
2:	ret
	/* try first part */
3:	mnegl	r0,r1
	bicl2	$~7,r1
	addl2	$24,r1
	ffs	r0,r1,(r5),r0
	jneq	2b
	/* calculate start and length(s) for "skpc" */
	ashl	$-3,r0,r1
	ashl	$-3,r4,r2
	subl2	r1,r2
	ashl	$-15,r2,r3
	bicl2	$~0x7fff,r2
	addl2	r5,r1
	/* try to find a byte != 00 */
	jbr	5f
4:	skpc	$0x00,$0x8000,(r1)
	jneq	6f
5:	sobgeq	r3,4b
	skpc	$0x00,r2,(r1)
	jeql	8f
	/* find a "0" bit in the byte found */
6:	ffs	$0,$8,(r1),r0
	/* calculate bit offset */
7:	subl2	r5,r1
	ashl	$3,r1,r1
	addl2	r1,r0
	ret
	/* check the last byte */
8:	bicl3	$~7,r4,r2
	jeql	9f
	ffs	$0,r2,(r1),r0
	jneq	7b
9:	mnegl	$1,r0
	ret

	.align	1
_nextclear:
	.word	0
	.data
	.align	2
0:	.long	0
	.text
	moval	0b,r0
	jsb	mcount
	/* put arguments into registers */
	movl	4(ap),r5
	movl	8(ap),r4
	addl3	$1,12(ap),r0
	/* calculate size of range */
	subl3	r0,r4,r1
	jleq	1f
	cmpl	r1,$32
	jgtr	3f
	/* use "ffc" once */
	ffc	r0,r1,(r5),r0
	jneq	2f
1:	mnegl	$1,r0
2:	ret
	/* try first part */
3:	mnegl	r0,r1
	bicl2	$~7,r1
	addl2	$24,r1
	ffc	r0,r1,(r5),r0
	jneq	2b
	/* calculate start and length(s) for "skpc" */
	ashl	$-3,r0,r1
	ashl	$-3,r4,r2
	subl2	r1,r2
	ashl	$-15,r2,r3
	bicl2	$~0x7fff,r2
	addl2	r5,r1
	/* try to find a byte != ff */
	jbr	5f
4:	skpc	$0xff,$0x8000,(r1)
	jneq	6f
5:	sobgeq	r3,4b
	skpc	$0xff,r2,(r1)
	jeql	8f
	/* find a "0" bit in the byte found */
6:	ffc	$0,$8,(r1),r0
	/* calculate bit offset */
7:	subl2	r5,r1
	ashl	$3,r1,r1
	addl2	r1,r0
	ret
	/* check the last byte */
8:	bicl3	$~7,r4,r2
	jeql	9f
	ffc	$0,r2,(r1),r0
	jneq	7b
9:	mnegl	$1,r0
	ret
