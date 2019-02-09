
#include	<sys/file.h>

#define	FILENAME	"test2.unix"
#define	NUMPAGES	1000
#define	PAGESIZE	4096
#define	FILESIZE 	(NUMPAGES*PAGESIZE)

char	page[PAGESIZE];
short	touched[NUMPAGES];

/* This program populates a file sequentially with a number of fixed size pages
*/

main()
{

	int	i, k;
	register int	j;		/* loop index */
	register int	*n;
	int	unixfile;		/* unix file id */
	long	offset;

	if ((unixfile = open(FILENAME, O_WRONLY | O_CREAT, 0644)) < 0)
	{
		printf("\ncan not open %s\n", FILENAME);
		exit(-1);
	}

	for (k = 0; k < NUMPAGES; k++) touched[k] = 0;
	for (k = 0; k < NUMPAGES; k++)
	{
		/* randomly pick a page (without replacement) */
		i = rand() % NUMPAGES;
		if (touched[i])
		{ /* too late for this one, try another */
			for (; touched[i]; i = (i == 0) ? NUMPAGES - 1: i - 1);
		} 
		touched[i] = 1;

		/* fill page i with integer i */
		for (n = (int *) page, j = 0; 
			j < PAGESIZE / sizeof(int) ; j++, *(n++) = i);

		/* write to the unix file */
		offset = i * PAGESIZE;
		if (lseek(unixfile, offset, (long) 0) != offset)
			printf(" seek inconsistent\n");
		if (write(unixfile, page, PAGESIZE) != PAGESIZE)
			printf(" write inconsistent\n");

	}

	close(unixfile);

	/* read the pages back in */
	if ((unixfile = open(FILENAME, O_RDONLY)) < 0)
	{
		printf("\ncan not open %s\n", FILENAME);
		exit(-1);
	}

	for (k = 0; k < NUMPAGES; k++) touched[k] = 0;
	for (k = 0; k < NUMPAGES; k++)
	{
		/* randomly pick a page (without replacement) */
		i = rand() % NUMPAGES;
		if (touched[i])
		{ /* too late for this one, try another */
			for (; touched[i]; i = (i == 0) ? NUMPAGES - 1: i - 1);
		} 
		touched[i] = 1;

		offset = i * PAGESIZE;
		if (lseek(unixfile, offset, (long) 0) != offset)
			printf(" seek inconsistent\n");
		if (read(unixfile, page, PAGESIZE) != PAGESIZE)
			printf(" read inconsistent\n");

		/* check the data */
		for (n = (int *) page, j = 0; j < PAGESIZE / sizeof(int) ; j++)
			if (*(n++) != i)
			{
				printf(" error detected on page %d\n", i);
				exit(-1);
			}
	}

	close(unixfile);

	/* clean up the mess */

	unlink(FILENAME);

	printf(" END_OF_TEST\n");

}


