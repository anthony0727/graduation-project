
#include <wiss.h>
#include <wiss_r.h>

#define	WISSERROR(p,c)	if((int)(c)<0) wiss_fatalerror(p,(int)(c))

/* wiss file name */
#define	FILENAME	"test1.o1"

#define	FILESIZE 	4096*10

char	message[] = "seek to location xxxxxx and print 2048 bytes";
char	filler[] = "<<x[[ xxxxxx ]]x>>";
char	page[4100];
char	tpage[4096];

/* this routine convert a integer into an ascii string 
   - leading zeros supressed
*/
convert(ascii, binary, length)
char	*ascii;
int	binary;
int	length;
{
	for (; length > 0; length--)
	{
		ascii[length-1] = binary % 10 + '0';
		if ((binary /= 10) == 0) break;
	}
	for (length--; length > 0; length--)
		ascii[length-1] = ' ';
}

page_dump(Buffer)
register char	*Buffer;
{
	register	i;

	for (i = 0; i < PAGESIZE; Buffer++, i++)
	{
		if (i % 60 == 0) printf("\n %4d   ", i);
		printf("%c", *Buffer == '\0' ? '.' : *Buffer);
	}

	printf("\n");

}



load_database(vol, filename)
int	vol;		/* volume id */
char	*filename;	/* wiss file to store the database */
{

	int	i, j;			/* loop index */
	int	errorcode;		/* for returned error codes */
	int	unixfile;		/* unix file id */
	int	openfile_number;	/* open file number of the wiss file */
	int	offset;
	long	loffset;

	/* create and open a wiss file */
	(void) ux_destroy(vol, filename);
	errorcode = ux_create(vol, filename);
	WISSERROR("test1/ux_create", errorcode);
	openfile_number = ux_open(vol, filename, WRITE);
	WISSERROR("test1/wiss_openfile", openfile_number);

	/* create and open a unix file */
	if (creat(filename, 0644) < 0)
	{
		printf(" can not create %s \n", filename);
		exit(-1);
	} 
	if ((unixfile = open(filename, 1)) < 0)
	{
		printf("\ncan not open %s\n", filename);
		exit(-1);
	}

	for (i = 0; i < 10; i++)
	{
		offset = rand() % FILESIZE;
		filler[2] = filler [15] = '0' + i;	
		convert(&message[17], offset, 6);
		printf("%s\n", message);
		for (j = 0; j < 2048 / 16; j++)
		{
			convert(&filler[6], j, 6);
			strncpy(&page[j * 16], filler, 16);
		}
		loffset = offset;
		
		/* write to the unix file */
		if (lseek(unixfile, loffset, (long) 0) != loffset)
			printf(" seek inconsistent\n");
		if (write(unixfile, page, 2048) !=2048)
			printf(" write inconsistent\n");

		/* write to the wiss file */
		errorcode = ux_lseek(openfile_number, offset, 0);
		WISSERROR("test1/ux_lseek", errorcode);
		errorcode = ux_write(openfile_number, page, 2048);
		if (errorcode != 2048)
			printf(" wiss write inconsistent %d written\n",
				errorcode);

	}

	close(unixfile);
	errorcode = ux_close(openfile_number);
	WISSERROR("test1/ux_close", errorcode);
}


compare(vol, filename)
int	vol;		/* volume id */
char	*filename;	/* wiss file to store the database */
{

	int	i, j;			/* loop index */
	int	errorcode;		/* for returned error codes */
	int	unixfile;		/* unix file id */
	int	openfile_number;	/* open file number of the wiss file */
	int	pageno;
	int	count;

	/* create and open a wiss file */
	openfile_number = ux_open(vol, filename, READ);
	WISSERROR("test1/wiss_openfile", openfile_number);

	count = ST_getfilesize(openfile_number); 
	printf(" file size = %d\n", count);
	for (pageno = 0; count > 0; pageno++, count -= PAGESIZE)
	{
		printf(" starting location %4d,  ", 
			ux_lseek(openfile_number, 0L, 1));
		if ((j = ux_read(openfile_number, page, PAGESIZE)) < 0)
		{ 
			printf(" unexcepted wiss error\n");
			exit(-1);
		}
		printf(" %d bytes read from the wiss file\n", j);
		page_dump(page);

	}

	errorcode = ux_close(openfile_number);
	WISSERROR("test1/ux_close", errorcode);
}

main(argc, argv)
int	argc;	
char	**argv;
{
	int	errorcode;		/* for returned errors */
	int	vol;			/* volume id */

	/* warm up procedure for wiss */
	wiss_checkflags(&argc,&argv);
	wiss_init();

	/* mount the volume call "wiss2" */
	vol = wiss_mount("wiss2");	
	WISSERROR("test1/wiss_mount", vol);

	load_database(vol, FILENAME);

	/* dismount the volume call "wiss2" */
	errorcode = wiss_dismount("wiss2");  
	WISSERROR("test1/wiss_dismount", errorcode);

	wiss_init();	/* clear all system tables */

	/* mount the volume call "wiss2" */
	vol = wiss_mount("wiss2");	
	WISSERROR("test1/wiss_mount", vol);

	compare(vol, FILENAME);

	/* dismount the volume call "wiss2" */
	errorcode = wiss_dismount("wiss2");  
	WISSERROR("test1/wiss_dismount", errorcode);
}

