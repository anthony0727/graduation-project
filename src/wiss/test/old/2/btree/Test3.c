#include <wiss.h>
#include <st.h>	

#define	SIDOFFSET	0
#define	SNAMEOFFSET	SIDOFFSET + sizeof(int)
#define	SSEXOFFSET	SNAMEOFFSET + 50
#define	SAGEOFFSET	SSEXOFFSET + 2
#define	SYROFFSET	SAGEOFFSET + sizeof(int)
#define SGPAOFFSET	SYROFFSET + sizeof(int)
#define	SRECLENGTH	34

/* error handlers */
#define	IOERROR(p,c)	if((int)(c)<0) \
		{printf("%s %s\n", p, io_error(c));io_final(); exit(-1);}
#define	BFERROR(p,c)	if((int)(c)<0) \
	{printf("%s %s\n",p,bf_error(c));bf_final(); io_final(); exit(-1);}
#define	ERROR(p,c)	if((int)(c)<0) printf("%s %s\n", p, st_error((int)(c)))
#define	STERROR(p,c)	if((int)(c)<0) {ERROR(p, c); st_final(); bf_final();\
				io_final();}


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

static	KEYINFO	keyattr[] ={ 	{ SIDOFFSET, sizeof(int), TINTEGER},
				{ SNAMEOFFSET, 30, TSTRING},
				{ SGPAOFFSET, sizeof(float), TFLOAT},
			   };


/************************************************************************
**
**				test3.c
** Description:
*************************************************************************/

main(argc, argv)
int	argc;	
char	**argv;
{
	int	errorcode;		/* for returned errors */
	int	vol;			/* volume id */
	int	i;
	int	oldflag;

	/* warm up procedure for wiss */
	wiss_checkflags(&argc,&argv);
	i = io_init();			/* initialize level 0 */
	IOERROR("test3/io_init", i);
	
	i = bf_init();		/* initialize level 1 */
	BFERROR("test3/bf_init", i);

	i = st_init();			/* initialize level 2 */
	STERROR("test3/st_init", i);


	/* mount the volume call "wiss2" */
	vol = st_mount("wiss2");	
	STERROR("test3/st_mount", vol);

/* step 1. create an empty data file */
	printf(" creating an empty data file %s\n", TESTFILE);
	errorcode = st_createfile(vol, TESTFILE, 6, 90,90);
	STERROR("test3/st_createfile", errorcode);


/* step 2. create 3 (empty) index files */
	printf(" creating three indices\n");
	/* index on id */
	errorcode = st_createindex(vol, TESTFILE, 1, &keyattr[0], 
			50, FALSE, FALSE);
	STERROR("test3/st_createindex", errorcode);

	/* index on name */
	errorcode = st_createindex(vol, TESTFILE, 2, &keyattr[1], 
			50, FALSE, FALSE);
	STERROR("test3/st_createindex", errorcode);

	/* index on GPA */
	errorcode = st_createindex(vol, TESTFILE, 3, &keyattr[2], 
			50, FALSE, FALSE);
	STERROR("test3/st_createindex", errorcode);



/* step 3. load the data base -- fill the data file */
	printf(" loading the database ... \n");
	load_database(vol, TESTFILE);

	/* RETRIEVE FROM (TESTFILE) */
	printf(" retrieving all records ... \n");
	retrieve(vol, TESTFILE, NULL);



/* step 4. create 3 index files by insertion */
	printf(" creating 3 index file by insertion \n");
	insertindex(vol, TESTFILE, 1);
	insertindex(vol, TESTFILE, 2);
	insertindex(vol, TESTFILE, 3);

/* step 5. delete all indices and drop the index files */

	printf(" deleting all the index files\n");
	deleteindex(vol, TESTFILE, 1);
	deleteindex(vol, TESTFILE, 2);
	deleteindex(vol, TESTFILE, 3);

	errorcode = st_dropbtree(vol, TESTFILE, 1);
	STERROR("test3/st_dropbtree", errorcode);
	errorcode = st_dropbtree(vol, TESTFILE, 2);
	STERROR("test3/st_dropbtree", errorcode);
	errorcode = st_dropbtree(vol, TESTFILE, 3);
	STERROR("test3/st_dropbtree", errorcode);

/* step 6. create 3 index files directly (from bottom up) */

	printf(" creating three index files in batch\n");
	errorcode = st_createindex(vol, TESTFILE, 1, 
			&keyattr[0], 50, FALSE, FALSE);
	STERROR("test3/st_createindex", errorcode);
	errorcode = st_createindex(vol, TESTFILE, 2, 
			&keyattr[1], 50, FALSE, FALSE);
	STERROR("test3/st_createindex", errorcode);
	errorcode = st_createindex(vol, TESTFILE, 3, 
			&keyattr[2], 50, FALSE, FALSE);
	STERROR("test3/st_createindex", errorcode);


/* step 7. delete all indices and drop the index files */

	printf(" deleting all the index files\n");
	deleteindex(vol, TESTFILE, 1);
	deleteindex(vol, TESTFILE, 2);
	deleteindex(vol, TESTFILE, 3);

	errorcode = st_dropbtree(vol, TESTFILE, 1);
	STERROR("test3/st_dropbtree", errorcode);
	errorcode = st_dropbtree(vol, TESTFILE, 2);
	STERROR("test3/st_dropbtree", errorcode);
	errorcode = st_dropbtree(vol, TESTFILE, 3);
	STERROR("test3/st_dropbtree", errorcode);


/* clear up */
	/* remove the file since the test in done */
	errorcode = st_destroyfile(vol, TESTFILE);
	STERROR("test3/st_destroyfile", errorcode);

	/* dismount the volume call "wiss2" */
	errorcode = st_dismount("wiss2");  
	STERROR("test3/st_dismount", errorcode);

}

retrieve(vol, filename)
int 	vol;		/* volume id */
char	*filename;	/* where the records are to be retrieved */
{
	int	errorcode;		/* for returned error codes */
	int 	openfile_number;	/* open file number of the wiss file */
	RID	currid;
	studentrec student;		/* buffer for student records */

	/* open the wiss file with READ permission */
	openfile_number = st_openfile(vol, filename, READ);
	STERROR("test3/st_openfile", openfile_number);


	/* do a sequential scan, and print out those qualified records */
	for ( errorcode = st_firstfile(openfile_number, &currid); 
					errorcode >= eNOERROR;)
	{
		/* read in the qualified record */
		errorcode = st_readrecord(openfile_number, &currid,
				(char *)&student, sizeof(studentrec));
		STERROR("test3/st_readrecord", errorcode);

		/* print student record */
		PRINTST(student);

		/* set cursor to the next qualified record */
		errorcode = st_nextfile(openfile_number, &currid, &currid);

	}
	printf("\n");

	errorcode = st_closefile(openfile_number);
	STERROR("test3/st_closefile", errorcode);

}

load_database(vol, filename)
int	vol;		/* volume id */
char	*filename;	/* wiss file to store the database */
{

#define	INPUT	"studentdata/student"

	int	i;			/* loop index */
	int	errorcode;		/* for returned error codes */
	int	unixfile;		/* unix file id */
	int	openfile_number;	/* open file number of the wiss file */
	char	buf[100];		/* buffer for records in unix file */
	RID	*newrid;		/* RID of the newly created record */
	studentrec	student;	/* buffer for records in wiss file  */

	/* open the wiss file with WRITE permission */
	openfile_number = st_openfile(vol, filename, WRITE);
	STERROR("test3/st_openfile", openfile_number);

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
		errorcode = st_insertrecord(openfile_number, 
			&student, sizeof(studentrec), NULL, &newrid);
		STERROR("test3/st_insertrecord", errorcode);
	}

	/* close the wiss file */
	errorcode = st_closefile(openfile_number);
	STERROR("test3/st_closefile", errorcode);

}

/* create the whole index file by insertion */
insertindex(vol, filename, index)
int 	vol;		/* volume id */
char	*filename;	/* where the records are to be retrieved */
int	index;		/* index number */
{
	int	errorcode;		/* for returned error codes */
	int 	openfile_number;	/* open file number of the wiss file */
	int	indexfile_number;
	RID	currid;
	studentrec student;		/* buffer for student records */
	PID	pid;
	FID	fid;
	KEY	key;

	/* open the wiss file with READ permission */
	openfile_number = st_openfile(vol, filename, READ);
	STERROR("test3/st_openfile", openfile_number);

	indexfile_number = st_openbtree(vol, filename, index, WRITE);
	STERROR("test3/st_openbtree", indexfile_number);


	/* do a sequential scan, and insert all the indices */
	for ( errorcode = st_firstfile(openfile_number, &currid); 
					errorcode >= eNOERROR;)
	{
		/* read in the qualified record */
		errorcode = st_readrecord(openfile_number, &currid,
				(char *)&student, sizeof(studentrec));
		STERROR("test3/st_readrecord", errorcode);

		key.length = keyattr[index-1].length;
		key.type = keyattr[index-1].type;
		movebytes(key.value, 
			((char *)&student)+keyattr[index-1].offset, key.length);
		
		errorcode = st_insertindex(indexfile_number, &key, &currid);
		STERROR("test3/st_insertindex", errorcode);

		/* set cursor to the next qualified record */
		errorcode = st_nextfile(openfile_number, &currid, &currid);

	}
	printf("\n");

	/* close the index file */
	errorcode = st_closefile(indexfile_number);
	STERROR("test3/st_closefile", errorcode);
	errorcode = st_closefile(openfile_number);
	STERROR("test3/st_closefile", errorcode);

}



/* delete all the entries in the index file one by one */
deleteindex(vol, filename, index)
int 	vol;		/* volume id */
char	*filename;	/* where the records are to be retrieved */
int	index;		/* index # of the index file */
{
	int	errorcode;		/* for returned error codes */
	int 	openfile_number;	/* open file number of the wiss file */
	int	indexfile_number;
	RID	currid;
	studentrec student;		/* buffer for student records */
	PID	pid;
	FID	fid;
	KEY	key;


	/* open the wiss file with READ permission */
	openfile_number = st_openfile(vol, filename, READ);
	STERROR("test3/st_openfile", openfile_number);

	indexfile_number = st_openbtree(vol, filename, index, WRITE);
	STERROR("test3/st_openbtree", indexfile_number);


	/* do a sequential scan, and print out those qualified records */
	for ( errorcode = st_firstfile(openfile_number, &currid); 
					errorcode >= eNOERROR;)
	{
		/* read in the qualified record */
		errorcode = st_readrecord(openfile_number, &currid,
				(char *)&student, sizeof(studentrec));
		STERROR("test3/st_readrecord", errorcode);

		key.length = keyattr[index-1].length;
		key.type = keyattr[index-1].type;
		movebytes(key.value, 
			((char *)&student)+keyattr[index-1].offset, key.length);
		errorcode = st_deleteindex(indexfile_number, &key, &currid);
		STERROR("test3/st_deleteindex", errorcode);

		/* set cursor to the next qualified record */
		errorcode = st_nextfile(openfile_number, &currid, &currid);

	}
	printf("\n");

	/* close the index file */
	errorcode = st_closefile(indexfile_number);
	STERROR("test3/st_closefile", errorcode);
	errorcode = st_closefile(openfile_number);
	STERROR("test3/st_closefile", errorcode);

}
