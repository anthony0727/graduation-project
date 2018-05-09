#include <wiss.h>
#include <wiss_r.h>
#include <lockquiz.h>
#include <locktype.h>

extern	Trace0, Trace1, Trace2, Trace3;
int	T0, T1, T2, T3;

#define	SIDOFFSET	0
#define	SNAMEOFFSET	SIDOFFSET + sizeof(int)
#define	SSEXOFFSET	SNAMEOFFSET + 50
#define	SAGEOFFSET	SSEXOFFSET + 2
#define	SYROFFSET	SAGEOFFSET + sizeof(int)
#define SGPAOFFSET	SYROFFSET + sizeof(int)
#define	SRECLENGTH	34

/* error handlers */
#define	ERROR(p,c)	if((int)(c)<0) wiss_error(p,(int)(c))
#define	WISSERROR(p,c)	if((int)(c)<0) wiss_fatalerror(p,(int)(c))

#define	PRINTSTAT(v, f, i) \
	printf("# of file pages = %d, # of records = %d,",\
	wiss_filepages(v, f), wiss_recordcard(v, f));\
  	printf("# of index pages = %d, # of keys = %d\n", \
	wiss_indexpages(v, f, i), wiss_keycard(v, f, i));

/* wiss file name */
#define	TESTFILE	"student"

/* print student record */
#define	PRINTST(student)\
	printf("(ID=%3d,Age=%2.2d,Year=%d,Sex=%c,",\
		student.id, student.age, student.yr,\
		student.sex[0] == 'm' ? 'M' : 'F');\
	printf("GPA=%4.2f,Name=\"%-20.20s\")\n",student.gpa, student.name)

/* student record format (for the wiss file) */
typedef	struct
	{
		int 	id;	
		char 	name[50];
		char 	sex[2];
		int 	age;		
		int 	yr;
		float 	gpa;
	} studentrec;

/* index numbers */
#define	AGE	0
static	KEYINFO	keyattr[] ={ 
				{ SAGEOFFSET, sizeof(int), TINTEGER}
			   };

static	KEY	key[] = {
				{ TINTEGER, "", sizeof(int)}
			};

/* hard wired boolean expressions */
static 	BOOLEXP boolexp[] =
{
	{GE, {SAGEOFFSET, sizeof(int), TINTEGER}, sizeof(BOOLEXP), ""}, 
	{LE, {SAGEOFFSET, sizeof(int), TINTEGER}, NULL, ""}, 
 };

/************************************************************************
**
**				test16.c
** Description:
*************************************************************************/

main(argc, argv)
int	argc;	
char	**argv;
{
	int	errorcode;		/* for returned errors */
	int	vol;			/* volume id */
	int	i;
	int	trans_id;

	/* warm up procedure for wiss */
	wiss_checkflags(&argc,&argv);

T2 = Trace2;
T3 = Trace3;
Trace2 = 0;
Trace3 = 0;

	i = wiss_init();			/* initialize level 3 */
	WISSERROR("test16/wiss_init", i);

        trans_id = begin_trans();               /* inserted by LSM */
        printf("new transaction id = %d\n",trans_id);

	/* mount the volume call "wiss3" */
	vol = wiss_mount("wiss3");	
	WISSERROR("test16/wiss_mount", vol);

/* step 1. create an empty data file */
	printf(" creating an empty data file %s\n", TESTFILE);
	errorcode = wiss_createfile(vol, TESTFILE, 6, 90,90);
	WISSERROR("test16/wiss_createfile", errorcode);

/* step 2. create an empty index on age */
	printf(" creating an empty index\n");

	/* index on age */
	errorcode = wiss_createindex(vol, TESTFILE, /* index # */ AGE, 
			&keyattr[AGE], 50, FALSE, FALSE, trans_id, TRUE, FALSE);
	WISSERROR("test16/wiss_createindex", errorcode);

	PRINTSTAT(vol, TESTFILE, AGE);

/* step 3. load the data base -- fill the data file */
	/* and create the index on age by insertion */
	printf(" loading the database ... \n");
	load_database(vol, TESTFILE, AGE, &keyattr[AGE], trans_id);

	PRINTSTAT(vol, TESTFILE, AGE);

/* step 4. index scan through the whole file */
	printf(" retrieving all records ... \n");
	idxretrieve(vol, TESTFILE, /* access path */ AGE, 
			/* no search predicate */ NULL, &keyattr[AGE],
			/* no upper & lower bound */ NULL,  NULL, trans_id);

/* step 5. happy new year, everyone is one year older now */

		printf(" happy new year !\n");
		updateindex(vol, TESTFILE,AGE, &keyattr[AGE], trans_id);
		idxretrieve(vol, TESTFILE, /* access path */ AGE, 
				/* no search predicate */ NULL, &keyattr[AGE],
				/* no upper & lower bound */ NULL,  NULL, trans_id);

	PRINTSTAT(vol, TESTFILE, AGE);

/* step 6. delete persons who age is between 30 and 40 */

		printf(" deleting persons whose age are between 30 and 40\n");
		*((int *)boolexp[0].value) = 30;
		*((int *)boolexp[1].value) = 40;
		deleteindex(vol, TESTFILE, AGE, &keyattr[AGE], boolexp, trans_id);
		idxretrieve(vol, TESTFILE, /* access path */ AGE, 
				/* no search predicate */ NULL, &keyattr[AGE],
				/* no upper & lower bound */ NULL,  NULL, trans_id);

	PRINTSTAT(vol, TESTFILE, AGE);

/* step 7. happy new year again, everyone is one year older now */

		printf(" happy new year again !\n");
		updateindex(vol, TESTFILE,AGE, &keyattr[AGE], trans_id);
		idxretrieve(vol, TESTFILE, /* access path */ AGE, 
				/* no search predicate */ NULL, &keyattr[AGE],
				/* no upper & lower bound */ NULL,  NULL, trans_id);

	PRINTSTAT(vol, TESTFILE, AGE);

/* step 8. delete persons whose ID are  between 20 and 60 */

		printf(" deleting persons whose id are between 20 and 60\n");
		*((int *)boolexp[0].value) = 20;
		*((int *)boolexp[1].value) = 60;
		boolexp[0].fielddesc.offset = SIDOFFSET;
		boolexp[1].fielddesc.offset = SIDOFFSET;
		deleteindex(vol, TESTFILE, AGE, &keyattr[AGE], boolexp, trans_id);
		boolexp[0].fielddesc.offset = SAGEOFFSET;
		boolexp[1].fielddesc.offset = SAGEOFFSET;
		idxretrieve(vol, TESTFILE, /* access path */ AGE, 
				/* no search predicate */ NULL, &keyattr[AGE],
				/* no upper & lower bound */ NULL,  NULL, trans_id);

	PRINTSTAT(vol, TESTFILE, AGE);

/* 9. delete rest of the  records */
	printf("deleting the remaining records ...\n");
		deleteindex(vol, TESTFILE, AGE, &keyattr[AGE], NULL, trans_id);
		idxretrieve(vol, TESTFILE, /* access path */ AGE, 
				/* no search predicate */ NULL, &keyattr[AGE],
				/* no upper & lower bound */ NULL,  NULL, trans_id);

	printf(" after the deletion :\n");
	PRINTSTAT(vol, TESTFILE, AGE);

/* Finally, clean it up */
		/* remove the file since the test in done */
		errorcode = wiss_dropindex(vol, TESTFILE, AGE, trans_id, TRUE, FALSE);
		WISSERROR("test16/wiss_dropindex", errorcode);

		errorcode = wiss_destroyfile(vol, TESTFILE, trans_id, TRUE, FALSE);
		WISSERROR("test16/wiss_destroyfile", errorcode);

         	/* now commit the transaction */
         	i = commit_trans(trans_id);
         	if (i != 1)
            		printf("error status return from commit_trans = %d\n", i);
         	else printf("commit ok\n");

		/* dismount the volume call "wiss3" */
		errorcode = wiss_dismount("wiss3");  
		WISSERROR("test16/wiss_dismount", errorcode);
        (void) wiss_final();

} /* end of main */

load_database(vol, filename, indexno, keyattr, trans_id)
int	vol;		/* volume id */
char	*filename;	/* wiss file to store the database */
int	indexno;	/* index # */
KEYINFO	*keyattr;	/* attrubutes of the index */
int	trans_id;
{

#define	INPUT	"../2/btree/studentdata/student"

	int	i;			/* loop index */
	int	scanid;			/* ID of the index insert scan */
	int	errorcode;		/* for returned error codes */
	int	unixfile;		/* unix file id */
	int	openfile_number;	/* open file number of the wiss file */
	int	indexfile_number;	/* open file number of the index file */
	char	buf[100];		/* buffer for records in unix file */
	RID	*newrid;		/* RID of the newly created record */
	studentrec	student;	/* buffer for records in wiss file  */


	/* open the unix file first */
	unixfile = open(INPUT, 0);
	if (unixfile < 0)
	{
		printf("\ncan not open student data file");
		return;
	}

	/* open the wiss file with WRITE permission */
	openfile_number = wiss_openfile(vol, filename, WRITE);
	WISSERROR("test16/wiss_openfile", openfile_number);


	/* open the wiss index file with WRITE permission */
	indexfile_number = wiss_openindex(vol, filename, indexno, WRITE);
	WISSERROR("test16/wiss_openindex", indexfile_number);

              /* lock the file in IX mode */
        i = wiss_lock_file(trans_id, indexfile_number, l_IX, COMMIT, FALSE);
        WISSERROR("test1/lockfile",i);

	/* open a index scan on the file just opened */
	scanid = wiss_openindexscan(openfile_number, indexfile_number,
		keyattr, NULL, NULL, NULL, trans_id, TRUE, l_IX, FALSE);
	WISSERROR("test16/wiss_openindexscan", scanid);

	/* read records from the unix file and put them into the wiss file */
	while (read(unixfile, buf, SRECLENGTH))
	{
		/* convert the record format */
		sscanf(buf,"%d", &student.id);
		for (i = 0; i < 20; i++)
			student.name[i] = buf[3 + i]; 		
		sscanf(&buf[23], "%c %2d %d %f", 
			 student.sex, &student.age, &student.yr, &student.gpa);

		/* insert it into the wiss file */
		errorcode = wiss_insertscan(scanid,
			&student, sizeof(studentrec), NULL);
		WISSERROR("test16/wiss_insertscan", errorcode);
	}

	/* close the scan */

	errorcode = wiss_closescan(scanid);
	WISSERROR("test16/wiss_closescan", errorcode);

	errorcode = wiss_closefile(indexfile_number);
	WISSERROR("test16/wiss_closefile", errorcode);

	errorcode = wiss_closefile(openfile_number);
	WISSERROR("test16/wiss_closefile", errorcode);

}

/* retrieve all records satisfying the given predicate thru the given index */
idxretrieve(vol, filename, indexno, boolexp, keyattr, ub, lb, trans_id)
int 	vol;		/* volume id */
char	*filename;	/* where the records are to be retrieved */
int	indexno;	/* index # of the key index */
BOOLEXP	*boolexp;	/* qualification for retrieval */
KEYINFO	*keyattr;	/* attribute of the given key */
KEY	*ub;		/* upper bound of scan */
KEY	*lb;		/* lower bound of scan */
int	trans_id;
{
	int	errorcode;		/* for returned error codes */
	int 	openfile_number;	/* open file number of the wiss file */
	int 	indexfile_number;	/* open file number of the index file */
	int 	scanid;			/* id of the scan opened */
	studentrec student;		/* buffer for student records */
	int	i;

	/* open the wiss file with READ permission */
	openfile_number = wiss_openfile(vol, filename, READ);
	WISSERROR("test16/wiss_openfile", openfile_number);

	/* open the wiss index file with READ permission */
	indexfile_number = wiss_openindex(vol, filename, indexno, READ);
	WISSERROR("test16/wiss_openindex", indexfile_number);

              /* lock the file in IX mode */
        i = wiss_lock_file(trans_id, indexfile_number, l_IS, COMMIT, FALSE);
        WISSERROR("test1/lockfile",i);

	/* open a index scan on the file just opened */
	scanid = wiss_openindexscan(openfile_number, indexfile_number,
		keyattr, lb, ub, boolexp, trans_id, TRUE, l_IS, FALSE);
	WISSERROR("test16/wiss_openindexscan", scanid);

	/* set the scan cursor to the first qualified record if any */
	errorcode = wiss_fetchfirst(scanid, NULL);

	/* do a index scan, and print out those qualified records */
	while (errorcode >= eNOERROR)
	{
		/* read in the qualified record */
		errorcode = wiss_readscan(scanid,
				(char *)&student, sizeof(studentrec));
		WISSERROR("test16/wiss_readscan", errorcode);

		/* print student record */
		PRINTST(student);

		/* set cursor to the next qualified record */
		errorcode = wiss_fetchnext(scanid, NULL);

	}
	printf("\n");

	/* close the scan */
	errorcode = wiss_closescan(scanid);
	WISSERROR("test16/wiss_closescan", errorcode);

	errorcode = wiss_closefile(indexfile_number);
	WISSERROR("test16/wiss_closefile", errorcode);

	errorcode = wiss_closefile(openfile_number);
	WISSERROR("test16/wiss_closefile", errorcode);

}

/* retrieve all records satisfying the given predicate thru the given index */
updateindex(vol, filename, indexno, keyattr, trans_id)
int 	vol;		/* volume id */
char	*filename;	/* where the records are to be retrieved */
int	indexno;	/* index # of the key index */
KEYINFO	*keyattr;	/* attribute of the given key */
int	trans_id;
{
	int	errorcode;		/* for returned error codes */
	int 	openfile_number;	/* open file number of the wiss file */
	int 	indexfile_number;	/* open file number of the index file */
	int 	scanid;			/* id of the scan opened */
	studentrec student;		/* buffer for student records */
	int	i;

	/* open the wiss file with WRITE permission */
	openfile_number = wiss_openfile(vol, filename, WRITE);
	WISSERROR("test16/wiss_openfile", openfile_number);

	/* open the wiss index file with WRITE permission */
	indexfile_number = wiss_openindex(vol, filename, indexno, WRITE);
	WISSERROR("test16/wiss_openindex", indexfile_number);

              /* lock the file in IX mode */
        i = wiss_lock_file(trans_id, indexfile_number, l_IX, COMMIT, FALSE);
        WISSERROR("test1/lockfile",i);

	/* open a index scan on the file just opened */
	scanid = wiss_openindexscan(openfile_number, indexfile_number,
		keyattr, NULL, NULL, NULL, trans_id, TRUE, l_IX, FALSE);
	WISSERROR("test16/wiss_openindexscan", scanid);

	/* set the scan cursor to the first qualified record if any */
	errorcode = wiss_fetchfirst(scanid, NULL);

	/* do a index scan, and print out those qualified records */
	while (errorcode >= eNOERROR)
	{
		/* read in the qualified record */
		errorcode = wiss_readscan(scanid,
				(char *)&student, sizeof(studentrec));
		WISSERROR("test16/wiss_readscan", errorcode);

		/* one year older */
		student.age ++;
		/* update the record */
		errorcode = wiss_updatescan(scanid,
				(char *)&student, sizeof(studentrec));
		WISSERROR("test16/wiss_updatescan", errorcode);
		
		/* set cursor to the next qualified record */
		errorcode = wiss_fetchnext(scanid, NULL);

	}
	printf("\n");


Trace2 = T2;
Trace3 = T3;
	/* close the scan */
	errorcode = wiss_closescan(scanid);
	WISSERROR("test16/wiss_closescan", errorcode);

Trace2 = 0;
Trace3 = 0;
	errorcode = wiss_closefile(indexfile_number);
	WISSERROR("test16/wiss_closefile", errorcode);

	errorcode = wiss_closefile(openfile_number);
	WISSERROR("test16/wiss_closefile", errorcode);

}

deleteindex(vol, filename, indexno, keyattr, boolexp, trans_id)
int 	vol;		/* volume id */
char	*filename;	/* where the records are to be retrieved */
int	indexno;	/* index # of the index file */
KEYINFO	*keyattr;	/* attributes of the index */
BOOLEXP	*boolexp;
int	trans_id;
{
	int	errorcode;		/* for returned error codes */
	int	scanid;
	int 	openfile_number;	/* open file number of the wiss file */
	int	indexfile_number;
	RID	currid;
	studentrec student;		/* buffer for student records */
	PID	pid;
	FID	fid;
	int	i;

	/* open the wiss file with WRITE permission */
	openfile_number = wiss_openfile(vol, filename, WRITE);
	WISSERROR("test16/wiss_openfile", openfile_number);

	/* open the wiss index file with WRITE permission */
	indexfile_number = wiss_openindex(vol, filename, indexno, WRITE);
	WISSERROR("test16/wiss_openindex", indexfile_number);

              /* lock the file in IX mode */
        i = wiss_lock_file(trans_id, indexfile_number, l_IX, COMMIT, FALSE);
        WISSERROR("test1/lockfile",i);

	/* open a index scan on the file just opened */
	scanid = wiss_openindexscan(openfile_number, indexfile_number,
		keyattr, NULL, NULL, boolexp, trans_id, TRUE, l_IX, FALSE);
	WISSERROR("test16/wiss_openindexscan", scanid);

	/* set the scan cursor to the first qualified record if any */
	errorcode = wiss_fetchfirst(scanid, NULL);


	/* do a index scan, and print out those qualified records */
	while (errorcode >= eNOERROR)
	{
		/* read in the qualified record */
		errorcode = wiss_deletescan(scanid);
		WISSERROR("test16/wiss_deletescan", errorcode);

		/* set cursor to the next qualified record */
		errorcode = wiss_fetchnext(scanid, NULL);

	}
	printf("\n");

/*
Trace2 = -1;
Trace3 = -1;
Trace0 = -1;
*/
	/* close the scan */
	errorcode = wiss_closescan(scanid);
	WISSERROR("test16/wiss_closescan", errorcode);

/*
Trace2 = 0;
Trace3 = 0;
Trace0 = 0;
*/
	
	errorcode = wiss_closefile(indexfile_number);
	WISSERROR("test16/wiss_closefile", errorcode);

	errorcode = wiss_closefile(openfile_number);
	WISSERROR("test16/wiss_closefile", errorcode);
}
