/* program to test record routines */
/* 	
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

#define SHORT		20
#define LONG		1000
extern char *io_error(), *bf_error(), *st_error();

#define	IOERROR(p,c)	if((int)(c)<0) \
		{printf("%s %s\n", p, io_error((int)(c)));io_final(); exit(-1);}
#define	BFERROR(p,c)	if((int)(c)<0) \
		{printf("%s %s\n", p, bf_error((int)(c)));bf_final(); io_final(); exit(-1);}

#define	ERROR(p,c)	if((int)(c)<0) printf("%s %s\n", p, st_error((int)(c)))
#define	STERROR(p,c)	if((int)(c)<0) {ERROR(p, c); st_final(); bf_final();\
				io_final();}

#define PRREC(rec) 	if (*(rec+12) == 's') printf("%20.20s\n", rec);\
			else printf("%20.20s...%20.20s\n",rec,rec +980)
#define FILENAME	"tfile35"
#define DUMPFILE	ofn = st_openfile(vol, FILENAME, READ);\
			STERROR("test35/st_openfile", ofn);\
			dumpfile_forward(ofn);\
			i = st_closefile(ofn);\
			STERROR("test35/st_closefile", i)


dumpfile_forward(ofn)
int	ofn;
{
	int	i,j;
	RID	rid1, rid2;
	char	buf[LONG];

	printf(" dump file forward with open file number %d\n", ofn);

	i = st_firstfile(ofn, &rid1);

	for(; i >= 0; rid1 = rid2)
	{	
		for (j = 0; j < LONG; buf[j++] = ' ');
		i = st_readrecord(ofn, &rid1, buf, LONG);
		STERROR("test35/st_readrecord", i);
		PRREC(buf);	/* print the record */

		i = st_nextfile(ofn, &rid1, &rid2); 
	}

}
	

main(argc,argv)
int	argc;
char	**argv;
{
	int i, j, vol, ofn;
	char rec[10][ 1001];
	RID	rid[10];

	wiss_checkflags(&argc,&argv);
	i = io_init();			/* initialize level 0 */
	IOERROR("test35/io_init", i);
	
	i = bf_initialize();		/* initialize level 1 */
	BFERROR("test35/bf_initialize", i);

	i = st_init();			/* initialize level 2 */
	STERROR("test35/st_init", i);

/* testing */

	printf(" pick up where test34 left off\n");

	vol = st_mount("wiss2");  	/* mount the device wiss2 */
	STERROR("test35/st_mount", vol);

	ofn = st_openfile(vol, FILENAME, WRITE);
	if (ofn >= 0)
		printf(" WIss Bugs!!!\n");
	else
		printf(" Good, file %s does not exist\n", FILENAME);

	i = st_createfile(vol, FILENAME, 9, 90,90);
	STERROR("test35/st_createfile", i);

	printf(" Recreate file %s again\n", FILENAME);

	printf(" There should be No records in this file !\n");
	DUMPFILE;

	i = st_destroyfile(vol, FILENAME);
	STERROR("test35/st_destroyfile", i);

	i = st_dismount("wiss2");  /* dismount the device wiss2 */
	STERROR("test35/st_dismount", i);

	printf(" If any record appeared, then Wiss has a big bug\n");

}
