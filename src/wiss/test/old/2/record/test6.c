/* program to test long data item routines */
/* 	
   PFF, in the following, is the page fill factor in %. 
   This factor can be used as a variable to observe the behavior of 
   space allocation/relocation scheme implemented. 
	
*/

#include <wiss.h>
#include <st.h>

extern char *io_error(), *bf_error(), *st_error();

#define	IOERROR(p,c)	if((int)(c)<0) \
	{printf("%s %s\n",p,io_error((int)(c)));io_final(); exit(-1);}
#define	BFERROR(p,c)	if((int)(c)<0) \
	{printf("%s %s\n",p,bf_error((int)(c)));bf_final();io_final();exit(-1);}
#define	ERROR(p,c)	if((int)(c)<0) printf("%s %s\n", p, st_error((int)(c)))
#define	STERROR(p,c)	if((int)(c)<0) {ERROR(p, c); st_final(); bf_final();\
				io_final();exit(-1);}

#define FILENAME	"tfile6"
#define	CELLSIZE	14
#define	PFF		75
	
main(argc,argv)
int	argc;
char	**argv;
{
	int	e;
	int 	i, j, k, vol, ofn;
	RID lrid;	
	char rec[1000];

/* initialization */

	printf(" Initializing ... \n");
	wiss_checkflags(&argc,&argv);
	e = io_init();			/* initialize level 0 */
	IOERROR("test6/io_init", e);
	e = bf_init();		/* initialize level 1 */
	BFERROR("test6/bf_init", e);
	e = st_init();			/* initialize level 2 */
	STERROR("test6/st_init", e);
	vol = st_mount("wiss2");  	/* mount the device wiss2 */
	STERROR("test6/st_mount", vol);
	e = st_createfile(vol, FILENAME, 9, PFF,PFF);
	STERROR("test6/st_createfile", e);
	ofn = st_openfile(vol, FILENAME, WRITE);
	STERROR("test6/st_openfile(for insertion)", ofn);
	e = st_createlong(ofn, &lrid);
	STERROR("test6/st_createlong", e);
#ifdef	BUFTRACE
	BF_dumpfixed();
#endif

/* 1. insert (append) 10 cells */

	printf(" About to insert (append) 10 cells \n");
	k = 0;	/* cell id */
	for (i = 0; i < 10; i++) {	
		sprintf(rec, "[cell(%2d,%3d)]", k,i);
		printf("  inserting %s into offset %d\n", rec, i*CELLSIZE);
		e = st_insertframe(ofn, &lrid,  i*CELLSIZE, rec, CELLSIZE);
		STERROR("test6/st_insertframe", e);
		r_dumplong(ofn, &lrid);
	}
#ifdef	BUFTRACE
	BF_dumpfixed();
#endif

/* 2. insert more cells into the middle */

	printf(" About to insert more cells into the middle\n");
	for (k++, j=1; j <= 8; j += j , k++) {	
		/* k : cell id, j : # of cells at a time */
		for (i = 0; i < j; i++)
			sprintf(rec + i * CELLSIZE, "[cell(%2d,%3d)]", k,i);
		printf("  inserting bytes into offset %d : %s\n", 
							j * CELLSIZE, rec);
		e = st_insertframe(ofn, &lrid, j * CELLSIZE, rec, j * CELLSIZE);
		STERROR("test6/st_insertframe", e);
		r_dumplong(ofn, &lrid);
	}
#ifdef	BUFTRACE
	BF_dumpfixed();
#endif

/* 3. read frame */
 
	printf(" About to test st_readframe \n");
	for (j = 1; j <= 8; j += j) {
		for (i = 0; i < 1000; rec[i++] = ' ');
		printf("  read %d bytes from offset %d \n",
			j * CELLSIZE, j * CELLSIZE);
		e = st_readframe(ofn, &lrid, j * CELLSIZE, rec, j * CELLSIZE);
		STERROR("test6/st_readframe", e);
	printf(" ++----------------------------------------------------++\n");
		for (i = 0, e = j * CELLSIZE; e > 0; e -= 50 , i++) 
			printf(" || %-50.50s ||\n", rec + i * 50);
	printf(" ++----------------------------------------------------++\n");
	}
#ifdef	BUFTRACE
	BF_dumpfixed();
#endif

/* 4. write frame */

	printf(" About to write frame \n");
	for (k++, j = 8; j >= 1; j /= 2 , k++) {
		for (i = 0; i < j; i++)
			sprintf(rec + i * CELLSIZE, "<cell(%2d,%3d)>", k,i);
		printf("  write %d bytes into offset %d : frame %s \n",
			j * CELLSIZE, j * CELLSIZE, rec);
		e = st_writeframe(ofn,&lrid,j * CELLSIZE,rec,j*CELLSIZE);
		STERROR("test6/st_writeframe", e);
		r_dumplong(ofn, &lrid);
	}
#ifdef	BUFTRACE
	BF_dumpfixed();
#endif

/* 5. read frame */

	printf(" About to read frames again \n");
	for (j = 1; j <= 8; j += j ) {
		for (i = 0; i < 1000; rec[i++] = ' ');
		printf("  read %d bytes from offset %d \n",
			j * CELLSIZE, j * CELLSIZE);
		e = st_readframe(ofn, &lrid, j * CELLSIZE, rec, j * CELLSIZE);
		STERROR("test6/st_readframe", e);
	printf(" ++----------------------------------------------------++\n");
		for (i = 0, e = j * CELLSIZE; e > 0; e -= 50 , i++) 
			printf(" || %-50.50s ||\n", rec + i * 50);
	printf(" ++----------------------------------------------------++\n");
	}
#ifdef	BUFTRACE
	BF_dumpfixed();
#endif

/* 6. delete cells from the middle */

	printf("About to delete cells from the middle\n");
	for (j = 8; j >= 1; j /= 2 ) {
		printf("  delete %d bytes from offset %d\n", 
			j * CELLSIZE, j * CELLSIZE);
		e = st_deleteframe(ofn, &lrid,  j * CELLSIZE, j * CELLSIZE);
		STERROR("test6/st_deleteframe", e);
		r_dumplong(ofn, &lrid);
	}
#ifdef	BUFTRACE
	BF_dumpfixed();
#endif

/* 7. compress */

	printf(" About to compress ! \n");
	e = st_compresslong(ofn, &lrid);
	STERROR("test6/st_compresslong", e);
	r_dumplong(ofn, &lrid);
#ifdef	BUFTRACE
	BF_dumpfixed();
#endif

/* 8. destory long */

	printf(" about to destroy a long data item \n");
	e = st_destroylong(ofn, &lrid);
	STERROR("test6/st_destroylong", e);
	e = st_closefile(ofn);
	STERROR("test6/st_closefile(for destroy_long", e);
#ifdef	BUFTRACE
	BF_dumpfixed();
#endif

/* finalization */

	e = st_destroyfile(vol, FILENAME);
	STERROR("test6/st_destroyfile", e);
	e = st_dismount("wiss2");  /* dismount the device wiss2 */
	STERROR("test6/st_dismount", e);
	printf("That's all folks\n");

}
