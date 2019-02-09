#include <wiss.h>
#include <wiss_r.h>
#include <lockquiz.h>
#include <locktype.h>

#define	SIDOFFSET	0
#define	SNAMEOFFSET	SIDOFFSET + sizeof(int)
#define	SSEXOFFSET	SNAMEOFFSET + 50
#define	SAGEOFFSET	SSEXOFFSET + 2
#define	SYROFFSET	SAGEOFFSET + sizeof(int)
#define SGPAOFFSET	SYROFFSET + sizeof(int)
#define	SRECLENGTH	34

/* error handlers */
#define	WISSERROR(p,c)	if((int)(c)<0) am_fatalerror(p,(int)(c))

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

/* hard wired boolean expressions */
static 	BOOLEXP boolexp[5] =
{
	{GE, {SGPAOFFSET, sizeof(float), TFLOAT}, sizeof(BOOLEXP), ""}, 
	{NE, {SNAMEOFFSET, 1, TSTRING}, sizeof(BOOLEXP), "c"}, 
	{EQ, {SSEXOFFSET, 1, TSTRING}, sizeof(BOOLEXP), "f"}, 
	{GT, {SIDOFFSET, sizeof(int), TINTEGER}, sizeof(BOOLEXP), ""}, 
	{LT, {SIDOFFSET, sizeof(int), TINTEGER}, NULL, ""}, 
};

/* index numbers */
#define	AGE	0
#define	GPA	1

static	KEYINFO	keyattr[] ={ 	
				{ SAGEOFFSET, sizeof(int), TINTEGER},
				{ SGPAOFFSET, sizeof(float), TFLOAT},
			   };

static	KEY	key[] ={ 	
				{ TINTEGER, "", sizeof(int)},
				{ TFLOAT, "", sizeof(float)},
			};

/************************************************************************
**
**				test8.c
**
** Description:
**
*************************************************************************/

main(argc, argv)
int	argc;	
char	**argv;
{
	int	errorcode;		/* for returned errors */
	int	vol;			/* volume id */
	int	trans_id, i;

	/* warm up procedure for wiss */
	wiss_checkflags(&argc,&argv);
	wiss_init();

        trans_id = begin_trans();               /* inserted by LSM */
        printf("new transaction id = %d\n",trans_id);

	/* mount the volume call "wiss3" */
	vol = wiss_mount("wiss3");	
	WISSERROR("test8/wiss_mount", vol);

	/* create a wiss file which has 6 pages initially, both the extent
            fill factor and page fill factor are 90%                      */
	errorcode = wiss_createfile(vol, TESTFILE, 6, 90,90);
	WISSERROR("test8/wiss_createfile", errorcode);

	/* build boolean expression: sought records have a GPA greater than
	   or equal to 2.5, a student ID number greater than 30 and less
	   than 90, a name that does not begin with the letter "c", and
	   describe female students */

	*(float *)boolexp[0].value = 2.5;
	*(int *)boolexp[3].value = 30;
	*(int *)boolexp[4].value = 90;

	/* load the database from a unix file, and put it into a wiss file
	   call "TESTFILE"						*/
	printf(" loading the database ... \n");
	load_database(vol, TESTFILE, trans_id);

	/* create indices */
	printf(" creating two indices (one on AGE, one on GPA)\n");
	errorcode = wiss_createindex(vol, TESTFILE, AGE, 
			&keyattr[AGE], 100, FALSE, FALSE, trans_id, TRUE, FALSE);
	WISSERROR("test8/wiss_createindex", errorcode);
	errorcode = wiss_createindex(vol, TESTFILE, GPA, 
			&keyattr[GPA], 100, FALSE, FALSE, trans_id, TRUE, FALSE);
	WISSERROR("test8/wiss_createindex", errorcode);

	/* RETRIEVE FROM (TESTFILE) */
	printf(" retrieving all records ... \n");
	retrieve(vol, TESTFILE, NULL, trans_id);

	/* RETRIEVE FROM (TESTFILE) WHERE (boolexp) */
	/* this retrieval is done through the index on GPA */

	printf(" retrieve records having a GPA greater than or equal to 2.5");
	printf(", a student id between 30 and 90, \n  a name that does not ");
	printf("begin with the letter \"c\", and describe female students\n");

	*((float *)key[GPA].value) = 2.5;

	idxretrieve(vol, TESTFILE, /* access path */ GPA, 
			/* search predicate */ &boolexp[1], &keyattr[GPA],
			/* no upper bound */ NULL,
			/* lower bound : GPA >= 2.5 */ &key[GPA], trans_id);


	/* JOIN (TESTFILE) AND (TESTFILE) ON AGE */
	printf(" do a equ-join on age over 30\n");

	*((int *)key[AGE].value) = 30;

	eq_join(vol, TESTFILE, /* access path */ AGE, 
			/* no search predicate */ NULL, &keyattr[AGE],
			/* no upper bound */ NULL,
			/* lower bound : AGE >= 30 */ &key[AGE], trans_id);

	/* DELETE FROM (TESTFILE) WHERE (boolexp) */
	printf(" delete records having a GPA greater than or equal to 2.5");
	printf(", a student id between 30 and 90, \n  a name that does not ");
	printf("begin with the letter \"c\", and describe female students\n");

	delete(vol, TESTFILE, /* access path */ GPA, 
			/* search predicate */ &boolexp[1], &keyattr[GPA],
			/* no upper bound */ NULL,
			/* lower bound : GPA >= 2,5 */ &key[GPA], trans_id);

	/* RETRIEVE FROM (TESTFILE) */
	printf(" retrieve all records after the deletion \n");
	retrieve(vol, TESTFILE, NULL, trans_id);
	
	/* drop indices */
	printf(" droping all the indices\n");
	errorcode = wiss_dropindex(vol, TESTFILE, AGE, trans_id, TRUE, FALSE);
	WISSERROR("test8/wiss_dropindex", errorcode);
	errorcode = wiss_dropindex(vol, TESTFILE, GPA, trans_id, TRUE, FALSE);
	WISSERROR("test8/wiss_dropindex", errorcode);

	/* remove the file since the test in done */
	errorcode = wiss_destroyfile(vol, TESTFILE, trans_id, TRUE, FALSE);
	WISSERROR("test8/wiss_destroyfile", errorcode);

         /* now commit the transaction */
         i = commit_trans(trans_id);
         if (i != 1)
            printf("error status return from commit_trans = %d\n", i);
         else printf("commit ok\n");

	/* dismount the volume call "wiss3" */
	errorcode = wiss_dismount("wiss3");  
	WISSERROR("test8/wiss_dismount", errorcode);

	wiss_final();

}

/* retrieve all records satisfying the given predicate */
retrieve(vol, filename, boolexp, trans_id)
int 	vol;		/* volume id */
char	*filename;	/* where the records are to be retrieved */
BOOLEXP	*boolexp;	/* qualification for retrieval */
int	trans_id;
{
	int	errorcode;		/* for returned error codes */
	int 	openfile_number;	/* open file number of the wiss file */
	int 	scanid;			/* id of the scan opened */
	studentrec student;		/* buffer for student records */
	int	i;

	/* open the wiss file with READ permission */
	openfile_number = wiss_openfile(vol, filename, READ);
	WISSERROR("test8/wiss_openfile", openfile_number);

              /* lock the file in IX mode */
        i = wiss_lock_file(trans_id, openfile_number, l_IS, COMMIT, FALSE);
        WISSERROR("test1/lockfile",i);

	/* open a sequential scan on the file just opened */
	scanid = wiss_openfilescan(openfile_number, boolexp, trans_id, TRUE, l_IS, FALSE);
	WISSERROR("test8/wiss_openfilescan", scanid);

	/* set the scan cursor to the first qualified record */
	errorcode = wiss_fetchfirst(scanid, NULL);

	/* do a sequential scan, and print out those qualified records */
	while (errorcode >= eNOERROR)
	{
		/* read in the qualified record */
		errorcode = wiss_readscan(scanid,
				(char *)&student, sizeof(studentrec));
		WISSERROR("test8/wiss_readscan", errorcode);

		/* print student record */
		PRINTST(student);

		/* set cursor to the next qualified record */
		errorcode = wiss_fetchnext(scanid, NULL);

	}
	printf("\n");

	/* close the scan */
	errorcode = wiss_closescan(scanid);
	WISSERROR("test8/wiss_closescan", errorcode);

	errorcode = wiss_closefile(openfile_number);
	WISSERROR("test8/wiss_closefile", errorcode);

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
	WISSERROR("test8/wiss_openfile", openfile_number);

	/* open the wiss index file with READ permission */
	indexfile_number = wiss_openindex(vol, filename, indexno, READ);
	WISSERROR("test8/wiss_openindex", indexfile_number);

              /* lock the file in IX mode */
        i = wiss_lock_file(trans_id, indexfile_number, l_IS, COMMIT, FALSE);
        WISSERROR("test1/lockfile",i);

	/* open an index scan on the file just opened */
	scanid = wiss_openindexscan(openfile_number, indexfile_number,
		keyattr, lb, ub, boolexp, trans_id, TRUE, l_IS, FALSE);
	WISSERROR("test8/wiss_openindexscan", scanid);

	/* set the scan cursor to the first qualified record */
	errorcode = wiss_fetchfirst(scanid, NULL);

	/* do an index scan, and print out those qualified records */
	while (errorcode >= eNOERROR)
	{
		/* read in the qualified record */
		errorcode = wiss_readscan(scanid,
				(char *)&student, sizeof(studentrec));
		WISSERROR("test8/wiss_readscan", errorcode);

		/* print student record */
		PRINTST(student);

		/* set cursor to the next qualified record */
		errorcode = wiss_fetchnext(scanid, NULL);

	}
	printf("\n");

	/* close the scan */
	errorcode = wiss_closescan(scanid);
	WISSERROR("test8/wiss_closescan", errorcode);

	errorcode = wiss_closefile(indexfile_number);
	WISSERROR("test8/wiss_closefile", errorcode);

	errorcode = wiss_closefile(openfile_number);
	WISSERROR("test8/wiss_closefile", errorcode);

}

delete(vol, filename, indexno, boolexp, keyattr, ub, lb, trans_id)
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
	int	scanid;			/* id of the scan opened */
	studentrec student;		/* student record buffer */
	int	i;

	/* open the wiss file with WRITE permission */
	openfile_number = wiss_openfile(vol, filename, WRITE);
	WISSERROR("test8/wiss_openfile", openfile_number);

	/* open the wiss index file with WRITE permission */
	indexfile_number = wiss_openindex(vol, filename, indexno, WRITE);
	WISSERROR("test8/wiss_openindex", indexfile_number);

              /* lock the file in IX mode */
        i = wiss_lock_file(trans_id, indexfile_number, l_IX, COMMIT, FALSE);
        WISSERROR("test1/lockfile",i);

	/* open an index scan on the file just opened */
	scanid = wiss_openindexscan(openfile_number, indexfile_number,
		keyattr, lb, ub, boolexp, trans_id, TRUE, l_IX, FALSE);
	WISSERROR("test8/wiss_openindexscan", scanid);

	/* set the scan cursor pointing to the first qualified record */
	errorcode = wiss_fetchfirst(scanid, NULL);

	printf(" deleting ... \n");

	/* do an index scan on the file, and delete qualified records */
	while (errorcode >= eNOERROR)
	{ 
		/* print the record to be deleted */
		errorcode = wiss_readscan(scanid,
				(char *)&student, sizeof(studentrec));
		WISSERROR("test8/wiss_readscan", errorcode);

		/* print student record */
		PRINTST(student);

		/* delete qualified records */
		errorcode = wiss_deletescan(scanid);
		WISSERROR("test8/wiss_readscan", errorcode);

		/* fetch the next qualified record, but don't return its RID */
		errorcode = wiss_fetchnext(scanid, NULL);
	}

	/* close the scan */
	errorcode = wiss_closescan(scanid);
	WISSERROR("test8/wiss_closescan", errorcode);

	/* close the wiss files */

	errorcode = wiss_closefile(indexfile_number);
	WISSERROR("test8/wiss_closefile", errorcode);

	errorcode = wiss_closefile(openfile_number);
	WISSERROR("test8/wiss_closefile", errorcode);

}

load_database(vol, filename, trans_id)
int	vol;		/* volume id */
char	*filename;	/* wiss file to store the database */
int	trans_id;
{

#define	INPUT	"../2/btree/studentdata/student"

	int	i;			/* loop index */
	int	errorcode;		/* for returned error codes */
	int	unixfile;		/* unix file id */
	int	openfile_number;	/* open file number of the wiss file */
	char	buf[100];		/* buffer for records in unix file */
	RID	*newrid;		/* RID of the newly created record */
	studentrec	student;	/* buffer for records in wiss file  */

	/* open the wiss file with WRITE permission */
	openfile_number = wiss_openfile(vol, filename, WRITE);
	WISSERROR("test8/wiss_openfile", openfile_number);

	/* open the unix file first */
	unixfile = open(INPUT, 0);
	if (unixfile < 0)
	{
		printf("\ncan not open student data file");
		return;
	}

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
		errorcode = wiss_insertrecord(openfile_number, 
			&student, sizeof(studentrec), NULL, &newrid, trans_id, TRUE, FALSE);
		WISSERROR("test8/wiss_insertrecord", errorcode);
	}

	/* close the wiss file */
	errorcode = wiss_closefile(openfile_number);
	WISSERROR("test8/wiss_closefile", errorcode);

}

/* EQ-JOIN on the same file */
eq_join(vol, filename, indexno, boolexp, keyattr, ub, lb, trans_id)
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
	int 	scanid1, scanid2;	/* ids of the scan opened */
	studentrec student1, student2;	/* buffers for student records */
	static	BOOLEXP	boolexp1 = 	/* hard-wired boolean expression */
		{EQ, {SAGEOFFSET, sizeof(int), TINTEGER}, NULL, ""};
	int	i;

	/* open the wiss file with READ permission */
	openfile_number = wiss_openfile(vol, filename, READ);
	WISSERROR("test8/wiss_openfile", openfile_number);

	/* open the wiss index file with READ permission */
	indexfile_number = wiss_openindex(vol, filename, indexno, READ);
	WISSERROR("test8/wiss_openindex", indexfile_number);

              /* lock the file in IX mode */
        i = wiss_lock_file(trans_id, indexfile_number, l_IS, COMMIT, FALSE);
        WISSERROR("test1/lockfile",i);

	/* open an index scan on the file just opened */
	scanid1 = wiss_openindexscan(openfile_number, indexfile_number,
		keyattr, lb, ub, boolexp, trans_id, TRUE, l_IS, FALSE);
	WISSERROR("test8/wiss_openindexscan", scanid1);

	/* set the scan cursor to the first record */
	errorcode = wiss_fetchfirst(scanid1, NULL);

	/* do a sequential scan on the file as the outer loop */
	while (errorcode >= eNOERROR)
	{
		/* read in the record */
		errorcode = wiss_readscan(scanid1,
				(char *)&student1, sizeof(studentrec));
		WISSERROR("test8/wiss_readscan", errorcode);

		*(int *)boolexp1.value = student1.age;

              /* lock the file in IX mode */
        	i = wiss_lock_file(trans_id, indexfile_number, l_IS, COMMIT, FALSE);
        	WISSERROR("test1/lockfile",i);

		/* open an index scan on the file just opened */
		scanid2 = wiss_openindexscan(openfile_number, indexfile_number,
			keyattr, lb, ub, &boolexp1, trans_id, TRUE, l_IS, FALSE);
		WISSERROR("test8/wiss_openindexscan", scanid2);

		/* set the cursor (of the inner loop) to the first record */
		errorcode = wiss_fetchfirst(scanid2, NULL);
	
		/* inner loop scan */
		while (errorcode >= eNOERROR)
		{
			/* read in the record */
			errorcode = wiss_readscan(scanid2,
				(char *)&student2, sizeof(studentrec));
			WISSERROR("test8/wiss_readscan", errorcode);
	
			/* print the joined record */
			printf(" JOINED on Age %d : \n", student1.age);
			printf("1st student->");
			PRINTST(student1);
			printf("2nd student->");
			PRINTST(student2);

			/* set cursor to the next qualified record */
			errorcode = wiss_fetchnext(scanid2, NULL);
		}
	
		/* close the inner scan */
		errorcode = wiss_closescan(scanid2);
		WISSERROR("test8/wiss_closescan", errorcode);

		/* set cursor to the next record (outer loop) */
		errorcode = wiss_fetchnext(scanid1, NULL);

	}
	printf("\n");

	/* close the outer scan */
	errorcode = wiss_closescan(scanid1);
	WISSERROR("test8/wiss_closescan", errorcode);

	/* close the files */

	errorcode = wiss_closefile(indexfile_number);
	WISSERROR("test8/wiss_closefile", errorcode);

	errorcode = wiss_closefile(openfile_number);
	WISSERROR("test8/wiss_closefile", errorcode);

} /* eq_join */

