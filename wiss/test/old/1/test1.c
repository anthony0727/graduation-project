
#include <bf.h>

#define	BUFTRACE	1

/* program to initialize wiss1 with some data */
/* Assumes wiss1 has 11 extents of 10 pages each */
/* program will create and allocate and write 5 files */
/* of 13 pages each.  */

extern int io_diskreads, io_diskwrites;
extern char *io_error(), *bf_error();

#define	IOERROR(p,c)	if((int)(c)<0) \
	{printf("%s %s\n",p,io_error((int)(c)));io_final();exit(-1);}
#define	BFERROR(p,c)	if((int)(c)<0) \
	{printf("%s %s\n",p,bf_error((int)(c)));bf_final();io_final();exit(-1);}

FID fileid[5];
PID pids[5] [20]; /* i,j: i is for file, j is for pages  */

main(argc,argv)
int	argc;
char	**argv;
{

	int e,i,j,k,volid;
	int *page;

	wiss_checkflags(&argc,&argv);

	e = io_init();  /* initialize level 0 */
	IOERROR("test1/io_init",e);
	volid = io_mount("wiss1");  /* mount the device wiss1 */
	IOERROR("test1/io_mount",volid);
	e = bf_init();
	BFERROR("test1/bf_init",e);
	bf_setbufsize(10); /* use only 10 buffers */

	printf ("\nwiss1 should have 11 extents of 10 pages each");
	printf ("\n create and allocate 5 files of 13 pages each.");
	printf ("\n\teach file will require two extents \n");
	for (j=0;j<5;j++) {
		e = io_createfile(volid, 10, 100, &fileid[j]);
		IOERROR("test1/io_createfile",e);
		e = io_allocpages(&(fileid[j]), NULL, 13, &pids[j][0]);
		IOERROR("test1/allocpages",e);
	}

	/* initialize all data pages  */
	io_clearstat();
	printf("initializing the data pages\n");
	for (j=0;j<5;j++) {
		e = bf_openfile(j, WRITE);
		BFERROR("test1/bf_openfile",e);
		for (i=0;i<13;i++) {
			e = bf_getbuf(j,&pids[j][i],&page);
			BFERROR("test1/bf_getbuf",e);
			for (k=0;k<1024;k++) page[k] = j;
			e = bf_freebuf(j,&pids[j][i],page);
		}
		e = bf_closefile(j);
		BFERROR("test1/bf_closefile",e);
	}

#ifdef	BUFTRACE
	printf("\n * after initializing the data pages\n");
	BF_dumpbuftable();
#endif

	/* flush out all the pages */
	printf("forcing all the dirty pages out\n");
	e = bf_dismount("wiss1");
	BFERROR("test1/bf_dismount",e);
	e = bf_final();
	BFERROR("test1/bf_final",e);
	printf(" %d disk reads and %d disk writes\n", 
		io_diskreads, io_diskwrites);

#ifdef	BUFTRACE
	printf("\n * the buffer should be empty\n");
	BF_dumpbuftable();
#endif

	e = io_checker(volid);
	e = io_dismount("wiss1");  /* unmount the device wiss1 */
	IOERROR("test1/io_dismount",e);

	volid = io_mount("wiss1");  /* mount the device wiss1 */
	IOERROR("test1/io_mount",volid);
	bf_init();
	bf_setbufsize(10); /* use only 10 buffers */

	io_clearstat();
	printf("\nOpen all five files in READ/WRITE\n");
	for (i=0;i<5;i++) {
		e = bf_openfile(i, WRITE);
		BFERROR("test1/bf_openfile",e);
	}

#ifdef	BUFTRACE
	printf("\n * initially, the buffer pool should be empty\n");
	BF_dumpbuftable();
#endif

	printf("check pages and update the pages to all 88's\n");
	for (i=0;i<5;i++) {
		check(i, i, i);
		update(i, i, 88);
	}

#ifdef	BUFTRACE
	printf("\n * all pages should be dirty\n");
	BF_dumpbuftable();
#endif

	/* check the modified pages (in buffer) */
	printf("check the pages in buffer\n");
	for (i=0;i<5;i++) {
		check(i, i, 88);
	}

#ifdef	BUFTRACE
	printf("\n * all pages should be dirty\n");
	BF_dumpbuftable();
#endif

	/* force the dirty pages out */
	printf("flush all the pages out\n");
	for (i=0;i<5;i++) {
		e = bf_flushbuf(i, FALSE);
		BFERROR("test1/bf_flushbuf",e);
	}

#ifdef	BUFTRACE
	printf("\n * all the buffers should be clean\n");
	BF_dumpbuftable();
#endif

	printf("check pages and update the pages to all 99's\n");
	for (i=0;i<5;i++) {
		check(i, i, 88);
		update(i, i, 99);
	}

#ifdef	BUFTRACE
	printf("\n * all pages should be dirty\n");
	BF_dumpbuftable();
#endif

	/* check the modified pages (in buffer) */
	printf("check the pages in buffer\n");
	for (i=0;i<5;i++) {
		check(i, i, 99);
	}

#ifdef	BUFTRACE
	printf("\n * all pages should be dirty\n");
	BF_dumpbuftable();
#endif

	/* force the dirty pages out */
	printf("force all the pages out, and empty the buffer pool\n");
	e = bf_dismount("wiss1");
	BFERROR("test1/bf_dismount",e);

#ifdef	BUFTRACE
	printf("\n * the buffer pool should be empty\n");
	BF_dumpbuftable();
#endif

	printf("check if all 99's, update them to 100's and discard\n");
	for (i=0;i<5;i++) {
		check(i, i, 99);
		dummy_update(i, i, 100);
	}

#ifdef	BUFTRACE
	printf("\n * the buffer pool should be empty\n");
	BF_dumpbuftable();
#endif

	/* check the pages - they should not be changed! */
	printf("check if all still 99's\n");
	for (i=0;i<5;i++) {
		check(i, i, 99);
	}

#ifdef	BUFTRACE
	printf("\n * all pages should be clean\n");
	BF_dumpbuftable();
#endif

	/* force the dirty pages out */
	printf("force all the pages out, and empty the buffer pool\n");
	e = bf_dismount("wiss1");
	BFERROR("test1/bf_dismount",e);

#ifdef	BUFTRACE
	printf("\n * the buffer pool should be empty\n");
	BF_dumpbuftable();
#endif

	printf("check if all still 99's\n");
	for (i=0;i<5;i++) {
		check(i, i, 99);
	}

#ifdef	BUFTRACE
	printf("\n * all pages should be clean\n");
	BF_dumpbuftable();
#endif

	/* check if everything ok */
	e = io_checker(volid);

	printf("\nNext close all files\n");
	for (i=0;i<5;i++) {
		e = bf_closefile(i);
		BFERROR("test1/bf_closefile",e);
	}

	e = bf_dismount("wiss1");  /* unmount the device wiss1 */
	BFERROR("test1/bf_dismount",e);

	e = bf_final();
	BFERROR("test1/bf_final",e);

	printf(" %d disk reads and %d disk writes\n", 
		io_diskreads, io_diskwrites);

	/* clean up the disk */
	for (j=0;j<5;j++) {
		e = io_destroyfile(&fileid[j]);
		IOERROR("test1/io_destroyefile",e);
	}
	e = io_final();
	BFERROR("test1/io_final",e);
}

/* update a page of a file to all "newvalue"'s */
update(filenum, pageid, newvalue)
{
	int	e, j;
	int	*bufp1, *bufp2;

	/* get the page */
	e = bf_readbuf(filenum, &pids[filenum][pageid], &bufp1);
	BFERROR("test1/bf_readbuf",e);

	/* change to all "newvalue"'s */
	bufp2 = bufp1;
	for (j=0;j<1024;j++) *bufp2++ = newvalue;
	e = bf_setdirty(filenum,&pids[filenum][pageid],bufp1);
	BFERROR("test1/bf_setdirty",e);

	/* unfix the buffer */
	e = bf_freebuf(filenum,&pids[filenum][pageid],bufp1);
	BFERROR("test1/bf_freebuf",e);
}

/* update a page of a file to all "newvalue"'s and discard the changes */
dummy_update(filenum, pageid, newvalue)
{
	int	e, j;
	int	*bufp1, *bufp2;

	/* get the page */
	e = bf_readbuf(filenum, &pids[filenum][pageid], &bufp1);
	BFERROR("test1/bf_readbuf",e);

	/* change to all "newvalue"'s */
	bufp2 = bufp1;
	for (j=0;j<1024;j++) *bufp2++ = newvalue;
	e = bf_setdirty(filenum,&pids[filenum][pageid],bufp1);
	BFERROR("test1/bf_setdirty",e);

	/* unfix the buffer */
	e = bf_discard(filenum,&pids[filenum][pageid],bufp1);
	BFERROR("test1/bf_discard",e);
}


/* check if a page of a file contains all "value"'s */
check(filenum, pageid, value)
{
	int	e, j;
	int	*bufp1, *bufp2;

	/* get the page */
	e = bf_readbuf(filenum, &pids[filenum][pageid], &bufp1);
	BFERROR("test1/bf_readbuf",e);

	/* check the content of the page */
	bufp2 = bufp1;
	for (j=0;j<1024;j++) if (*bufp2++ != value) break;
	if (j<1024) 
		printf("ERROR - page %d of file %d not full of %d's\n",
			pageid, filenum, value);
	else printf("check of page %d of file %d ok\n",pageid, filenum);

	/* unfix the buffer */
	e = bf_freebuf(filenum,&pids[filenum][pageid],bufp1);
	BFERROR("test1/bf_freebuf",e);
}

