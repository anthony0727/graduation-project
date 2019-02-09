#
/* test the slide routine */

#include	<wiss.h>
#include	<st.h>

#define		ERROR(a) if((int)(a) < eNOERROR) {printf("error %d\n", a), exit(-1);}

DATAPAGE	P;		/* page for our testing */

static char	a[] = /* 100 a's */
"aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa";
static char	b[] = /* 100 b's */
"bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb";
static char	abc[] = "012345678 112345678 212345678 312345678 412345678 512345678 612345678 712345678 812345678 912345678 a12345678 b12345678 c12345678 d12345678 e12345678 f12345678 g12345678 h12345678 i12345678 j12345678 ";


main(argc,argv)
int	argc;
char	**argv;
{
	RECORD		*area;		/* record area */
	register int	s;		/* slot number */
	int		e;

	/* initialize the page */
	P.thispage.Ppage  = 2;
	P.thispage.Pvolid = 63;
	P.fileid.Fvolid   = 63;
	P.fileid.Ffilenum = 15;
	P.prevpage = 1;
	P.nextpage = 3;
	P.ridcnt = 0;
	P.free = 0;
	printf("en empty page\n");
	r_dumppage(&P);

	s = 0;
	printf("removing an empty slot\n\n");
	e = r_slide(&P, 0, REMOVEREC, &area);
	if ((int) e >= eNOERROR) {
		printf("removal of non-existant record slipped by: 0x%x\n", area);
		exit(1);
	}

	s = 0;
	printf("create a record with 50 chars at slot %d\n", s);
	P.slot[-s] = EMPTYSLOT;
	P.ridcnt++;
	e = r_slide(&P, s, 50, &area);
	ERROR(e);
	area->type = NOTMOVED, area->kind = NORMAL;
	strncpy (area->data, abc, 50);
	r_dumppage(&P);

	s = 1;
	printf("\n create a record with 100 chars at slot %d\n", s);
	P.slot[-s] = EMPTYSLOT;
	P.ridcnt++;
	e = r_slide(&P, s, 100, &area);
	ERROR(e);
	area->type = NOTMOVED, area->kind = NORMAL;
	strncpy(area->data, abc, 100);
	r_dumppage(&P);

	s = 2;
	printf("\n create a record with 200 chars at slot %d\n", s);
	P.slot[-s] = EMPTYSLOT;
	P.ridcnt++;
	e = r_slide(&P, s, 200, &area);
	ERROR(e);
	area->type = NOTMOVED, area->kind = NORMAL;
	strncpy(area->data, abc, 200);
	r_dumppage(&P);

	s = 0;
	printf("\n remove slot %d\n", s);
	e = r_slide(&P, s, REMOVEREC, &area);
	ERROR(e);
	r_dumppage(&P);

	s = 0;
	printf("\n remove an empty slot\n");
	e = r_slide(&P, 0, REMOVEREC, &area);
	if ((int) e >= eNOERROR) {
		printf("removal of non-existant record slipped by: 0x%x\n", area);
		exit(1);
	}

	s = 0;
	printf("\n create another record with 100 chars at slot %d\n", s);
	e = r_slide(&P, s, 100, &area);
	ERROR(e);
	area->type = NOTMOVED, area->kind = NORMAL;
	strncpy(area->data, abc, 100);
	r_dumppage(&P);

	s = 0;
	printf("\n expand record at slot %d to 200 chard\n", s);
	e = r_slide(&P, s, 200, &area);
	ERROR(e);
	strncpy(area->data, abc, 200);
	r_dumppage(&P);

	s = 1;
	printf("\n shorten slot %d to 75\n", s);
	e = r_slide(&P, s, 75, &area);
	ERROR(e);
	strncpy(area->data, abc, 75);
	r_dumppage(&P);

	s = 1;
	printf("\n remove slot %d\n", s);
	e = r_slide(&P, s, REMOVEREC, &area);
	ERROR(e);
	r_dumppage(&P);

	s = 3;
	printf("\n create a record with 100 a's at slot %d\n", s);
	P.slot[-s] = EMPTYSLOT;
	P.ridcnt++;
	e = r_slide(&P, s, 100, &area);
	area->type = NOTMOVED, area->kind = NORMAL;
	ERROR(e);
	strncpy(area->data, a, 100);
	s = 4;
	printf("\n create a record with 100 b's at slot %d\n", s);
	P.slot[-s] = EMPTYSLOT;
	P.ridcnt++;
	e = r_slide(&P, s, 100, &area);
	ERROR(e);
	area->type = NOTMOVED, area->kind = NORMAL;
	strncpy(area->data, b, 100);
	r_dumppage(&P);

	s = 2;
	printf("\n remove slot %d\n", s);
	e = r_slide(&P, s, REMOVEREC, &area);
	ERROR(e);

	r_dumppage(&P);
	s = 3;
	printf("\n remove slot %d\n", s);
	e = r_slide(&P, s, REMOVEREC, &area);
	ERROR(e);
	r_dumppage(&P);
}
