#include <wiss.h>
#include <st.h>	

extern	Trace0, Trace1, Trace2;
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
	IOERROR("test20/io_init", i);
	i = bf_init();		/* initialize level 1 */
	BFERROR("test20/bf_init", i);
	i = st_init();			/* initialize level 2 */
	STERROR("test20/st_init", i);
	vol = st_mount("wiss2");	
	STERROR("test20/st_mount", vol);

	printf(" creating an empty data file %s\n", TESTFILE);
	errorcode = st_createfile(vol, TESTFILE, 6, 90,90);
	STERROR("test20/st_createfile", errorcode);

	printf(" loading the database ... \n");
	load_database(vol, TESTFILE);

/* clear up */
	/* remove the file since the test in done */
	errorcode = st_destroyfile(vol, TESTFILE);
	STERROR("test20/st_destroyfile", errorcode);

	/* dismount the volume call "wiss2" */
	errorcode = st_dismount("wiss2");  
	STERROR("test20/st_dismount", errorcode);

}


load_database(vol, filename)
int	vol;		/* volume id */
char	*filename;	/* wiss file to store the database */
{

#define	INPUT	"btree/studentdata/student"

	int	i;			/* loop index */
	int	count;
	int	errorcode;		/* for returned error codes */
	int	unixfile;		/* unix file id */
	int	openfile_number;	/* open file number of the wiss file */
	char	buf[100];		/* buffer for records in unix file */
	RID	*newrid;		/* RID of the newly created record */
	studentrec	student;	/* buffer for records in wiss file  */

	/* open the wiss file with WRITE permission */
	openfile_number = st_openfile(vol, filename, WRITE);
	STERROR("test20/st_openfile", openfile_number);

	/* open the unix file first */
	unixfile = open(INPUT, 0);
	if (unixfile < 0)
	{
		printf("\ncan not open student data file");
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
		errorcode = st_insertrecord(openfile_number, 
			&student, sizeof(studentrec), NULL, &newrid);
		STERROR("test20/st_insertrecord", errorcode);
	}

	/* close the wiss file */
	errorcode = st_closefile(openfile_number);
	STERROR("test20/st_closefile", errorcode);


}

