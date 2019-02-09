/* program to test record routines */
/* 	
	st_insertrecord
	st_deleterecord
	st_appendrecord
	st_readrecord
	st_writerecord
	
	st_firstfile
	st_lastfile
	st_nextfile
	st_prevfile
*/

#include <wiss.h>

#define	BUFTRACE	1

extern char *io_error(), *bf_error(), *st_error();

#define	IOERROR(p,c)	if((int)(c)<0) \
	{printf("%s %s\n",p,io_error((int)(c)));io_final();exit(-1);}
#define	BFERROR(p,c)	if((int)(c)<0) \
	{printf("%s %s\n",p,bf_error((int)(c)));bf_final();io_final();exit(-1);}

#define	ERROR(p,c)	if((int)(c)<0) printf("%s - %s\n",p,st_error((int)(c)))
#define	STERROR(p,c)	if((int)(c)<0) {ERROR(p,c);st_final();bf_final();\
				io_final();exit(-1);}
#define TERROR(p,c) 	if((int)(c)<0) \
			  { printf("  Error catched: "); ERROR(p,(int)(c));}\
			else printf(" %s did not catch the error !\n", p)

#define FILENAME	"tfile4"
#define FILENAMEA       "tfile4a"

main(argc,argv)
int	argc;
char	**argv;
{
	int  i, vol, ofn, ofna, len;
	RID rid1,rid2;	
	RID rid,rida;	
	PID pid;
	char recadr[15];

/* Initialization */
/* Create two files: one is empty, and the other contains one record. */

	printf(" Initializing\n");
	sprintf(recadr, "[test record]");
	len = 15;
	wiss_checkflags(&argc,&argv);
	i = io_init();			/* initialize level 0 */
	IOERROR("test4/io_init", i);
	i = bf_init();		/* initialize level 1 */
	BFERROR("test4/bf_init", i);
	i = st_init();			/* initialize level 2 */
	STERROR("test4/st_init", i);
	vol = st_mount("wiss2");  	/* mount the device wiss2 */
	STERROR("test4/st_mount", vol);
#ifdef	BUFTRACE
	BF_dumpfixed();
#endif

	printf(" Create an empty file and another file with one record\n");
	i = st_createfile(vol, FILENAME, 9, 90,90);
	STERROR("test4/st_createfile", i);
	i = st_createfile(vol, FILENAMEA , 9, 90,90);
	STERROR("test4/st_createfile", i);
	ofna = st_openfile(vol, FILENAMEA, WRITE);
	STERROR("test4/st_openfile", ofna);
	i = st_insertrecord(ofna, recadr, len, NULL, &rida);
	STERROR("test4/st_insertrecord", i);
	i = st_closefile(ofna);
	STERROR("test4/st_closefile", i);
#ifdef	BUFTRACE
	BF_dumpfixed();
#endif

/* 1. update a read-only file */

	printf(" Updating a read-only file\n");
	ofn = st_openfile(vol, FILENAME, READ);
	STERROR("test4/st_openfile", ofn);

	i = st_insertrecord(ofn, recadr, len, NULL, &rid);
	TERROR("test4/st_insertrecord", i);
	i = st_appendrecord(ofn, recadr, len, &rid);
	TERROR("test4/st_appendrecord", i);
	i = st_deleterecord(ofn, &rid);
	TERROR("test4/st_deleterecord", i);
	i = st_writerecord(ofn, &rid, recadr, len);
	TERROR("test4/st_writerecord", i);
	i = st_closefile(ofn);
	STERROR("test4/st_closefile", i);
#ifdef	BUFTRACE
	BF_dumpfixed();
#endif

/* 2. testing error detection in insert, delete, read, write */

	ofn = st_openfile(vol, FILENAME, WRITE);
	STERROR("test4/st_openfile", ofn);

	/* use pid form other file as hint to insertrecord */
	printf(" Give a bad location hint to insertrecord\n");
	i = st_insertrecord(ofn, recadr, len, &rida, &rid);
	TERROR("test4/st_insertrecord", i);
#ifdef	BUFTRACE
	BF_dumpfixed();
#endif

	/* delete a record twice */
	i = st_insertrecord(ofn, recadr, len, NULL, &rid);
	STERROR("test4/st_insertrecord", i);
	i = st_deleterecord(ofn, &rid);
	STERROR("test4/st_deleterecord", i);
	printf(" Delete a non-existing record\n");
	i = st_deleterecord(ofn, &rid);
	TERROR("test4/st_deleterecord", i);
#ifdef	BUFTRACE
	BF_dumpfixed();
#endif

	/* write to a non-existent record */
	printf(" Write to a non-existing record\n");
	i = st_writerecord(ofn, &rid, recadr, len);
	TERROR("test4/st_writerecord", i);
#ifdef	BUFTRACE
	BF_dumpfixed();
#endif

	/* read from a non-existent record */
	printf(" Read a non-existing record\n");
	i = st_readrecord(ofn, &rid, recadr, len);
	TERROR("test4/st_readrecord", i);
#ifdef	BUFTRACE
	BF_dumpfixed();
#endif

	i = st_closefile(ofn);
	STERROR("test4/st_closefile", i);
#ifdef	BUFTRACE
	BF_dumpfixed();
#endif

/* 3. testing firstfile, lastfile, ... */

	ofn = st_openfile(vol, FILENAME, READ);
	STERROR("test4/st_openfile", ofn);
	ofna = st_openfile(vol, FILENAMEA, READ);
	STERROR("test4/st_openfile", ofna);

	/* "FILENAME" is a empty file, "FILENAMEA" has one record */

	printf(" Testing st_firstfile st_lastfile st_nextfile st_prevfile\n");

	/* get the first record of an empty file */
	printf(" Retrieve the first record of an empty file\n");
	i = st_firstfile(ofn, &rid);
	TERROR("test4/st_firstfile", i);
#ifdef	BUFTRACE
	BF_dumpfixed();
#endif

	/* get the last record of an empty file */
	printf(" Retrieve the last record of an empty file\n");
	i = st_lastfile(ofn, &rid);
	TERROR("test4/st_lastfile", i);
#ifdef	BUFTRACE
	BF_dumpfixed();
#endif

	/* get "the previous" of the first */
	i = st_firstfile(ofna, &rid1);
	STERROR("test4/st_firstfile", i);
	printf(" Retrieve the PREVIOUS of the first record\n");
	i = st_prevfile(ofna, &rid1, &rid2);
	TERROR("test4/st_prevfile", i);
#ifdef	BUFTRACE
	BF_dumpfixed();
#endif

	/* get "the next" of the last */
	printf(" Retrieve the NEXT of the last record\n");
	i = st_lastfile(ofna, &rid1);
	STERROR("test4/st_lastfile", i);
	i = st_nextfile(ofna, &rid1, &rid2);
	TERROR("test4/st_nextfile", i);
#ifdef	BUFTRACE
	BF_dumpfixed();
#endif

	i = st_closefile(ofn);
	STERROR("test4/st_closefile", i);
	i = st_closefile(ofna);
	STERROR("test4/st_closefile", i);
#ifdef	BUFTRACE
	BF_dumpfixed();
#endif

/* clean up */

	printf(" Cleaning up the mess ...\n");
	i = st_destroyfile(vol, FILENAME);
	STERROR("test4/st_destroyfile", i);
	i = st_destroyfile(vol, FILENAMEA);
	STERROR("test4/st_destroyfile", i);
	i = st_dismount("wiss2");  /* dismount the device wiss2 */
	STERROR("test4/st_dismount", i);
}

