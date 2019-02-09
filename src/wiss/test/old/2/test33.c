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
#define FILENAME	"tfile10"
#define DUMPFILE	ofn = st_openfile(vol, FILENAME, READ);\
			STERROR("test33/st_openfile", ofn);\
			dumpfile_forward(ofn);\
			i = st_closefile(ofn);\
			STERROR("test33/st_closefile", i)


dumpfile_forward(ofn)
int	ofn;
{
	int	i,j;
	RID	rid1, rid2;
	char	buf[LONG];

	printf(" dump file forward with open file number %d\n", ofn);

	i = st_firstfile(ofn, &rid1);
	STERROR("test33/st_firstfile", i);

	for(; i >= 0; rid1 = rid2)
	{	
		for (j = 0; j < LONG; buf[j++] = ' ');
		i = st_readrecord(ofn, &rid1, buf, LONG);
		STERROR("test33/st_readrecord", i);
		PRREC(buf);	/* print the record */

		i = st_nextfile(ofn, &rid1, &rid2); 
	}

	ERROR("test33/st_nextfile", i);

}
	

main(argc,argv)
int	argc;
char	**argv;
{
	int i, j, vol, ofn;
	char rec[10][ 1001];
	RID	rid[10];

/* initilaize test records */

	for (i = 0; i < 10; i++)
	{
		sprintf(rec[i], "[record %d] (short)   ", i);
		for (j = SHORT; j < LONG-SHORT; j++)
			rec[i][j] = i + '0';
		sprintf(&rec[i][j], "   [end of record %d]", i);
	}

/* other initialization */

	wiss_checkflags(&argc,&argv);
	i = io_init();			/* initialize level 0 */
	IOERROR("test33/io_init", i);
	
	i = bf_initialize();		/* initialize level 1 */
	BFERROR("test33/bf_initialize", i);

	i = st_init();			/* initialize level 2 */
	STERROR("test33/st_init", i);

/* testing */

	printf(" pick up where test32 left off\n");
	vol = st_mount("wiss2");  	/* mount the device wiss2 */
	STERROR("test33/st_mount", vol);

	DUMPFILE;

	i = st_dismount("wiss2");  /* dismount the device wiss2 */
	STERROR("test33/st_dismount", i);

/* 3. append 5 short records */
	printf("about to append 5 short records : 0,2,4,6,8\n");

	vol = st_mount("wiss2");  	/* mount the device wiss2 */
	STERROR("test33/st_mount", vol);

	ofn = st_openfile(vol, FILENAME, WRITE);
	STERROR("test33/st_openfile(for append)", ofn);

	for (i = 0; i < 10; i = i + 2)
	{
		j = st_appendrecord(ofn, rec[i], SHORT, &rid[i]);
		STERROR("test33/st_appendrecord", j);
	}

	printf(" the order should be 1,3,5,7,9,0,2,4,6,8\n");
	i = st_closefile(ofn);
	STERROR("test33/st_closefile(for append)", i);
	DUMPFILE;

	i = st_dismount("wiss2");  /* dismount the device wiss2 */
	STERROR("test33/st_dismount", i);

/* finalization */

	vol = st_mount("wiss2");  	/* mount the device wiss2 */
	STERROR("test33/st_mount", vol);

	i = st_destroyfile(vol, FILENAME);
	STERROR("test33/st_destroyfile", i);

	i = st_dismount("wiss2");  /* dismount the device wiss2 */
	STERROR("test33/st_dismount", i);

	printf("That's all folks\n");

}
