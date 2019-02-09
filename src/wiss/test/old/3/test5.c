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
#define	ERROR(p,c)	if((int)(c)<0) am_error(p,(int)(c))

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
#define	ID	0
static	KEYINFO	keyattr[] ={ 
				{ SIDOFFSET, sizeof(int), TINTEGER}
			   };

static	KEY	key[] = {
				{ TINTEGER, "", sizeof(int)}
			};

/* hard wired boolean expressions */
static 	BOOLEXP boolexp[] =
{
	{GE, {SIDOFFSET, sizeof(int), TINTEGER}, sizeof(BOOLEXP), ""}, 
	{LE, {SIDOFFSET, sizeof(int), TINTEGER}, NULL, ""}, 
 };

/************************************************************************
**
**				test5.c
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
	
	(void) wiss_init();			

        trans_id = begin_trans();               /* inserted by LSM */
        printf("new transaction id = %d\n",trans_id);

	/* mount the volume call "wiss3" */
	vol = st_mount("wiss3");	
	WISSERROR("test5/st_mount", vol);

/* step 1. create an empty data file */
	printf(" creating an empty data file %s\n", TESTFILE);
	errorcode = st_createfile(vol, TESTFILE, 6, 90,90);
	WISSERROR("test5/st_createfile", errorcode);


/* step 2. create an empty index on ID */
	printf(" creating an empty index\n");

	/* index on ID */
	errorcode = st_createindex(vol, TESTFILE, /* index # */ ID, 
			&keyattr[ID], 50, FALSE, FALSE, trans_id, TRUE, FALSE);
	WISSERROR("test5/st_createindex", errorcode);


/* step 3. load the data base -- fill the data file */
	/* and create the index on ID by insertion */
	printf(" loading the database ... \n");
	load_database(vol, TESTFILE, ID, &keyattr[ID], trans_id);

/* step 4. index scan through the whole file */
	printf(" retrieving all records ... \n");
	idxretrieve(vol, TESTFILE, /* access path */ ID, 
			/* no search predicate */ NULL, &keyattr[ID],
			/* no upper & lower bound */ NULL,  NULL, trans_id);

/* step 5. increment ID # */

		printf(" changing all IDs (+1)\n");
		updateindex(vol, TESTFILE,ID, &keyattr[ID], trans_id);
		idxretrieve(vol, TESTFILE, /* access path */ ID, 
				/* no search predicate */ NULL, &keyattr[ID],
				/* no upper & lower bound */ NULL,  NULL, trans_id);

	/* step 6. delete persons who ID is between 30 and 40 */


		printf(" deleting all persons whose ID are between 30 and 40\n");
		*((int *)boolexp[0].value) = 30;
		*((int *)boolexp[1].value) = 40;
		deleteindex(vol, TESTFILE, ID, &keyattr[ID], boolexp, trans_id);
		idxretrieve(vol, TESTFILE, /* access path */ ID, 
				/* no search predicate */ NULL, &keyattr[ID],
				/* no upper & lower bound */ NULL,  NULL, trans_id);


	/* clean up */
		/* remove the file since the test in done */
		errorcode = st_dropbtree(vol, TESTFILE, ID, trans_id, TRUE, FALSE);
		WISSERROR("test5/st_dropbtree", errorcode);

		errorcode = st_destroyfile(vol, TESTFILE, trans_id, TRUE, FALSE);
		WISSERROR("test5/st_destroyfile", errorcode);

         /* now commit the transaction */
         i = commit_trans(trans_id);
         if (i != 1)
            printf("error status return from commit_trans = %d\n", i);
         else printf("commit ok\n");

		/* dismount the volume call "wiss3" */
		errorcode = st_dismount("wiss3");  
		WISSERROR("test5/st_dismount", errorcode);
	
		wiss_final();
	}

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
	WISSERROR("test5/wiss_openfile", openfile_number);

	/* open the wiss index file with WRITE permission */
	indexfile_number = wiss_openindex(vol, filename, indexno, WRITE);
	WISSERROR("test5/wiss_openindex", indexfile_number);

              /* lock the file in IX mode */
        i = wiss_lock_file(trans_id, indexfile_number, l_IX, COMMIT, FALSE);
        WISSERROR("test1/lockfile",i);

	/* open a index scan on the file just opened */
	scanid = wiss_openindexscan(openfile_number, indexfile_number,
		keyattr, NULL, NULL, NULL, trans_id, TRUE, l_IX, FALSE);
	WISSERROR("test5/wiss_openindexscan", scanid);

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
		WISSERROR("test5/wiss_insertscan", errorcode);
	}

	/* close the scan */

	errorcode = wiss_closescan(scanid);
	WISSERROR("test5/wiss_closescan", errorcode);

	errorcode = wiss_closefile(indexfile_number);
	WISSERROR("test5/wiss_closefile", errorcode);

	errorcode = wiss_closefile(openfile_number);
	WISSERROR("test5/wiss_closefile", errorcode);

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
	WISSERROR("test5/wiss_openfile", openfile_number);

	/* open the wiss index file with READ permission */
	indexfile_number = wiss_openindex(vol, filename, indexno, READ);
	WISSERROR("test5/wiss_openindex", indexfile_number);

              /* lock the file in IX mode */
        i = wiss_lock_file(trans_id, indexfile_number, l_IX, COMMIT, FALSE);
        WISSERROR("test1/lockfile",i);

	/* open a index scan on the file just opened */
	scanid = wiss_openindexscan(openfile_number, indexfile_number,
		keyattr, lb, ub, boolexp, trans_id, TRUE, l_IX, FALSE);
	WISSERROR("test5/wiss_openindexscan", scanid);

	/* set the scan cursor to the first qualified record if any */
	errorcode = am_fetchfirst(scanid, NULL,OR);

	/* do a index scan, and print out those qualified records */
	while (errorcode >= eNOERROR)
	{
		/* read in the qualified record */
		errorcode = wiss_readscan(scanid,
				(char *)&student, sizeof(studentrec));
		WISSERROR("test5/wiss_readscan", errorcode);

		/* print student record */
		PRINTST(student);

		/* set cursor to the next qualified record */
		errorcode = am_fetchnext(scanid, NULL,OR);

	}
	printf("\n");

	/* close the scan */
	errorcode = wiss_closescan(scanid);
	WISSERROR("test5/wiss_closescan", errorcode);

	errorcode = wiss_closefile(indexfile_number);
	WISSERROR("test5/wiss_closefile", errorcode);

	errorcode = wiss_closefile(openfile_number);
	WISSERROR("test5/wiss_closefile", errorcode);

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
	WISSERROR("test5/wiss_openfile", openfile_number);

	/* open the wiss index file with WRITE permission */
	indexfile_number = wiss_openindex(vol, filename, indexno, WRITE);
	WISSERROR("test5/wiss_openindex", indexfile_number);

              /* lock the file in IX mode */
        i = wiss_lock_file(trans_id, indexfile_number, l_IX, COMMIT, FALSE);
        WISSERROR("test1/lockfile",i);

	/* open a index scan on the file just opened */
	scanid = wiss_openindexscan(openfile_number, indexfile_number,
		keyattr, NULL, NULL, NULL, trans_id, TRUE, l_IX, FALSE);
	WISSERROR("test5/wiss_openindexscan", scanid);

	/* set the scan cursor to the first qualified record if any */
	errorcode = am_fetchfirst(scanid, NULL,OR);

	/* do a index scan, and print out those qualified records */
	while (errorcode >= eNOERROR)
	{
		/* read in the qualified record */
		errorcode = wiss_readscan(scanid,
				(char *)&student, sizeof(studentrec));
		WISSERROR("test5/wiss_readscan", errorcode);

		student.id ++;
		/* update the record */
		errorcode = wiss_updatescan(scanid,
				(char *)&student, sizeof(studentrec));
		WISSERROR("test5/wiss_updatescan", errorcode);
		
		/* set cursor to the next qualified record */
		errorcode = am_fetchnext(scanid, NULL,OR);

	}
	printf("\n");

	/* close the scan */
	errorcode = wiss_closescan(scanid);
	WISSERROR("test5/wiss_closescan", errorcode);

	errorcode = wiss_closefile(indexfile_number);
	WISSERROR("test5/wiss_closefile", errorcode);

	errorcode = wiss_closefile(openfile_number);
	WISSERROR("test5/wiss_closefile", errorcode);

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
	WISSERROR("test5/wiss_openfile", openfile_number);

	/* open the wiss index file with WRITE permission */
	indexfile_number = wiss_openindex(vol, filename, indexno, WRITE);
	WISSERROR("test5/wiss_openindex", indexfile_number);

              /* lock the file in IX mode */
        i = wiss_lock_file(trans_id, indexfile_number, l_IX, COMMIT, FALSE);
        WISSERROR("test1/lockfile",i);

	/* open a index scan on the file just opened */
	scanid = wiss_openindexscan(openfile_number, indexfile_number,
		keyattr, NULL, NULL, boolexp, trans_id, TRUE, l_IX, FALSE);
	WISSERROR("test5/wiss_openindexscan", scanid);

	/* set the scan cursor to the first qualified record if any */
	errorcode = am_fetchfirst(scanid, NULL,OR);

	/* do a index scan, and print out those qualified records */
	while (errorcode >= eNOERROR)
	{
		/* read in the qualified record */
		errorcode = wiss_deletescan(scanid);
		WISSERROR("test5/wiss_deletescan", errorcode);

		/* set cursor to the next qualified record */
		errorcode = am_fetchnext(scanid, NULL,OR);

	}
	printf("\n");

	/* close the scan */
	errorcode = wiss_closescan(scanid);
	WISSERROR("test5/wiss_closescan", errorcode);
	
	errorcode = wiss_closefile(indexfile_number);
	WISSERROR("test5/wiss_closefile", errorcode);

	errorcode = wiss_closefile(openfile_number);
	WISSERROR("test5/wiss_closefile", errorcode);
}
