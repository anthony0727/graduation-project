#
#include <bf.h>


extern char *io_error(), *bf_error();
extern int io_diskreads, io_diskwrites;

#define	IOERROR(p,c)	if((int)(c)<0) \
	{printf("%s %s\n",p,io_error((int)(c)));io_final();exit(-1);}
#define	BFERROR(p,c)	if((int)(c)<0) \
	{printf("%s %s\n",p,bf_error((int)(c)));bf_final();io_final();exit(-1);}

#define	NUMFILES	5
#define	NUMPAGES	11

main(argc,argv)
int	argc;
char	**argv;
{

	int e,i,j,k,l,m,n, volid;
	int *bufp1;
	int fnum, pnum;

	FID fileid[NUMFILES];
	int file_in_use[NUMFILES];

	PID pids[NUMFILES][NUMPAGES]; /* i,j: i is for file, j is for pages  */
	int update_count[NUMFILES][NUMPAGES];

	wiss_checkflags(&argc,&argv);

	printf("initializing the files\n");
	/* prepare for the testing */
	for (j = 0; j < NUMFILES; j++) {
		file_in_use[j] = 0;
		for (k = 0; k < NUMPAGES; k++) update_count[j][k] = 0;
	}	
	
	e = io_init();  /* initialize level 0 */
	IOERROR("test2/io_init",e);
	
	volid = io_mount("wiss1");  /* mount the device wiss1 */
	IOERROR("test2/io_mount",volid);

	for (j=0;j<NUMFILES;j++) {
		e = io_createfile(volid, 5, 100, &fileid[j]);
		IOERROR("test2/io_createfile",e);
		e = io_allocpages(&(fileid[j]), NULL, NUMPAGES, pids[j]);
		IOERROR("test2/allocpages",e);
	}

	(void) io_checker(volid);
	e = io_dismount("wiss1");  /* unmount the device wiss1 */
	IOERROR("test2/io_dismount",e);
	printf(" %d disk reads and %d disk writes\n", 
		io_diskreads, io_diskwrites);

/* grand testing of level 1 */

	volid = io_mount("wiss1");  /* mount the device wiss1 */
	IOERROR("test2/io_mount",volid);
	(void) bf_init();
	(void) bf_setbufsize(10);	/* use 10 buffers only */

	io_clearstat();
	printf("testing of level 1\n");

#ifndef	VERBOSE
	printf("please wait");
#endif

	for (n = 0; n < 1000; n++) {
#ifndef	VERBOSE
		if (n % 50 == 0) printf("\n");
		printf(".");
#endif
		/* decide which file, page to test */
		fnum = random() % NUMFILES;
		pnum = random() % NUMPAGES;

		if (!file_in_use[fnum]) {
#ifdef	VERBOSE
			printf(" open file %d\n", fnum);
#endif
			e = bf_openfile(fnum, WRITE);
			BFERROR("test2/bf_openfile",e);
			file_in_use[fnum] = 1;
		}

		if (update_count[fnum][pnum] == 0) {  /* init the page */
#ifdef	VERBOSE
			printf("initialize page %d (file %d) with %d's\n", 
				pnum, fnum, fnum);
#endif
			e = bf_getbuf(fnum, &pids[fnum][pnum], &bufp1);
			BFERROR("test2/bf_getbuf",e);
			for (j=0;j<1024;j++) bufp1[j] = fnum;
			update_count[fnum][pnum] = 1;
			e = bf_freebuf(fnum, &pids[fnum][pnum], bufp1);
			BFERROR("test2/bf_freebuf",e);
		}
		else {
			k = fnum + update_count[fnum][pnum] - 1;
#ifdef	VERBOSE
			printf("checking page %d (file %d)", pnum, fnum);
			printf(" - should be all %d's\n", k);
#endif
			/* check if the page is correct */
			e = bf_readbuf(fnum, &pids[fnum][pnum], &bufp1);
			BFERROR("test2/bf_readbuf",e);
			for (j=0;j<1024;j++) if (bufp1[j] != k) break;
			if (j<1024) {
	  	  		printf("\nERROR - page %3d:%3d (file %d) incorrect\n", 
					pids[fnum][pnum].Pvolid, 
					pids[fnum][pnum].Ppage, fnum);
				printf("it has been updated %d times\n", 
						update_count[fnum][pnum]);
				printf(" page = %d, %d, ...\n", bufp1[0], bufp1[1]);
				BF_dumpbuftable();
				exit(-1);
			}

			if (random()%2) { /* don't update */
				e = bf_freebuf(fnum, &pids[fnum][pnum], bufp1);
				BFERROR("test2/bf_freebuf",e);
				continue;
			}
				
#ifdef	VERBOSE
			printf("fill page %d (file %d) with %d's\n", 
				pnum, fnum, k+1);
#endif


			if (random()%2) { /* abandon the update */
				/* force the page to disk before update */
				e = bf_flushbuf(fnum, FALSE);
				BFERROR("test2/bf_flushbuf",e);
				/* update the page */
				for (j=0;j<1024;j++) bufp1[j] = k + 1;
				e = bf_setdirty(fnum, &pids[fnum][pnum],bufp1); 
				BFERROR("test2/bf_setdirty",e);
#ifdef	VERBOSE
				printf("abandon the changes\n");
#endif
				e = bf_discard(fnum, &pids[fnum][pnum], bufp1);
				BFERROR("test2/bf_discard",e);
			}
			else {
				/* update the page */
				for (j=0;j<1024;j++) bufp1[j] = k + 1;
				e = bf_setdirty(fnum, &pids[fnum][pnum],bufp1); 
				BFERROR("test2/bf_setdirty",e);
				update_count[fnum][pnum]++;
				e = bf_freebuf(fnum, &pids[fnum][pnum], bufp1);
				BFERROR("test2/bf_freebuf",e);
			}
			if (random() % 10==7) {
#ifdef	VERBOSE
				printf(" close file %d\n", fnum);
#endif
				e = bf_closefile(fnum);
				BFERROR("test2/bf_closefile",e);
				file_in_use[fnum] = 0;
			}
		}

	}


	printf("\n\n Update Statistics :\n");
	for (j = 0; j < NUMFILES; j++) {
		printf(" file %d:\n\t", j);
		for (k = 0; k < NUMPAGES; k++) {
			printf("page %2d, %2d time%c",
				k, update_count[j][k],
				update_count[j][k]==1?' ':'s');
			if (k % 4 == 3) printf("\n\t");
			else printf("; ");
		}
		printf("\n");
	}
	printf(" %d disk reads and %d disk writes\n", 
		io_diskreads, io_diskwrites);

				
	for (j=0; j<NUMFILES;j++) 
		if (file_in_use[j]) {
			e = bf_closefile(j);
			BFERROR("test2/bf_closefile",e);
		}

	e = bf_dismount("wiss1");  /* unmount the device wiss1 */
	BFERROR("test2/bf_dismount",e);

	(void) io_checker(volid);
	/* clean up the disk */
	for (j=0;j<5;j++) {
		e = io_destroyfile(&fileid[j]);
		IOERROR("test2/io_destroyefile",e);
	}
	(void) io_checker(volid);

	e = io_dismount("wiss1");
	IOERROR("test2/io_dismount",e);
	e = io_final();
	BFERROR("test1/io_final",e);

	printf(" All's well! \n");
	
}

