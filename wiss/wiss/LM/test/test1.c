/* test1.c */

#include "wiss.h"		/* for FID ans PID */
#include "locktype.h"		/* for graph.h */
#include "lockquiz.h"		/* for l_IS, l_X ... */
/* #include "mode.h"		/* for graph.h */
/* #include "graph.h"		/* for graph_bucket */

#define	 ctoi(c)		(c - '0')
#define  itoc(i)		(i + '0')

#define  PAGE_PER_FILE		3
#define  MAXFILE		3

extern activate();
extern abort_trans();
extern committ_trans();
extern lock_file();
extern m_release_file();
extern lock_page();
extern m_release_page();

/* to test server_lm.o without the rpc connection */
reply_to_waiter(clientNo, transId, action) 
int clientNo, transId, action;  /* action is either GRANTED or ABORTED */
{
  printf("\n        (call to function reply_to_waiter ==> nop) \n");
}

/*** transaction number ***/
which_trans()
{
	char trans;

	printf("\n Which transaction (0/1/2) ? ");
	new_sgtty();
	while (!strchr("012", (trans = getchar())));
	old_sgtty();
	putchar(trans);
	putchar('\n');
	return(ctoi(trans));
} /* end which_trans() */

/*** file id ***/
FID which_file()
{
	char	file;
	FID	fid;

	fid.Fvolid = 1989;

	printf("\n Which file (0 to %d) ? ", MAXFILE -1);
	new_sgtty();
	while (((file = getchar()) < itoc(0)) || (file > itoc(MAXFILE -1)));
	old_sgtty();
	putchar(file);
	putchar('\n');

	fid.Ffilenum = ctoi(file);
	return(fid);
} /* end which_file() */

/*** page id ***/
PID which_page(lb, ub)
     int 	lb, ub;		/* lower/upper bound */
{
	char	page;
	PID	pid;

	pid.Pvolid = 1989;
	
	printf("\n Which page (%d to %d) ? ", lb, ub);
	new_sgtty();
	while (((page = getchar()) < itoc(lb)) || (page > itoc(ub)));
	old_sgtty();
	putchar(page);
	putchar('\n');

	pid.Ppage = ctoi(page);
	return(pid);
} /* end which_page() */

/* mode */
LOCKTYPE which_mode()
{
	char	mode;

	printf("\n Which mode (IS->%d, IX->%d, S->%d, X->%d) ? ",
		l_IS, l_IX, l_S, l_X);
	new_sgtty();
	while (!strchr("1235", (mode = getchar())));
	old_sgtty();
	putchar(mode);
	putchar('\n');
	return((LOCKTYPE) ctoi(mode));
} /* end which_mode() */

/* lock duration */
DURATION which_dur()
{
	char  	dur;

	printf("\n Which duration (INSTANT->i, MANUAL->m, COMMIT->c) ? ");
	new_sgtty();
	while (!strchr("imc", (dur = getchar())));
	old_sgtty();
	putchar(dur);
	putchar('\n');
	if (dur == 'i')
	  return(DURATION)INSTANT;
	else if (dur == 'm')
	  return(DURATION)MANUAL;
	else return(DURATION)COMMIT;
} /* end which_dur() */

/* conditionnal lock */
short which_cond()
{
	char	cond;

	printf("\n Conditionnal lock (TRUE->t, FALSE->f) ? ");
	new_sgtty();
	while (!strchr("tf", (cond = getchar())));
	old_sgtty();
	putchar(cond);
	putchar('\n');
	if (cond == 't')
	  return(short)TRUE;
	else
	  return(short)FALSE;
} /* end which_cond() */


/*** routine calling lock_file() ***/
r_lock_file()
{
	int 	trans;
	FID	 file;
	LOCKTYPE mode;
	DURATION dur;
	short  cond;
	int	 status;

	trans = which_trans();
	file = which_file();
	mode = which_mode();
	dur = which_dur();
	cond = which_cond();

	status = lock_file(trans, file, mode, dur, cond);
	printf("--> %d\n", status);
}

/*** routine calling m_release_file() ***/
r_release_file()
{
	int 	trans;
	FID	 file;
	int	 status;

	trans = which_trans();
	file = which_file();

	status = m_release_file(trans, file);
	printf("--> %d\n", status);
}

/*** routine calling lock_page() ***/
r_lock_page()
{
	int 	trans;
	FID	file;
	PID	 page;
	LOCKTYPE mode;
	DURATION dur;
	short  cond;
	int	 status;

	trans = which_trans();
	file = which_file();
	page = which_page(file.Ffilenum * PAGE_PER_FILE, 
			  (file.Ffilenum +1) * PAGE_PER_FILE -1);
	mode  = which_mode();
	dur = which_dur();
	cond = which_cond();

	status = lock_page(trans, file, page, mode, dur, cond);
	printf("--> %d\n", status);
}

/*** routine calling m_release_page() ***/
r_release_page()
{
	int 	 trans;
	PID	 page;
	int	 status;

	trans = which_trans();
	page = which_page(0, MAXFILE*PAGE_PER_FILE -1);

	status = m_release_page(trans, page);
	printf("--> %d\n", status);
}

/*** routine calling signal_file_destroy() ***/
r_signal_file_destroy()
{
	FID	 file;
	int	 status;

	file = which_file(0, MAXFILE -1);

	status = signal_file_destroy(file);
	printf("--> %d\n", status);
}


main()
{

	int i;
	int	status;			/* for returned flags */
	char	run_option;		/* the number of the primitive to run */

	printf(" START: Driver for lock manager ...\n");
	initialize_resources();
	init_sgttyb();

	for(; ;){
		/* make the user choose the lockmanager primitive to run */
		printf("\n Choose an operation :\n");
		printf("\t [q] quit\n");
		printf("\t [o] open transaction\n");
		printf("\t [a] abort transaction\n");
		printf("\t [c] commit transaction\n");
		printf("\t   [L] lock a file\n");
		printf("\t   [R] release a file lock\n");
		printf("\t   [l] lock a page\n");
		printf("\t   [r] release a page lock\n");
		printf("\t   [s] signal a destroyed file\n");
		printf("\t [t] state of a transaction \n");
		printf("\t [f] state of a file\n");
		printf("\t [p] state of a page\n");
		printf("\n Please, your choice ?  ");

		new_sgtty();
		while (!strchr("qQoOaAcClLrRsStTfFpP", (run_option = getchar())));
		old_sgtty();
		putchar(run_option);
		if (run_option == 'q' || run_option == 'Q')
			break;

		putchar('\n');

		switch(run_option){
		case 'o':
		case 'O':
			activate(which_trans());
			break;
		case 'a':
		case 'A':
			abort_trans(which_trans());
			break;
		case 'c':
		case 'C':
			committ_trans(which_trans());
			break;
		case 'L':
			r_lock_file();
			break;
		case 'R':
			r_release_file();
			break;
		case 'l':
			r_lock_page();
			break;
		case 'r':
			r_release_page();
			break;
		case 's':
		case 'S':
			r_signal_file_destroy();
			break;
		case 't':
		case 'T':
			trans_state(which_trans());
			break;
		case 'f':
		case 'F':
			file_state(which_file());
			break;
		case 'p':
		case 'P':
			page_state(which_page(0, MAXFILE*PAGE_PER_FILE -1));
			break;
		} /* end switch */
	} /* end for */

	printf("\n\n ...TERMINATED run\n");
}
