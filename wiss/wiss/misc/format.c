
/********************************************************/
/*                                                      */
/*               WiSS Storage System                    */
/*          Version SystemV-4.0, September 1990	        */
/*                                                      */
/*              COPYRIGHT (C) 1990                      */
/*                David J. DeWitt 		        */
/*               Madison, WI U.S.A.                     */
/*                                                      */
/*	         ALL RIGHTS RESERVED                    */
/*                                                      */
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

#
/* WiSS utility "format" format a device to WiSS conventions

   INVOCATION:
	format [-] [-i] devicename

	The -i flag implies an already-existing device, and just
	prints out the device information.

	This program prompts standard output and reads replies from
	standard input.  The flag - is useful for reading specifications
	from a redirected input; parameters read are then echoed.

 */

char	signon[] = "WiSS Format Utility Version 2.0  (July, 1984)";

char	procName[] = "Format";

#include <wiss.h>
#include <io.h>
#include <sys/file.h>
#include <stdio.h>
#include <stdlib.h>

extern char	*io_error();
#define WISSERROR(x) if ((int)(x) < eNOERROR)\
	{printf("format error : %s\n", io_error(x)); io_final(); exit(-1);}
#define	CHECKMEM(a)  if (a == NULL){printf("No more main memory!\n"); exit(-1);}

extern SMDESC	*smPtr;

extern char *strcpy();
int	Redirected;

PrintHeader(DevName, Page)
char	*DevName;
PAGE	*Page;
{
	printf("Volume on device %s\n", DevName);
	printf("Title:\t\t\t%s\n", Page->VH.VHtitle);
	printf("Volume ID:\t\t%d\n", Page->VH.VHvolid);
	printf("# of Extents:\t\t%d\nPages per extent:\t%d\n",
		Page->VH.VHnumext, Page->VH.VHextsize);
	printf("\tNumber of Existing Files:\t%d\n", Page->VH.VHnumfile);
	printf("\tNumber of Available Extents:\t%d\n", Page->VH.VHfreeext);
	if (Page->VH.VHfiledir < 0)
		printf(" No file directory yet\n");
	else
		printf("\tDirectory root is page \t%d\n", Page->VH.VHfiledir);
}

Display(DevName)
char	*DevName;
{
	register i;
	int	e;

	/* see if it's already mounted ? */
	for (i = 0; i < MAXVOLS; i++)
		if (!strcmp(smPtr->VolDev[i].VDvolname, DevName))
		{
			PrintHeader(DevName, smPtr->VolDev[i].VDheader[MHEADER]);
			return(eNOERROR);
		}

	/* look for an empty entry and mount the device */
	for (i = 0; smPtr->VolDev[i].VDvolid != NOVOL; )
		if (++i >= MAXVOLS)
			return(e0TOOMANYVOLS);	/* table full ! */

	e = IO_Mount(DevName, i);
	WISSERROR(e);

	PrintHeader(DevName, smPtr->VolDev[i].VDheader[MHEADER]);

	e = IO_DisMount(i);
	WISSERROR(e);
	smPtr->VolDev[i].VDvolname[0] = '\0';
	openFileDesc[i] = NOVOL;
	smPtr->VolDev[i].VDvolid = NOVOL;

	return(eNOERROR);
}


Usage()
{
	printf("usage: format [-] [-i] devicename\n");
	exit(1);
}

readln()
/* Read past the end of the current line */
{
	for (; getchar()!= '\n'; ) ;
}

Create(DevName)
char	*DevName;
{
	int	i;		/* volatile register */
	int	volid;		/* volume ID */
	int	numext;		/* # of extents */
	int	extsize;	/* # of pages in an extent */
	PAGE	*Page;
	char	Title[256];	/* volume title */

	printf("Volume on device %s\n", DevName);
	printf("Title:\t\t\t");			/* prompt for title */
	for (i = 0; (Title[i]=getchar()) != '\n' && i < TITLEMAX - 1; i++);
	Title[i] = '\0';
	if (Redirected) printf("%s\n", Title);

	for (volid = -1; volid < 0;)
	{
		printf("Volume ID:\t\t");
		(void) scanf("%d", &volid);
		if (Redirected) printf("%d\n", volid);
		if (volid < 0)
		{ 	printf("*need a non-negative integer\n\n");
			if (Redirected) exit(-1);
		}
	}
	readln();

	i = sizeof(Page->VH.VHmap) * BITSPERBYTE; /* bound on # of exents */
	for (numext = 0; ;)
	{
		printf("# of Extents:\t\t");
		(void) scanf("%d", &numext);
		if (Redirected) printf("%d\n", numext);
		if (numext < 2) 
			printf("*need at least 2 extents \n\n");
		else if (numext >= i) 
			printf(" can't have more than %d extents\n", i - 1);
		else break;
		if (Redirected) exit(-1);
	}
	readln();

	for (extsize = 0; ;)
	{
		printf("Pages per extent:\t");
		(void) scanf("%d", &extsize);
		if (Redirected) printf("%d\n", extsize);
		if (extsize < 1)
			printf("*need at least 1 pages per extent\n\n");
		else if (extsize >= (1 << 15) )
			printf(" can't have more than %d pages\n", (1<<15) -1);
		else break;
		if (Redirected) exit(-1);
	}
	readln();

	/* actually format the device */
	(void) IO_Format(DevName, Title, volid, numext, extsize);
}


main(argc,argv)
int	argc;
char	*argv[];
{
	int	Info;		/* derived from command line */

	wiss_checkflags(&argc, &argv);
printf("starting wiss_init\n");
	wiss_init();
printf("ending wiss_init\n");
	Redirected = Info = FALSE;
	while (--argc && **++argv == '-')
	{
		switch (*((*argv)+1))
		{
		  case ' ':
		  case '\0':
			Redirected = TRUE;
			break;

		  case 'i':
			Info = TRUE;
			break;

		  default:
			Usage();
		}
	}

	if (argc != 1 || **argv == '-') Usage();
	printf("\n%s\n\n", signon);
	if (Info) (void) Display(*argv);
	else (void) Create(*argv);
	wiss_final();
	exit(0);
}
