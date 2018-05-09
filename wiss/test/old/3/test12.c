#include <wiss.h>
#include <wiss_r.h>
#include <lockquiz.h>
#include <locktype.h>

#define MAXKEYLEN 256

/* error handlers */
#define	WISSERROR(p,c)	if((int)(c)<0) am_fatalerror(p,(int)(c))

/* wiss file name */
#define	TESTFILE	"student"
#define	NUMREC		1000
#define RECLEN		200
#define	NUMIDXS		1

/* structure of the test records */
#define	SIDOFFSET	0
#define	SNAMEOFFSET	SIDOFFSET + sizeof(int)
#define	SSEXOFFSET	SNAMEOFFSET + 50
#define	SAGEOFFSET	SSEXOFFSET + 2
#define	SYROFFSET	SAGEOFFSET + sizeof(int)
#define SGPAOFFSET	SYROFFSET + sizeof(int)
#define	SRECLENGTH	34
/* the remaining bytes are junk */


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

/* print student record */
#define	PRINTST(student)\
	printf("(ID=%3d,Age=%2.2d,Year=%d,Sex=%c,",\
		student.id, student.age, student.yr,\
		student.sex[0] == 'm' ? 'M' : 'F');\
	printf("GPA=%4.2f,Name=\"%-20.20s\")\n",student.gpa, student.name)
	

/* hard wired boolean expressions */
static 	BOOLEXP boolexp[5] =
{
	{GE, {SGPAOFFSET, sizeof(float), TFLOAT}, sizeof(BOOLEXP), ""}, 
	{NE, {SNAMEOFFSET, 1, TSTRING}, sizeof(BOOLEXP), "C"}, 
	{EQ, {SSEXOFFSET, 1, TSTRING}, sizeof(BOOLEXP), "f"}, 
	{GT, {SIDOFFSET, sizeof(int), TINTEGER}, sizeof(BOOLEXP), ""}, 
	{LT, {SIDOFFSET, sizeof(int), TINTEGER}, NULL, ""}, 
 };



char	*mfname[26*3] = {
		    "Allan", "Alex", "Anthony", 
		    "Barney", "Ben", "Bert", 
		    "Caspar", "Charles", "Crist", 
		    "Danny", "David", "Don", 
		    "Ed", "Eric", "Evan", 
		    "Felix", "Frank", "Fred", 
		    "George", "Glen", "Grant", 
		    "Harry", "Henry", "Howard", 
		    "Irving", "Isaac", "Israel", 
		    "Jesse", "John", "James", 
		    "Karl", "Ken", "Kit", 
		    "Lewis", "Lou", "Lucas", 
		    "Mac", "Mark", "Marty", 
	 	    "Ned", "Neil", "Noel",
		    "Orson", "Otto", "Owen", 
		    "Paul", "Pete", "Phil", 
		    "Quentin", "Quenton", "Quincy",
		    "Ralph", "Richard","Roger", 
		    "Sam", "Simon", "Steve", 
		    "Tadd", "Ted", "Terry", 
		    "Ulysses", "Upton", "Uriah", 
		    "Verna", "Victor", "Vivian",
	      	    "Walter", "Ward", "Willian", 
		    "Xavier",  "Xeron", "Xu",
		    "Yale", "Yancey", "Yankee", 
		    "Zack", "Zany", "Zelig", 
		    };

char	*ffname[26*3] = {
		    "Alice", "Anna", "April",
		    "Barbara", "Beth", "Betty",
		    "Carol", "Cathy", "Crystal",
		    "Debby", "Dina", "Doris",
	    	    "Elva", "Emily", "Eve",
	  	    "Fanny", "Frances", "Fulvia",
		    "Gail", "Gilda", "Ginny",
		    "Helene", "Hedy", "Hulda",
 		    "Ida", "Iris", "Ivy",
		    "Jane", "Joan", "Julie",
		    "Kate", "Kathy", "Kitty",
		    "Lara", "Linda", "Lisa",
		    "Mary", "Maria", "May",
		    "Nancy", "Nanny", "Nina", 
		    "Olga", "Olive", "Opal",
		    "Patty", "Penny", "Polly", 
		    "Queen", "Queenie", "Quack",
		    "Robin", "Rosa", "Ruby",
		    "Sandra", "Sandy", "Sophia",
		    "Tessie", "Theodosia","Thresa",
		    "Una", "Ursa", "Ursula",
		    "Vicky", "Victoria", "Violet",
		    "Wanda", "Wendy", "Wilma",
		    "Xenia", "Xoanon", "Xanthippe",
		    "Yolanda", "Yvette", "Yvonne",
		    "Zenia", "Zodoary", "Zoe",
		};

char	*lname[26*3] = {"Adams", "Alburn", "Allen",
		    "Beil", "Best", "Blum",
		    "Cate", "Cork", "Cox",
		    "Davis", "Doyle", "Duff",
		    "Edge", "Erickson", "Esser",
		    "Fabar", "Farley", "Fox",
		    "Gard", "Gerber", "Green",
		    "Hagemann", "Hart", "Hoffmann",
		    "Ice", "Ill", "Imm",
		    "Jacobs", "Johnson", "Judd",
		    "Kamps", "Keyes", "Kohl",
		    "Lewis", "Lincoln", "Little", 
		    "Marty", "Miller" ,"Murphy",
		    "Newell", "Newton", "Nichlos", 
		    "Obey", "Olsen", "Oscar",
		    "Packer", "Page", "Patterson",
		    "Quale", "Quam", "Quick",
		    "Ray", "Reed", "Rider",
		    "Saba", "Schmidt", "Scott",
		    "Tank", "Taylor", "Timm",
		    "Uhl", "Uhr", "Ullman",
	 	    "Van", "Vick", "Victor",
		    "Wagner", "Walker", "Wall",
		    "Xex", "Xistris", "Xu",
		    "Yank", "York", "Young",
		    "Zahn", "Zaph", "Zimmer"};

/* index numbers */
#define	AGE	0
#define	GPA	AGE+1

static	KEYINFO	keyattr =  {SAGEOFFSET, sizeof(int), TINTEGER};
static	KEY	key =  {TINTEGER, "", sizeof(int)};

int	delete_count;

/************************************************************************
**
**				test12.c
**
** Description:
**
*************************************************************************/

main(argc, argv)
int	argc;	
char	**argv;
{
	int	i, j;
	int	count;
	int	errorcode;		/* for returned errors */
	int	vol;		/* volume id */
	int	trans_id; 



	/* warm up procedure for wiss */
	wiss_checkflags(&argc,&argv);
	wiss_init();

        trans_id = begin_trans();               /* inserted by LSM */
        printf("new transaction id = %d\n",trans_id);

/* warming up */

	vol = wiss_mount("wiss3");


/* create empty files */

	/* create a wiss file which has 6 pages initially, both the extent
            fill factor and page fill factor are 90%                      */
	errorcode = wiss_createfile(vol, TESTFILE, 6, 90,90);
	WISSERROR("test12/wiss_createfile", errorcode);


/* populating databases */

	printf(" populating database on volume wiss3 ... \n");
	populate(vol, TESTFILE, NUMREC, trans_id);


/* count and check the # of records generated */

	/* RETRIEVE FROM (TESTFILE) */
	printf(" retrieving all records on volume wiss3 ... \n");
	count = retrieve(vol, TESTFILE, NULL, trans_id);
	if (count != NUMREC)
		{
		    printf(" # of records retrieved (%d) incorrect\n", count);
			exit(-1);
		}


/* building indices */

		printf(" creating 2 indices on volume wiss3\n");
		errorcode = wiss_createindex(vol, TESTFILE, 1, 
			keyattr, 100, FALSE, FALSE, trans_id, TRUE, FALSE);
		WISSERROR("test12/wiss_createindex", errorcode);


/* retrieve and check key order thorugh indices */

	/* retrieve and check through indices */
	/* RETRIEVE FROM (TESTFILE) WHERE (boolexp) */
	/* this retrieval is done through the index on GPA */
	printf(" retrieving and checking the database through");
	printf(" index 1 on volume wiss3\n"  );
	count = idxretrieve(vol, TESTFILE, 1, NULL,
				&keyattr, NULL, NULL, trans_id);
	if (count != NUMREC)
		{
	 	   printf(" # of records retrieved(%d) incorrect\n", 
					count);
				exit(-1);
		}
	

	/* build boolean expression: sought records have a GPA greater than
	   or equal to 2.5, a student ID number greater than 30 and less
	   than 90, a name that does not begin with the letter "c", and
	   describe female students */

	*(float *)boolexp[0].value = 2.5;
	*(int *)boolexp[3].value = 30;
	*(int *)boolexp[4].value = 90;

	delete_count = delete(vol, TESTFILE, boolexp, trans_id);
	printf(" 1 records deleted from volume wiss3\n");

	/* RETRIEVE FROM (TESTFILE) */
	printf(" retrieving all records on volume wiss3 ... \n");
	count = retrieve(vol, TESTFILE, NULL, trans_id);
	if (count + delete_count != NUMREC)
	{
		printf(" # of records retrieved (%d) incorrect\n", 
				count);
		exit(-1);
	}

	/* remove the file since the test in done */
	printf(" removing all files and droping all the indices\n");
	errorcode = wiss_destroyfile(vol, TESTFILE, trans_id, TRUE, FALSE);
	WISSERROR("test12/wiss_destroyfile", errorcode);

		/* drop indices */
	errorcode = wiss_dropindex(vol, TESTFILE, 1, trans_id, TRUE, FALSE);
	WISSERROR("test12/wiss_dropindex", errorcode);

         /* now commit the transaction */
         i = commit_trans(trans_id);
         if (i != 1)
            printf("error status return from commit_trans = %d\n", i);
         else printf("commit ok\n");

	/* dismount the first volume */
	errorcode = wiss_dismount("wiss3");  
	WISSERROR("test12/wiss_dismount", errorcode);

        (void) wiss_final();
}

int retrieve(vol, filename, boolexp, trans_id)
int 	vol;		/* volume id */
char	*filename;	/* where the records are to be retrieved */
BOOLEXP	*boolexp;	/* qualification for retrieval */
int	trans_id;
{

	int	errorcode;		/* for returned error codes */
	int 	openfile_number;	/* open file number of the wiss file */
	int 	scanid;			/* id of the scan opened */
	int	count;
	studentrec student;		/* buffer for student records */
	int	i;

	/* open the wiss file with READ permission */
	openfile_number = wiss_openfile(vol, filename, READ);
	WISSERROR("test12/wiss_openfile", openfile_number);

              /* lock the file in IX mode */
        i = wiss_lock_file(trans_id, openfile_number, l_IS, COMMIT, FALSE);
        WISSERROR("test1/lockfile",i);

	/* open a sequential scan on the file just opened */
	scanid = wiss_openfilescan(openfile_number, boolexp, trans_id, TRUE, l_IS, FALSE);
	WISSERROR("test12/wiss_openfilescan", scanid);

	/* set the scan cursor to the first qualified record if any */
	errorcode = wiss_fetchfirst(scanid, NULL);

	/* do a sequential scan, and print out those qualified records */
	count = 0;
	while (errorcode >= eNOERROR)
	{
		/* read in the qualified record */
		errorcode = wiss_readscan(scanid,
				(char *)&student, sizeof(studentrec));
		WISSERROR("test12/wiss_readscan", errorcode);
		count++;

		/* print student record */
/*
		PRINTST(student);
*/

		/* set cursor to the next qualified record */
		errorcode = wiss_fetchnext(scanid, NULL);

	}

	/* close the scan */
	errorcode = wiss_closescan(scanid);
	WISSERROR("test12/wiss_closescan", errorcode);

	errorcode = wiss_closefile(openfile_number);
	WISSERROR("test12/wiss_closefile", errorcode);

	return(count);
}

populate(vol, filename, num_rec, trans_id)
int	vol;		/* volume id */
char	*filename;	/* wiss file to store the database */
int	num_rec;	/* how many records */
int	trans_id;
{

	register i, j;			/* loop index */
	int	errorcode;		/* for returned error codes */
	int	openfile_number;	/* open file number of the wiss file */
	char	buf[RECLEN];		/* buffer for records in unix file */
	RID	*newrid;		/* RID of the newly created record */
	int	rand_num;
	char	*sptr;
	studentrec	student;	/* buffer for records in wiss file  */

	/* open the wiss file with WRITE permission */
	openfile_number = wiss_openfile(vol, filename, WRITE);
	WISSERROR("test12/wiss_openfile", openfile_number);

	for (i = 0; i < num_rec; i++)
	{
		rand_num = rand();
		student.id = i;
		student.sex[0] = (rand_num % 13 % 2) ? 'm' : 'f';
		if (student.sex[0] == 'm')
			sptr = mfname[rand_num % 389 % 26*3];
		else
			sptr = ffname[rand_num % 389 % 26*3];
		for (j = 0; *sptr; j++)
			student.name[j] = *(sptr++);
		student.name[j] = ' ';
		sptr = lname[rand_num % 377 % 26*3];
		for (j++; *sptr; j++)
			student.name[j] = *(sptr++);
		student.name[j] = '\0';
		student.age = 18 + rand_num % 80;
		student.yr = rand_num % 5 + 1;
		student.gpa = (rand_num % 41) / 10.0; 

		/* insert it into the wiss file */
		errorcode = wiss_insertrecord(openfile_number, 
			&student, sizeof(studentrec), NULL, &newrid, trans_id, TRUE, FALSE);
		WISSERROR("test12/wiss_insertrecord", errorcode);

	}

	/* close the wiss file */
	errorcode = wiss_closefile(openfile_number);
	WISSERROR("test12/wiss_closefile", errorcode);

}

/* retrieve all records satisfying the given predicate thru the given index */
int idxretrieve(vol, filename, indexno, boolexp, keyattr, ub, lb, trans_id)
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
	int	count;
	char	keybuf[MAXKEYLEN];	/* for key order validation */
	char	*sptr;
	studentrec student;		/* buffer for student records */
	int	i;

	/* open the wiss file with READ permission */
	openfile_number = wiss_openfile(vol, filename, READ);
	WISSERROR("test12/wiss_openfile", openfile_number);

	/* open the wiss index file with READ permission */
	indexfile_number = wiss_openindex(vol, filename, indexno, READ);
	WISSERROR("test12/wiss_openindex", indexfile_number);

              /* lock the file in IX mode */
        i = wiss_lock_file(trans_id, indexfile_number, l_IS, COMMIT, FALSE);
        WISSERROR("test1/lockfile",i);

	/* open a index scan on the file just opened */
	scanid = wiss_openindexscan(openfile_number, indexfile_number,
		keyattr, lb, ub, boolexp, trans_id, TRUE, l_IS, FALSE);
	WISSERROR("test12/wiss_openindexscan", scanid);

	/* set the scan cursor to the first qualified record if any */
	errorcode = wiss_fetchfirst(scanid, NULL);

	/* do a index scan, and print out those qualified records */
	count = 0;
	while (errorcode >= eNOERROR)
	{
		/* read in the qualified record */
		errorcode = wiss_readscan(scanid,
				(char *)&student, sizeof(studentrec));
		WISSERROR("test12/wiss_readscan", errorcode);

		/* print student record */
/*
		PRINTST(student);
*/

		sptr = ((char *)&student)+ keyattr->offset;
		if (count > 0)
		{ /* check key order */
			if (compare_key(keybuf, sptr, keyattr->type, 
				keyattr->length) > 0)
			{ /* out of order */
				printf(" key %d wrong key sequence\n", count+1);
				printf(" key of record %d is ", count);
				print_data(keyattr->type,keyattr->length,keybuf);
				printf("\n");
				printf(" key of record %d is ", count+1);
				print_data(keyattr->type,keyattr->length,sptr);
				printf("\n");
				exit(-1);
			}
		}

		/* save the key */
		movebytes(keybuf, sptr, keyattr->length);
		count++;


		/* set cursor to the next qualified record */
		errorcode = wiss_fetchnext(scanid, NULL);

	}

	/* close the scan */
	errorcode = wiss_closescan(scanid);
	WISSERROR("test12/wiss_closescan", errorcode);

	errorcode = wiss_closefile(indexfile_number);
	WISSERROR("test12/wiss_closefile", errorcode);

	errorcode = wiss_closefile(openfile_number);
	WISSERROR("test12/wiss_closefile", errorcode);

	return(count);

}

int delete(vol, filename, boolexp, trans_id)
int	vol;		/* volume id */
char 	*filename;	/* from where the records are to be deleted */
BOOLEXP	*boolexp;	/* qualification for records to be deleted */
int	trans_id;
{

	int	errorcode;		/* for returned error codes */
	int 	openfile_number;	/* open file number of the wiss file */
	int	scanid;			/* id of the scan opened */
	int	count;
	studentrec student;		/* student record buffer */
	int	i;

	/* open the wiss file with WRITE permission */
	openfile_number = wiss_openfile(vol, filename, WRITE);
	WISSERROR("test12/wiss_openfile", openfile_number);

              /* lock the file in IX mode */
        i = wiss_lock_file(trans_id, openfile_number, l_IX, COMMIT, FALSE);
        WISSERROR("test1/lockfile",i);

	/* open a sequential scan on that file with boolean qualification */
	scanid = wiss_openfilescan(openfile_number, boolexp, trans_id, TRUE, l_IS, FALSE);
	WISSERROR("test12/wiss_openfilescan", scanid);

	/* set the scan cursor pointing to the first qualified record if any */
	errorcode = wiss_fetchfirst(scanid, NULL);

	/* do a sequential scan on the file, and delete qualified records */
	count = 0;
	while (errorcode >= eNOERROR)
	{ 

		/* print the record to be deleted */
		errorcode = wiss_readscan(scanid,
				(char *)&student, sizeof(studentrec));
		WISSERROR("test12/wiss_readscan", errorcode);
		count++;

		/* print student record */
		
		/* delete qualified records */
		errorcode = wiss_deletescan(scanid);
		WISSERROR("test12/wiss_readscan", errorcode);

		/* fetch the next qualified record, but don't return its RID */
		errorcode = wiss_fetchnext(scanid, NULL);
	}

	/* close the scan */
	errorcode = wiss_closescan(scanid);
	WISSERROR("test12/wiss_closescan", errorcode);

	/* close the wiss file */
	errorcode = wiss_closefile(openfile_number);
	WISSERROR("test12/wiss_closefile", errorcode);

	return(count);
}
