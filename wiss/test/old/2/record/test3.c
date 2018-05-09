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
	{printf("%s %s\n", p, io_error((int)(c)));io_final(); exit(-1);}
#define	BFERROR(p,c)	if((int)(c)<0) \
	{printf("%s %s\n", p, bf_error((int)(c)));bf_final(); io_final(); exit(-1);}

#define	ERROR(p,c)	if((int)(c)<0) printf("%s %s\n", p, st_error((int)(c)))
#define	STERROR(p,c)	if((int)(c)<0) {ERROR(p, c); st_final(); bf_final();\
				io_final(); exit(-1);}

#define TERROR(p,c) 	if((int)(c)<0) \
			  { printf("  wiss catched the error: "); ERROR(p,(int)(c));}\
			else printf("  %s did not catch the error !\n", p)

#define ZERROR(p,c) 	if((int)(c)<=0) \
			  { printf("  wiss catched the error\n");}\
			else printf("  %s did not catch the error !\n", p)

#define FILENAME	"tfile3"


main(argc,argv)
int	argc;
char	**argv;
{
	int i, j, vol, ofn, len;
	RID rid, rid1, rid2;	
	PID pid;
	char recadr[1000];

/* initilaization */

	len = 10;

	printf(" Initializing ... \n");
	wiss_checkflags(&argc,&argv);
	i = io_init();			/* initialize level 0 */
	IOERROR("test3/io_init", i);
	i = bf_init();		/* initialize level 1 */
	BFERROR("test3/bf_init", i);
	i = st_init();			/* initialize level 2 */
	STERROR("test3/st_init", i);
	vol = st_mount("wiss2");  	/* mount the device wiss2 */
	STERROR("test3/st_mount", vol);
	i = st_createfile(vol, FILENAME, 9, 90,90);
	STERROR("test3/st_createfile", i);
	ofn = st_openfile(vol, FILENAME, WRITE);
	STERROR("test3/st_openfile", ofn);

/* 1. reference to a non-existent file */

	printf(" Accessing a non-existing file\n");
	i = st_insertrecord(ofn + 1, recadr, len, NULL, &rid);
	TERROR("test3/st_insertrecord", i);
	i = st_appendrecord(ofn + 1, recadr, len, &rid);
	TERROR("test3/st_appendrecord", i);
	i = st_deleterecord(ofn + 1, &rid);
	TERROR("test3/st_deleterecord", i);
	i = st_readrecord(ofn + 1, &rid, recadr, len);
	TERROR("test3/st_readrecord", i);
	i = st_writerecord(ofn + 1, &rid, recadr, len);
	TERROR("test3/st_writerecord", i);

	i = st_firstfile(ofn + 1, &rid);
	TERROR("test3/st_firstfile", i);
	i = st_lastfile(ofn + 1, &rid);
	TERROR("test3/st_lastfile", i);
	i = st_nextfile(ofn + 1, &rid1, &rid2);
	TERROR("test3/st_nextfile", i);
	i = st_prevfile(ofn + 1, &rid1, &rid2);
	TERROR("test3/st_prevfile", i);
#ifdef	BUFTRACE
	BF_dumpfixed();
#endif

/* 1a. reference to a non-existent file */

	printf(" Accessing a non-existing file (a negative file #)\n");
	i = st_insertrecord(-1, recadr, len,NULL, &rid);
	TERROR("test3/st_insertrecord", i);
	i = st_appendrecord(-1, recadr, len, &rid);
	TERROR("test3/st_appendrecord", i);
	i = st_deleterecord(-1, &rid);
	TERROR("test3/st_deleterecord", i);
	i = st_readrecord(-1, &rid, recadr, len);
	TERROR("test3/st_readrecord", i);
	i = st_writerecord(-1, &rid, recadr, len);
	TERROR("test3/st_writerecord", i);

	i = st_firstfile(-1, &rid);
	TERROR("test3/st_firstfile", i);
	i = st_lastfile(-1, &rid);
	TERROR("test3/st_lastfile", i);
	i = st_nextfile(-1, &rid1, &rid2);
	TERROR("test3/st_nextfile", i);
	i = st_prevfile(-1, &rid1, &rid2);
	TERROR("test3/st_prevfile", i);
#ifdef	BUFTRACE
	BF_dumpfixed();
#endif

/* 2. NULL RID pointer */

	printf(" Testing with a null RID pointer\n");
	i = st_deleterecord(ofn, NULL);
	TERROR("test3/st_deleterecord", i);
	i = st_readrecord(ofn, NULL, recadr, len);
	TERROR("test3/st_readrecord", i);
	i = st_writerecord(ofn, NULL, recadr, len);
	TERROR("test3/st_writerecord", i);

	i = st_nextfile(ofn, NULL, NULL);
	TERROR("test3/st_nextfile", i);
	i = st_prevfile(ofn, NULL, NULL);
	TERROR("test3/st_prevfile", i);
#ifdef	BUFTRACE
	BF_dumpfixed();
#endif

/* 3. negative record length */

	printf(" Testing with a negative record length\n");
	i = st_insertrecord(ofn, recadr, -1, NULL, &rid);
	ZERROR("test3/st_insertrecord", i);
	i = st_appendrecord(ofn, recadr, -1, &rid);
	ZERROR("test3/st_appendrecord", i);
	i = st_readrecord(ofn, &rid, recadr, -1);
	ZERROR("test3/st_readrecord", i);
	i = st_writerecord(ofn, &rid, recadr, -1);
	ZERROR("test3/st_writerecord", i);
#ifdef	BUFTRACE
	BF_dumpfixed();
#endif

/* 4. NULL record pointer */

	printf(" Testing with a null record buffer pointer\n");
	i = st_insertrecord(ofn, NULL, len, NULL, &rid);
	TERROR("test3/st_insertrecord", i);
	i = st_appendrecord(ofn, NULL, len, &rid);
	TERROR("test3/st_appendrecord", i);
	i = st_readrecord(ofn, &rid, NULL, len);
	TERROR("test3/st_readrecord", i);
	i = st_writerecord(ofn, &rid, NULL, len);
	TERROR("test3/st_writerecord", i);
#ifdef	BUFTRACE
	BF_dumpfixed();
#endif

/* finalization */

	printf(" Cleaning up the mess ...\n");
	i = st_closefile(ofn);
	STERROR("test3/st_closefile", i);
	i = st_destroyfile(vol, FILENAME);
	STERROR("test3/st_destroyfile", i);
	i = st_dismount("wiss2");  /* dismount the device wiss2 */
	STERROR("test3/st_dismount", i);

}

