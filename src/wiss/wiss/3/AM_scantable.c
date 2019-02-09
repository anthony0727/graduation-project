
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


#
/* Module : AM_scantable
     This module contains routines which maintain and provide services to 
     the open scan table for level 3
  
   IMPORTS:
  	int	st_accessmode();	
  
   EXPORTS:
  	int		AM_initscantalbe();
  	int		AM_addscan(openfilenumber);
  	int		AM_removescan(scanid);
  	int		AM_nextscan(opnefilenumber, scanid);
  	SCANINFO	*AM_getscan(scanid);
*/
  
#include	<wiss.h>
#include	<am.h>
/*
	#include	<prio.h>
*/

extern	SCANINFO *AM_getscan();
extern  char	 *malloc();

#define		INITARRAYSIZE	4	/* initial block pointer array size */
#define		BLOCKSIZE	16	/* size of a scan record block */

#ifdef	DEBUG

#undef	INITARRAYSIZE
#define	INITARRAYSIZE	1
#undef	BLOCKSIZE
#define	BLOCKSIZE	4

#endif

/* formula to convert a scanID into a memory address */
#define	SCANRECADDR(s)	(&(blockptr[(s)/BLOCKSIZE]->info[(s)%BLOCKSIZE]))

/* these two check for INVALID filenum (f) and scanid (s) */
#define	CHECKFILENO(f)	if((f)<0||(f)>=MAXOPENFILES) return(e3BADFILENO)
#define	CHECKSCANID(s)	if((s)<0||(s)/BLOCKSIZE>last_ptr) return(e3BADSCANID);\
	 	else if(SCANRECADDR(s)->filenum==NIL)return(e3BADSCANID)

/*
#define	CHECKFILENO_VSEM(f,b)	if((f)<0||(f)>=MAXOPENFILES) \
                          { SendSem(b) ; return(e3BADFILENO); }
#define	CHECKSCANID_VSEM(s,b)	if((s)<0||(s)/BLOCKSIZE>last_ptr) \
                              {SendSem(b);return(e3BADSCANID);} \
	 	else if(SCANRECADDR(s)->filenum==NIL) \
                              {SendSem(b);return(e3BADSCANID);}
*/

typedef struct	/* a block of scan records */
{
	SCANINFO	info[BLOCKSIZE];	/* information on scans */
} SCANBLOCK;


/* global (to this module) parameters used by the scan table module */
static int	freelist;		/* head of the free list */
static int	last_ptr;		/* last pointer used */
static int	ptrarray_size;		/* pointer array size */
static int	filechain[MAXOPENFILES];/* chains for scans of the same file */
static SCANBLOCK **blockptr;		/* pointer to the block pointer array */

/*****************************************************************************/
/*      	SCAN TABLE INTERNAL SUPPORT ROUTINE                          */
/*****************************************************************************/


int
static alloc_scanblock()

/* This routine allocates a new block and adds its address to the block
pointer array. Entries within the block are initialized as necessary.

RETURNS:
None

SIDE EFFECTS:
if the old array is too small to add one more entry, it will be
replaced by a new array with twice the size of the old array.

ERRORS:
e3NOMOREMEMORY : no more memory for scan table to grow

CROCKS:
blockptr is used as an indication whether the table is created for
the first time (ie, if blockptr is NULL, a new table is created).
*/
{

	SCANBLOCK	**new_array;	/* pointer to new pointer array */
	SCANBLOCK	*new_block;	/* pointer to new block*/
	register int	i;		/* counter */

#ifdef	TRACE
	if (checkset(&Trace3, tSCANTABLE))
		printf("alloc_scanblock()\n");
#endif

	if (last_ptr == ptrarray_size - 1 || blockptr == NULL)
	{ /* pointer array is full or the scan table  is created for the first
	  time, allocate a larger array and copy the old one into it */

		ptrarray_size <<= 1;	/* double the block array size */
		new_array = (SCANBLOCK **)malloc((unsigned)
			(ptrarray_size * sizeof(SCANBLOCK *)));
		if (new_array == NULL) return(e3NOMOREMEMORY);

		if (blockptr != NULL)
		{ /* copy the old array into the new one, and release it */
			for (i = 0; i <= last_ptr; i++)
				new_array[i] = blockptr[i];
			free((char *)blockptr);
		}
		else i = 0;	/* first array ever created */

		for (; i < ptrarray_size; i++)
			new_array[i] = NULL;	/* new ones go nowhere */
		blockptr = new_array;		/* now use new table */
	}

	/* get a new block and put its address in block pointer array */
	new_block = (SCANBLOCK *)malloc(sizeof(SCANBLOCK));
	if (new_block == NULL) return(e3NOMOREMEMORY);
	blockptr[++last_ptr] = new_block;

	/* Initializes all the entries within a newly allocated block.
	The file number of each entry is clear to NIL for future error
	checking. Entries of the new block are chained together and
	becomes the new free block list (the old list should be empty
	- if not, why allocate in the first place?).			*/

	for (i = 0, freelist = last_ptr * BLOCKSIZE; i < BLOCKSIZE; i++)
	{
		new_block->info[i].next = freelist + i + 1;
		new_block->info[i].filenum = NIL;
	}
	new_block->info[i - 1].next = NIL;	 /* end of free list */

	return(eNOERROR);

} /* alloc_scanblock */


/*****************************************************************************/
/*      	SCAN TABLE INTERNAL SUPPORT ROUTINE                          */
/*****************************************************************************/

int
AM_initscantable()

/* This routine initializes all variables used by the scan table,
  	and allocate the first block for the scan table.
  
   RETURNS:
  	None
  
   SIDE EFFECTS:
  	a new block pointer array is created
  
   ERRORS:
  	e3NOMOREMEMORY : not enough memory to create a scan table
*/
{
	register int	i;	/* index */
        /*int dummy; */		/* for WaitSem */
        int ret_val;

#ifdef	TRACE
	if (checkset(&Trace3, tSCANTABLE))
		printf("init_scantable()\n");
#endif
/*
         WaitSem (&sem);
*/
	/* clear all chains for linking scans of the same file */
	for (i = 0; i < MAXOPENFILES; i++) filechain[i] = NIL;

	/* set up these variables so that alloc_scanblock will create a new 
  	   pointer array with size 2*INITARRAYSIZE and one new scan block */

	last_ptr = -1;		/* no blocks yet */
	blockptr = NULL;
	ptrarray_size = INITARRAYSIZE;
	ret_val = alloc_scanblock();	/* create initial scan table */
/*
        SendSem(&sem);
*/
	return(ret_val);	

} /* AM_initscantable */


int
AM_addscan(filenum)
int	filenum;	/* open file number */

/* This routine adds a new scan to the scan table, and returns the scanid
  	of the new scan. Futher reference to this scan will require the scanid.
  
   RETURNS:
  	ScanID of the new scan
  
   SIDE EFFECTS:
  	updates to scan table and chain of scans for this file
  
   ERRORS:
  	e3BADFILENO if the file number passed is invalid.
*/

{
	register SCANINFO	*sptr;  	/* pointer to the scan record */
	int		scanid;		/* scan id of the new scan */
	int		e;		/* for returned errors */
	int             cprio;

#ifdef	TRACE
	if (checkset(&Trace3, tSCANTABLE))
		printf("AM_addscan(filenum = %d)\n", filenum);
#endif

	CHECKFILENO(filenum);	/* file number valid ? */

/*
	cprio = CurrentTask -> ncb_prio;
	CurrentTask -> ncb_prio = HIGHER_PRIO;
*/
	if (freelist == NIL)
	{	/* no more free entries, allocate a new block */
		e = alloc_scanblock();
		CHECKERROR(e);

/*
		CHECKERROR_VSEM(e,&sem);
*/
	}
	
	sptr = SCANRECADDR(freelist);	/* pointer to first free record */

	/* allocate a scan info record from the head of free list */
	scanid = freelist;
	freelist = sptr->next;	/* update the chain */

	/* add this scan to the file */
	sptr->next = filechain[filenum];
	filechain[filenum] = scanid;
	sptr->filenum = filenum;

	/* get the access right for this (data) file */
	sptr->accessflag = st_accessmode(filenum);
	PIDCLEAR(sptr->locked_page);
	CHECKERROR(sptr->accessflag);

/*
	CHECKERROR_VSEM(sptr->accessflag,&sem);
	CurrentTask -> ncb_prio = cprio;
*/
	return (scanid);

} /* AM_addscan */

 
int
AM_removescan(scanid)
int	scanid;			/* id of scan to be deleted */

/* This routine deletes an entry from the scan table, given the scanid
  	to be removed. This entry is placed on the empty list for future
  	use, but is never deallocated.
  
   RETURNS:
  	None
  
   SIDE EFFECTS:
  	None
  
   ERRORS:
  	e3BADSCANID if the parameter scanid is invalid.
*/
{
	register int	i;		/* loop index */
	SCANINFO	*sptr;		/* pointer to scan info record */
        int dummy;			/* for WaitSem */
	int cprio;

#ifdef	TRACE
	if (checkset(&Trace3, tSCANTABLE))
		printf("AM_removescan(scanid = %d)\n", scanid);
#endif
/*
	cprio = CurrentTask -> ncb_prio;
	CurrentTask -> ncb_prio = HIGHER_PRIO;
	CHECKSCANID_VSEM(scanid,&sem);	
*/
	CHECKSCANID(scanid);	/* scan Id valid ? */
	sptr = SCANRECADDR(scanid);	/* pointer to scan info record */

	/* remove it from the chain of scans open on that file */
	if (filechain[sptr->filenum] == scanid)
		filechain[sptr->filenum] = sptr->next; 
	else
	{	/* locate its position in the chain and adjust the chain */
		for (i = filechain[sptr->filenum];
			 SCANRECADDR(i)->next!=scanid; i=SCANRECADDR(i)->next);
		SCANRECADDR(i)->next = sptr->next;
	}
			
	sptr->filenum = NIL;	/* clears its file number */
	sptr->next = freelist;	/* add it to the free list */
	freelist = scanid;
/*
	CurrentTask -> ncb_prio = cprio;
*/
	return(eNOERROR);

} /* AM_removescan */


int
AM_nextscan(filenum,scanid)
int	filenum;		/* file number on file which scans are open */
int	scanid;			/* scanid of current scan */

/* This routine finds the next scan open on the specified file after 
     specified scanid. If the scanid is NIL the first scan opened is returned.
     If no more scans are left NIL is returned
  
   RETURNS:
  	scanid of the next scan on the same file
  	NIL if there is none
  
   SIDE EFFECTS:
  	None
  
   ERRORS:
  	e3BADFILENO if the file number is invalid.
  	e3BADSCANID if the scanid is invalid.
  	e3SCANFILENOTMATCH if the scanid and the file number do not match.
*/
{

	SCANINFO	*sptr;	/* pointer to scan info record */
        int cprio;		/* for WaitSem */

#ifdef	TRACE
	if (checkset(&Trace3, tSCANTABLE))
		printf("AM_nextscan(filenum=%d,scanid=%d)\n", filenum, scanid);
#endif
	CHECKFILENO(filenum);	 /* is file number valid ? */

/*
	cprio = CurrentTask -> ncb_prio;
	CurrentTask -> ncb_prio = HIGHER_PRIO;
*/

	if (scanid == NIL)	/* get first scan on this file */
              { 
/*
		CurrentTask -> ncb_prio = cprio;
*/
		return(filechain[filenum]);
              }
	CHECKSCANID(scanid);	 /* is scanid valid ? */
/*
	CHECKSCANID_VSEM(scanid,&sem);	 
*/
	sptr = SCANRECADDR(scanid);

	/* check to see that filenum goes with scanid */
	if (sptr->filenum != filenum) 
        { 
/*
	    CurrentTask -> ncb_prio = cprio;
*/
	    return(e3SCANFILENOTMATCH);
        }
/*
	CurrentTask -> ncb_prio = cprio;
*/
	return(sptr->next);

} /* AM_nextscan */


SCANINFO *AM_getscan(scanid)
int	scanid;			/* ID of the scan */

/* This converts a scan ID into a memory address of a scan table entry
  
   RETURNS:
  	address of the scan table entry	identified by scanid
  
   SIDE EFFECTS:
  	None
  
   ERRORS:
  	NULL : Bad scan ID
*/
  
{

	SCANINFO	*sptr;
        int dummy; 		/* for WaitSem */
	int cprio;

#ifdef	TRACE
	if (checkset(&Trace3, tSCANTABLE))
		printf("AM_getscan(scanid=%d)\n", scanid);
#endif

/*
	cprio = CurrentTask -> ncb_prio;
	CurrentTask -> ncb_prio = HIGHER_PRIO;
*/
	if (scanid < 0 || scanid/BLOCKSIZE > last_ptr) 
              { 
/*
		CurrentTask -> ncb_prio = cprio;
*/
		return((SCANINFO *) NULL);  /* not a valid scan id */
              }
	sptr = SCANRECADDR(scanid);

	if (sptr->filenum == NIL) 
        {
/*
		CurrentTask -> ncb_prio = cprio;
*/
                return((SCANINFO *) NULL);
         }
/*
	CurrentTask -> ncb_prio = cprio;
*/
	return(sptr);

} /* AM_getscan */


int
AM_dumpscantable()

/* This routine prints the contents of the scan table. It is intented as a 
    debugging tool to trace allocation and freeing of space within the table.
  	
   RETURNS:
  	None
  
   SIDE EFFECTS:
  	None
  
   ERRORS:
  	None.
*/
{

#ifdef	TRACE

	register int 	i, j;		/* indices */
	register int	filenum;	/* open file number */
	static	char	*typename[] = {"  sequ  ","  index ", 
				"  long  ", "  hash  "};

printf("===============================================================\n");
	printf("\n            LEVEL 3 SCAN TABEL DUMP (%d blocks allocated)\n", 
			last_ptr + 1);

	for (i = 0; i <= last_ptr; i++)
	{
		printf("\n%15c|--------+--------+--------+--------|\n", ' ');   
		printf(" block[%2d]:    | scanid |  file  |scantype|  next  |",i);
		printf("\n%15c|--------+--------+--------+--------|\n", ' ');   

		for (j = 0; j < BLOCKSIZE; j++)
			printf("%15c| %4d   | %4d   |%8.8s| %4d   |\n", ' ',
				i*BLOCKSIZE+j, blockptr[i]->info[j].filenum,
				blockptr[i]->info[j].filenum == NIL ? "   --   "
				 : typename[(int)blockptr[i]->info[j].scantype],
				blockptr[i]->info[j].next);

		printf("%15c|--------+--------+--------+--------|\n", ' ');   
	}

	printf("\n  FREE LIST ");
	if (freelist == NIL) printf(" : EMPTY\n");
	else for (i = freelist; i != NIL; i = SCANRECADDR(i)->next)
		printf("->%3d ", i);
	printf("\n");

	for (i = 0; i < MAXOPENFILES; i++)
		if ((j = filechain[i]) != NIL)
		{
			printf("  FILE CHAIN [%d] ", i);
			for (; j != NIL; j = SCANRECADDR(j)->next)
				printf("->%3d ", j);
			printf("\n");
		}

printf("===============================================================\n");

#endif	TRACE

} /* AM_dumpscantable */
