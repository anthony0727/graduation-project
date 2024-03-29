
/********************************************************/
/*							*/
/*		Wisconsin Storage System		*/
/*		Version 2.0 July, 1984			*/
/*							*/
/*		COPYRIGHT (C) 1984			*/
/*		Computer Sciences Department		*/
/*		University of Wisconsin			*/
/*		Madison, WI 53706			*/
/*							*/
/********************************************************/


/********************************************************/
/*							*/
/*		Wisconsin Storage System		*/
/*		Version 2.0 July, 1984			*/
/*							*/
/*		COPYRIGHT (C) 1984			*/
/*		Computer Sciences Department		*/
/*		University of Wisconsin			*/
/*		Madison, WI 53706			*/
/*							*/
/********************************************************/



/* Test Program 2.

   This program reads a student database from a UNIX file, and
   stores it in a wiss file.
   Index scans are then tested on the file.
   This test requires a wiss volume stored in a UNIX file called
   "wiss_disk."

*/


#include <wiss.h>
#include <wiss_r.h>

#include <lockquiz.h>
#include <locktype.h>

/* error handler */
#define	WISSERROR(p,c)	if((int)(c)<0) wiss_fatalerror(p,(int)(c))
/* the name of the device we are testing */
#define DEVICENAME	"wiss_disk"
/* the name of the wiss file */
#define	TESTFILE	"student"


/* schema information */
	
/* student record format */
typedef	struct
{
	int 	id;	
	char 	name[50];
	char 	sex[2];
	int 	age;		
	int 	yr;
	float 	gpa;
} studentrec;

/* field offsets within the student record */
#define	SIDOFFSET	0
#define	SNAMEOFFSET	SIDOFFSET + sizeof(int)
#define	SSEXOFFSET	SNAMEOFFSET + 50
#define	SAGEOFFSET	SSEXOFFSET + 2
#define	SYROFFSET	SAGEOFFSET + sizeof(int)
#define SGPAOFFSET	SYROFFSET + sizeof(int)
#define	SRECLENGTH	34


/* macro for printing a student record */
#define	PRINTST(student)\
	printf("(ID=%3d,Age=%2.2d,Year=%d,Sex=%c,",\
		student.id, student.age, student.yr,\
		student.sex[0] == 'm' ? 'M' : 'F');\
	printf("GPA=%4.2f,Name=\"%-20.20s\")\n",student.gpa, student.name)


/* a hard-wired boolean expression */
static 	BOOLEXP boolexp[5] =
{
	{GE, {SGPAOFFSET, sizeof(float), TFLOAT}, sizeof(BOOLEXP), ""}, 
	{NE, {SNAMEOFFSET, 1, TSTRING}, sizeof(BOOLEXP), ""}, 
	{EQ, {SSEXOFFSET, 1, TSTRING}, sizeof(BOOLEXP), ""}, 
	{GT, {SIDOFFSET, sizeof(int), TINTEGER}, sizeof(BOOLEXP), ""}, 
	{LT, {SIDOFFSET, sizeof(int), TINTEGER}, NULL, ""}, 
 };


/* index number */
#define	GPA	0

/* key attribute, and key templete */
static KEYINFO keyattr[] = { {SGPAOFFSET, sizeof(float), TFLOAT},};
static KEY key[] = { {TFLOAT, "", sizeof(float)},};


main(argc, argv)
int	argc;	
char	**argv;
{
	int	status;		/* for returned flags */
	int	vol;		/* volume id */
	int	trans_id;
	int	i;

	/* warm up procedure for wiss */
	(void) wiss_init();

 	trans_id = begin_trans();               /* inserted by LSM */
        printf("new transaction id = %d\n",trans_id);

	/* mount the device called DEVICENAME */
	vol = wiss_mount(DEVICENAME);	
	WISSERROR("test1/wiss_mount", vol);

	/* create a wiss file which has 6 pages initially, both the extent
            fill factor and page fill factor are 90%                      */
	status = wiss_createfile(vol, TESTFILE, 6, 90,90);
	WISSERROR("test2/wiss_createfile", status);

	/* build a boolean expression to find
	   female student records that has a GPA greater than
	   or equal to 2.5, a student ID greater than 30 and less
	   than 90, a name that does not begin with the letter "c".
	*/

	*(float *)boolexp[0].value = 2.5;
	*(char *)boolexp[1].value = 'c';
	*(char *)boolexp[2].value = 'f';
	*(int *)boolexp[3].value = 30;
	*(int *)boolexp[4].value = 90;


	/* LOAD the database from a unix file into a wiss file */
	printf("\n loading the database ... \n");
	load_database(vol, TESTFILE, trans_id);

	/* create an index */
	status = wiss_createindex(vol, TESTFILE, GPA + 1, 
			&keyattr[GPA], 100, FALSE, FALSE,trans_id,TRUE,FALSE);
	WISSERROR("test2/wiss_createindex", status);

	/* RETRIEVE FROM (TESTFILE) */
	printf("\n\nRetriev all records ... \n");
	retrieve(vol, TESTFILE, /* access path */ GPA + 1, NULL,
		&keyattr[GPA], NULL, NULL,trans_id);

	/* RETRIEVE FROM (TESTFILE) WHERE (boolexp) */
	/* this retrieval is done through the index on GPA */
	printf("\n\nRetrieve records having a GPA >= 2.5,\n");
	printf(" a student ID between 30 and 90, a name that does not begin\n");
	printf(" with the letter \"c\", and describes a female student.\n\n");

 	*(float *)(key[GPA].value) = 2.5; 
	retrieve(vol, TESTFILE, /* access path */ GPA + 1, 
			/* search predicate */ &boolexp[1], &keyattr[GPA],
			/* no upper bound */ NULL,
			/* lower bound : GPA >= 2.5 */ &key[GPA],trans_id);

	/* DELETE FROM (TESTFILE) WHERE (boolexp) */
	printf("\n\nDelete records having a GPA >= 2.5,\n");
	printf(" a student ID between 30 and 90, a name that does not begin\n");
	printf(" with the letter \"c\", and describes a female student.\n\n");

	delete(vol, TESTFILE, /* access path */ GPA + 1, 
			/* search predicate */ &boolexp[1], &keyattr[GPA],
			/* no upper bound */ NULL,
			/* lower bound : GPA >= 2,5 */ &key[GPA],trans_id);

	/* RETRIEVE FROM (TESTFILE) */
	printf("\n\nRetrieve all records after the deletion ... \n");
	retrieve(vol, TESTFILE, /* access path */ GPA + 1, NULL,
		&keyattr[GPA], NULL, NULL,trans_id);
	
	
	/* drop index */
	status = wiss_dropindex(vol, TESTFILE, GPA + 1,trans_id,TRUE,FALSE);
	WISSERROR("test2/wiss_dropindex", status);

	/* remove the file since the test in done */
	status = wiss_destroyfile(vol, TESTFILE,trans_id,TRUE,FALSE);
	WISSERROR("test2/wiss_destroyfile", status);

  	/* now commit the transaction */
         i = commit_trans(trans_id);
         if (i != 1)
            printf("error status return from commit_trans = %d\n", i);
         else printf("commit ok\n");

	/* dismount the device */
	status = wiss_dismount(DEVICENAME);  
	WISSERROR("test1/wiss_dismount", status);

	(void) wiss_final();  


} /* end of main */


/* retrieve all records satisfying the given predicate thru the given index */
retrieve(vol, filename, indexno, boolexp, keyattr, ub, lb,trans_id)
int 	vol;		/* volume id */
char	*filename;	/* where the records are to be retrieved */
int	indexno;	/* index # of the key index */
BOOLEXP	*boolexp;	/* qualification for retrieval */
KEYINFO	*keyattr;	/* attribute of the given key */
KEY	*ub;		/* upper bound of scan */
KEY	*lb;		/* lower bound of scan */
int	trans_id;
{
	int	status;		/* for returned error codes */
	int 	openfile_number;/* open file number of the wiss file */
	int 	indexfile_number;/* open file number of the index file */
	int 	scanid;		/* id of the scan opened */
	int	count;		/* record count */
	studentrec student;	/* buffer for student records */
	int	i;

	/* open the wiss file with READ permission */
	openfile_number = wiss_openfile(vol, filename, READ);
	WISSERROR("test2/wiss_openfile", openfile_number);

	/* open the wiss index file with READ permission */
	indexfile_number = wiss_openindex(vol, filename, indexno, READ);
	WISSERROR("test2/wiss_openindex", indexfile_number);

        i = wiss_lock_file(trans_id, indexfile_number, l_IS, COMMIT, FALSE);
        WISSERROR("test1/lockfile",i);


	/* open an index scan on the file just opened */
	scanid = wiss_openindexscan(openfile_number, indexfile_number,
		keyattr, lb, ub, boolexp, trans_id, TRUE, l_IS,FALSE);
	WISSERROR("test2/wiss_openindexscan", scanid);

	/* set the scan cursor to the first qualified record */
	status = wiss_fetchfirst(scanid, NULL,AND);

	/* do an index scan, and print out those qualified records */
	for (count = 0; status >= eNOERROR; count++)
	{
		/* read in the qualified record */
		status = wiss_readscan(scanid,
				(char *)&student, sizeof(studentrec));
		WISSERROR("test2/wiss_readscan", status);

		/* print student record */
		PRINTST(student);

		/* set cursor to the next qualified record */
		status = wiss_fetchnext(scanid, NULL,AND);

	}
	printf(" %d records retrieved\n", count);

	/* close the scan */
	status = wiss_closescan(scanid);
	WISSERROR("test2/wiss_closescan", status);

	status = wiss_closefile(indexfile_number);
	WISSERROR("test2/wiss_closefile", status);

	status = wiss_closefile(openfile_number);
	WISSERROR("test2/wiss_closefile", status);

} /* end of retrieve */

delete(vol, filename, indexno, boolexp, keyattr, ub, lb,trans_id)
int 	vol;		/* volume id */
char	*filename;	/* where the records are to be retrieved */
int	indexno;	/* index # of the key index */
BOOLEXP	*boolexp;	/* qualification for retrieval */
KEYINFO	*keyattr;	/* attribute of the given key */
KEY	*ub;		/* upper bound of scan */
KEY	*lb;		/* lower bound of scan */
int	trans_id;
{

	int	status;		/* for returned error codes */
	int 	openfile_number;/* open file number of the wiss file */
	int 	indexfile_number;/* open file number of the index file */
	int	scanid;		/* id of the scan opened */
	int	count;		/* record count */
	studentrec student;	/* student record buffer */
	int	i;

	/* open the wiss file with WRITE permission */
	openfile_number = wiss_openfile(vol, filename, WRITE);
	WISSERROR("test2/wiss_openfile", openfile_number);

	/* open the wiss index file with WRITE permission */
	indexfile_number = wiss_openindex(vol, filename, indexno, WRITE);
	WISSERROR("test2/wiss_openindex", indexfile_number);

        i = wiss_lock_file(trans_id, indexfile_number, l_IX, COMMIT, FALSE);
        WISSERROR("test1/lockfile",i);

	/* open an index scan on the file just opened */
	scanid = wiss_openindexscan(openfile_number, indexfile_number,
		keyattr, lb, ub, boolexp,trans_id, TRUE,l_IX,FALSE);
	WISSERROR("test2/wiss_openindexscan", scanid);

	/* set the scan cursor pointing to the first qualified record */
	status = wiss_fetchfirst(scanid, NULL,AND);

	printf(" deleting ... \n");

	/* do an index scan on the file, and delete qualified records */
	for (count = 0; status >= eNOERROR; count++)
	{ 
		/* print the record to be deleted */
		status = wiss_readscan(scanid,
				(char *)&student, sizeof(studentrec));
		WISSERROR("test2/wiss_readscan", status);

		/* print student record */
		PRINTST(student);

		/* delete qualified records */
		status = wiss_deletescan(scanid);
		WISSERROR("test2/wiss_readscan", status);

		/* fetch the next qualified record, but don't return its RID */
		status = wiss_fetchnext(scanid, NULL,AND);
	}

	printf(" %d records deleted\n", count);

	/* close the scan */
	status = wiss_closescan(scanid);
	WISSERROR("test2/wiss_closescan", status);

	/* close the wiss files */

	status = wiss_closefile(indexfile_number);
	WISSERROR("test2/wiss_closefile", status);

	status = wiss_closefile(openfile_number);
	WISSERROR("test2/wiss_closefile", status);

} /* end of delete */


load_database(vol, filename,trans_id)
int	vol;		/* volume id */
char	*filename;	/* wiss file to store the database */
int	trans_id;
{

#define	INPUT	"student"

	int	i;		/* loop index */
	int	status;		/* for returned error codes */
	int	unixfile;	/* unix file id */
	int	openfile_number;/* open file number of the wiss file */
	char	buf[100];	/* buffer for records in unix file */
	RID	newrid;	/* RID of the newly created record */
	int	count;		/* record count */
	studentrec	student;/* buffer for records in wiss file  */

	/* open the wiss file with WRITE permission */
	openfile_number = wiss_openfile(vol, filename, WRITE);
	WISSERROR("test2/wiss_openfile", openfile_number);

	/* open the unix file first */
	unixfile = open(INPUT, 0);
	if (unixfile < 0)
	{
		printf("\ncan not open student data file");
		exit(-1);
		return;
	}

	/* read records from the unix file and put them into the wiss file */
	for (count = 0; read(unixfile, buf, SRECLENGTH); count++)
	{
		/* convert the record format */
		sscanf(buf,"%d", &student.id);
		for (i = 0; i < 20; i++)
			student.name[i] = buf[3 + i]; 		
		sscanf(&buf[23], "%c %2d %d %f", 
			 student.sex, &student.age, &student.yr, &student.gpa);

		/* insert it into the wiss file */
		status = wiss_insertrecord(openfile_number, 
			&student, sizeof(studentrec), NULL, &newrid,trans_id,TRUE,FALSE);
		WISSERROR("test2/wiss_insertrecord", status);
	}

	printf(" %d records loaded\n", count);

	/* close the wiss file */
	status = wiss_closefile(openfile_number);
	WISSERROR("test2/wiss_closefile", status);

} /* end of load_database */

