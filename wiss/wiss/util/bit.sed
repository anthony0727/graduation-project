#_printmap:
#	.word	0xc00
#	movl	(sp),r11
#	movzbl	(r11)+,-(sp)
#	pushab	4f
#	calls	$2,_printf
#	ashl	$-3,4(sp),r10
#	jbr	2f
#1:	movzbl	(r11)+,-(sp)
#	pushab	5f
#	calls	$2,_printf
#2:	decl	r10
#	jgtr	1b
#	bicl3	$~7,4(sp),r0
#	jeql	3f
#	extzv	$0,r0,(r11),-(sp)
#	pushab	6f
#	calls	$2,_printf
#	ret
#3:	pushab	7f
#	calls	$1,_printf
#	ret
#	.data
#4:	.ascii	"{%02x\0"
#5:	.ascii	" %02x\0"
#6:	.ascii	" %02x"
#7:	.ascii	"}\0"
#	.text

s/calls	$2,_clearmap/ashl	$-3,4(sp),r0\
	movc5	$0,0,$0x00,r0,*(sp)+\
	bicl3	$~7,(sp)+,r0\
	insv	$0x00,$0,r0,(r3)/

s/calls	$2,_wsetmap/ashl	$-3,4(sp),r0\
	movc5	$0,0,$0xff,r0,*(sp)+\
	bicl3	$~7,(sp)+,r0\
	insv	$0xff,$0,r0,(r3)/

s/calls	$2,_countmap/movl	(sp)+,r5\
	ashl	$-3,(sp),r4\
	moval	0f,r3\
	clrl	r0\
	jbr	2f\
0:	.long	0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4\
1:	movzbl	(r5)+,r2\
	ashl	$-4,r2,r1\
	addl2	(r3)[r1],r0\
	bicl2	$0xf0,r2\
	addl2	(r3)[r2],r0\
2:	sobgeq	r4,1b\
	bicl3	$~7,(sp)+,r2\
	extzv	$0,r2,(r5),r2\
	ashl	$-4,r2,r1\
	addl2	(r3)[r1],r0\
	bicl2	$0xf0,r2\
	addl2	(r3)[r2],r0/

s/calls	$2,_mapnotempty/ashl	$-3,4(sp),r0\
	skpc	$0x00,r0,*(sp)+\
	jneq	1f\
	bicl3	$~7,(sp)+,r0\
	extzv	$0,r0,(r1),r0\
1:\
/

s/calls	$2,_setbit/insv	$1,4(sp),$1,*(sp)\
	addl2	$8,sp/

s/calls	$2,_clearbit/insv	$0,4(sp),$1,*(sp)\
	addl2	$8,sp/

s/calls	$2,_checkset/extzv	4(sp),$1,*(sp),r0\
	addl2	$8,sp/

s/calls	$3,_nextset/\
	addl3	$1,8(sp),r0\
	ffs	r0,$32,*(sp),r0\
	jneq	4f\
	ashl	$-3,r0,r1\
	ashl	$-3,4(sp),r0\
	subl2	r1,r0\
	jleq	2f\
	skpc	$0x00,r0,*(sp)[r1]\
	jeql	3f\
	ffs	$0,$8,(r1),r0\
1:	subl2	(sp),r1\
	ashl	$3,r1,r1\
	addl2	r1,r0\
	jbr	6f\
2:	addl2	(sp),r1\
3:	bicl3	$~7,4(sp),r0\
	jeql	5f\
	ffs	$0,r0,(r1),r0\
	jneq	1b\
	jbr	5f\
4:	cmpl	r0,4(sp)\
	jlss	6f\
5:	mcoml	$0,r0\
6:	addl2	$12,sp/

s/calls	$3,_nextclear/\
	addl3	$1,8(sp),r0\
	ffc	r0,$32,*(sp),r0\
	jneq	4f\
	ashl	$-3,r0,r1\
	ashl	$-3,4(sp),r0\
	subl2	r1,r0\
	jleq	2f\
	skpc	$0xff,r0,*(sp)[r1]\
	jeql	3f\
	ffc	$0,$8,(r1),r0\
1:	subl2	(sp),r1\
	ashl	$3,r1,r1\
	addl2	r1,r0\
	jbr	6f\
2:	addl2	(sp),r1\
3:	bicl3	$~7,4(sp),r0\
	jeql	5f\
	ffc	$0,r0,(r1),r0\
	jneq	1b\
	jbr	5f\
4:	cmpl	r0,4(sp)\
	jlss	6f\
5:	mcoml	$0,r0\
6:	addl2	$12,sp/
