
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
/* MODULE ST_directory: level 2 routines for maintaining file directory 

   IMPORTS:
	io_findfiledir(volid, *firstpage, *fileid)
	io_setfiledir(volid, *firstpage)
	io_readpage(*pid, bufptr)
	io_writepage(*pid, bufptr, type, *flag)
	io_allocpages(*fileid, *nearpid, numpages, *allocatedpages)
	io_freepage(*fileid, *pid)
	malloc(), free(), shmAlloc(), shmFree()

   EXPORTS:

	ST_mount(volid)
	ST_dismount(volid)

	ST_accessfiledesc(volid, filename, *descriptor, accessflag)
	ST_newfiledesc(volid, filename, *fid, pff)
	ST_zapfiledesc(volid, filename, *fid)

*/

/*
	The file directory is implemented by a hybrid method of "
	extendible hashing" and "chained bucket hash".

*/

#include        <wiss.h>
#include        <st.h>

extern char *malloc();
extern char *shmAlloc();
extern char *strcpy();

/****************************************************************************/
/* In this implementation, buffering of directory pages is done as follows :*/
/*  1. root page - a dedicated buffer is allocated from heap when mounting  */
/*                 a volume, and release when the volume dismounts          */
/*  2. leaf pages - there is one buffer (for each volume) for caching the   */
/*                  most recentlyi accessed leaf page                       */
/****************************************************************************/


/****************************************************************************/
/*         DECLARATIONS : Constants, Structures and Macros                  */
/****************************************************************************/

#define PRINTDESCRIPTOR(f) \
	{ printf("FID=%d:%d,",(f)->fileid.Fvolid,(f)->fileid.Ffilenum);\
	printf("Perm=%3o,",(f)->flags);\
	printf("1stPID=%d:%d,",(f)->firstpage.Pvolid,(f)->firstpage.Ppage);\
	printf("LastPID=%d:%d,",(f)->lastpage.Pvolid,(f)->lastpage.Ppage);\
	printf("pff=%d,filename=%s",(f)->pff, (f)->filename); }

/*  ----  short hands for accessing information in directory table   ----- */
#define	ROOT(i)		(smPtr->dirtable[i].root)
#define	LEAF(i)		(smPtr->dirtable[i].leaf)
#define VOLID(i)	(smPtr->dirtable[i].volid)
#define	ROOTFID(i)	(smPtr->dirtable[i].fid)
#define	DROOTPID(i)	(smPtr->dirtable[i].pid)
#define	GDEPTH(i)	(smPtr->dirtable[i].globaldepth)
#define	BUCKET(i,j)	(smPtr->dirtable[i].root->bucket[j])
#define MOUNTCNT(i) 	(smPtr->volMountCnt[i])

/*  -----  macros related to level 0 physical IO management ----- */
#define	ALLOCPAGE(fidptr,nearpidptr,newpidptr) \
	{int e; e = io_allocpages(fidptr, (PID *)nearpidptr,1,newpidptr);\
		CHECKERROR(e);}
#define	FREEPAGE(fidptr,pidptr) \
	{int e;	e = io_freepage(fidptr,(PID *)pidptr); CHECKERROR(e);}
#define	READROOT(pidptr,pageptr) \
	{int e; e = io_readpage((pidptr),(PAGE *)(pageptr)); CHECKERROR(e);}
#define	WRITEROOT(i)\
	{int e; e = io_writepage(&DROOTPID(i),(PAGE *)ROOT(i),SYNCH,NULL);\
		CHECKERROR(e);}
#define	READLEAF(vol,spid,dp) \
	{int e; PID z_pid; z_pid.Ppage = spid; z_pid.Pvolid = VOLID(vol);\
		if (!PIDEQ(dp->thispage, z_pid))\
		   {e = io_readpage(&z_pid, (PAGE *)(dp)); CHECKERROR(e);}}
#define	WRITELEAF(pidptr,dp) \
	{int e; e = io_writepage(pidptr,(PAGE *)dp,SYNCH,NULL); CHECKERROR(e);}

/*  -----  macros related to level 2 file directory  ----- */
#define	FINDVOL(v,j)	for(j = 0; j < MAXVOLS && VOLID(j) != (v); j++);\
			if(j == MAXVOLS) return(e2VOLNOTOPEN)
#define	BUDDY(i,depth)	(depth ? (i) ^ (1 << (depth - 1)) : 0)
#define	INITLEAFPAGE(dp,fid,pid,depth) \
	(dp)->fileid = fid; (dp)->thispage = pid; (dp)->nextpage = NULLPAGE;\
	(dp)->filecount = 0; (dp)->localdepth = depth 

/****************************************************************************/
/*         INTERNAL ROUTINES FOR SUPPORTING EXTENDIBLE HASHING              */
/****************************************************************************/

static hash(filename, bitlength)
char	*filename;	/* file name */
int	bitlength;	/* how many bits to use */

/* hash fouction : h(key) = sum(k1, k2,...) mod (2 ** bit length)
where key = k1.k2.k3...			 */
{
	register int	value;	/* accumulator */

	for (value = 0; *filename;) value += (int)(*(filename++));
	return(value % (1 << bitlength));	/* return rightmost bits */

} /* hash */


static create_directory(vol)
int	vol;		/* volume index */

/* This creates an empty file directory with one empty bucket
It is assumed that a buffer for the root page has been allocated */
{
	register int	i;		/* bucket index */
	PID		leafpid;	/* PID of the new leaf */

#ifdef TRACE
	if (checkset(&Trace2, tEXTHASH))
		printf("create_directory(vol=%d)\n", vol);
#endif

	/* initialize the root page (hash table) with one bucket */
	ALLOCPAGE(&ROOTFID(vol), NULL, &DROOTPID(vol));
	ALLOCPAGE(&ROOTFID(vol), &DROOTPID(vol), &leafpid);
	for (BUCKET(vol, 0) = leafpid.Ppage, i = 1; i < MAXBUCKETS; i++)
		BUCKET(vol, i) = NULLPAGE;
	INITLEAFPAGE(LEAF(vol), ROOTFID(vol), leafpid, 0);
	WRITEROOT(vol);		/* write root to volume */
	WRITELEAF(&leafpid, LEAF(vol));	/* write this page out */

	return(eNOERROR);

} /* create_directory */


static bucket_split(vol, h, dp)
int		vol;	/* volume index */
int		h;	/* index of the bucket to be split */
LEAFPAGE	*dp;	/* the leaf page to be split */

/* This splits bucket h (primary leaf) into two, and rehashes all entries */
{
	register	i, j;		/* indices */
	LEAFPAGE	*np;		/* pointer to the new leaf */
	PID		pid;		/* PID of the new page */

#ifdef TRACE
	if (checkset(&Trace2, tEXTHASH))
		printf("bucket_split(vol=%d,h=%d,dp=0x%x)\n", vol, h, dp);
#endif

	/* get a level 0 page and a temporary buffer for the new bucket */
	ALLOCPAGE(&(ROOTFID(vol)), &(dp->thispage), &pid);
	/* no need for shmAlloc as page is freed below */
	np = (LEAFPAGE *)malloc(sizeof(PAGE));
	if (np == NULL) return(e2NOMOREMEMORY);
	j = ++(dp->localdepth);	/* increment local depth */
	INITLEAFPAGE(np, ROOTFID(vol), pid, dp->localdepth);

	/* update hash table, half of the entries previously pointing to
	current bucket are now pointing to the new bucket            */
	for (i = BUDDY(h, j) % (1 << j); i < (1 << GDEPTH(vol)); i += (1 << j))
		BUCKET(vol, i) = pid.Ppage;

	/* rehash and relocate entries in the old bucket */
	for (i = 0; i < dp->filecount; i++)
	{
		j = hash(dp->filedesc[i].filename, GDEPTH(vol));
		if (BUCKET(vol, j) != BUCKET(vol, h))
		{ /* this entry belongs to new bucket */
			np->filedesc[np->filecount++] = dp->filedesc[i];
			if (--dp->filecount == 0)
				break;	/* no more entries in old bucket */
			dp->filedesc[i--] = dp->filedesc[dp->filecount];
		}
	}

	WRITELEAF(&(np->thispage), np);	/* write the new page out */
	(void)free((char *)np);	/* release the space */
	WRITEROOT(vol);			/* update root page */

#ifdef DEBUG
	if (checkset(&Trace2, tEXTHASH))
	{
		printf(" after spliting bucket %d:\n", h);
		WRITELEAF(&(dp->thispage), dp);	/* write the page out */
		dump_directory(vol);
	}
#endif
	return(eNOERROR);

} /* bucket_split */

static overflow_split(vol, dpptr)
int		vol;	/* volume index */
LEAFPAGE	**dpptr;/* in : overflow page to be splited
					out : page with at least an empty entry */
/* This finds an overflow page with an empty file descriptor entry on it,
or append a new overflow page to the chain if no pages are available.
in any case, the page that has empty entries on it will be return via dpptr */
{
	PID		newpid;		/* PID of the new overflow page */
	LEAFPAGE	*dp;		/* pointer to leaf page */

#ifdef TRACE
	if (checkset(&Trace2, tEXTHASH))
		printf("overflow_split(vol=%d, dp=0x%x)\n", vol, *dpptr);
#endif

	dp = *dpptr;	/* pointer to the page in concern */

	while (dp->nextpage != NULLPAGE && (dp->filecount >= MAXDESCRIPTORS))
	{
		READLEAF(vol, dp->nextpage, dp);	/* try next page */
	}

	if (dp->filecount >= MAXDESCRIPTORS)
	{	/* need new overflow page */
		ALLOCPAGE(&ROOTFID(vol), &(dp->thispage), &newpid);
		dp->nextpage = newpid.Ppage;	/* chain it up */
		WRITELEAF(&(dp->thispage), dp);	/* write this page out */
		/* set up the new overflow page in the same buffer */
		INITLEAFPAGE(dp, ROOTFID(vol), newpid, dp->localdepth);

#ifdef DEBUG
		if (checkset(&Trace2, tEXTHASH))
		{
			printf(" after appending a new overflow page \n");
			WRITELEAF(&(dp->thispage), dp);	/* set it dirty */
			dump_directory(vol);
		}
#endif
	}

	*dpptr = dp;	/* update pointer to overflow page */

	return(eNOERROR);

} /* overflow_split */


static bucket_merge(vol, h, dp)
TWO		vol;	/* volume index */
int		h;	/* hash value, current bucket index */
LEAFPAGE	*dp;	/* pointer to leaf page */

/* merge this bucket with its buddy if possible */
{
	LEAFPAGE	*bp;	/* pointer to buddy page buffer */
	register int	i, j;	/* tables indices */

#ifdef TRACE
	if (checkset(&Trace2, tEXTHASH))
		printf("bucket_merge(vol=%d, h=%d, dp=0x%x)\n", vol, h, dp);
#endif

	if (dp->localdepth == 0)
		return(eNOERROR);	/* only one bucket exists */

	/* read in the primary page of the buddy of the current bucket */
	/* no need for shmAlloc as page is freed below */
	bp = (LEAFPAGE *)malloc(sizeof(PAGE));
	if (bp == NULL) return(e2NOMOREMEMORY);
	PIDCLEAR(bp->thispage);
	READLEAF(vol, BUCKET(vol, BUDDY(h, dp->localdepth)), bp);

	if (bp->filecount + dp->filecount > MAXDESCRIPTORS ||
		bp->nextpage != NULLPAGE || bp->localdepth != dp->localdepth)
	{	/* different depth or not compressible, release buffers */
		(void)free((char *)bp);
		return(eNOERROR);
	}

	/* update hash table, all pointers to this bucket will point to buddy */
	j = (bp->localdepth)--;	/* decrement local depth */
	for (i = h % (1 << j); i < (1 << GDEPTH(vol)); i += (1 << j))
		BUCKET(vol, i) = bp->thispage.Ppage;
	WRITEROOT(vol);	/* update root page */

	/* move all entries to buddy */
	for (i = 0; i < dp->filecount; i++)
		bp->filedesc[bp->filecount++] = dp->filedesc[i];

	FREEPAGE(&(dp->fileid), &(dp->thispage));
	WRITELEAF(&(bp->thispage), bp);
	(void)free((char *)bp);

#ifdef DEBUG
	if (checkset(&Trace2, tEXTHASH))
	{
		printf(" after merging bucket %d with %d:\n", h, BUDDY(h, j));
		dump_directory(vol);	/* dump directory */
	}
#endif

	return(eNOERROR);

} /* bucket_merge */


static overflow_compress(vol, dp)
TWO		vol;	/* volume index */
LEAFPAGE	*dp;	/* pointer to leaf page */

/* This compress two overflow pages into one if possible */
{
	register int	i;	/* descriptor index */
	LEAFPAGE	*np;	/* pointer to next overflow page */

#ifdef TRACE
	if (checkset(&Trace2, tEXTHASH))
		printf("overflow_compress(vol=%d, dp=0x%x)\n", vol, dp);
#endif

	if (dp->nextpage == NULLPAGE) return(eNOERROR);	/* no next page */

	/* read in next overflow page */
	/* no need for shmAlloc as page is freed below */
	np = (LEAFPAGE *)malloc(sizeof(PAGE));
	if (np == NULL) return(e2NOMOREMEMORY);
	PIDCLEAR(np->thispage);
	READLEAF(vol, dp->nextpage, np);

	if (dp->filecount + np->filecount <= MAXDESCRIPTORS)
	{ /* there's enough room on the 1st page, lets compress */
		for (i = 0; i < np->filecount; i++)	/* compressing */
			dp->filedesc[dp->filecount++] = np->filedesc[i];
		dp->nextpage = np->nextpage;	/* update the overflow chain */
		FREEPAGE(&(np->fileid), &(np->thispage)); /*release disk space*/
		WRITELEAF(&(dp->thispage), dp);	/* write the page out */
	}

	(void)free((char *)np);	/* free the secondary buffer */

#ifdef DEBUG
	if (checkset(&Trace2, tEXTHASH))
	{
		printf(" after merging overflow pages in \n");
		dump_directory(vol);	/* dump directory */
	}
#endif

	return(eNOERROR);

} /* overflow_compress */


static dump_bkpages(vol, dp)
int		vol;	/* volume index */
LEAFPAGE	*dp;	/* pointer to a (primary) leaf page buffer */

/* Dump pages of the bucket (primary page + overflow pages)
whose primary page is pointed by dp			 */
{
	register int	j;	/* file descriptor index */

	printf("----------------------------------------------------------------\n");
	for (;;)  /* loop until end of overflow chain is reached */
	{
		printf("  FID=[%3d,%3d], PID=[%3d,%3d],",
			(dp->fileid).Fvolid, (dp->fileid).Ffilenum,
			(dp->thispage).Pvolid, (dp->thispage).Ppage);
		printf(" NEXT=[%3d], depth = %d,count = %2d\n",
			dp->nextpage, dp->localdepth, dp->filecount);

		if (dp->filecount != 0)
			for (j = 0; j < dp->filecount; j++)
			{  /* print one descriptor at a time */
				printf("    descriptor[%d]:", j);
				PRINTDESCRIPTOR(&(dp->filedesc[j]));
				printf("\n");
			}
		else printf("    < EMPTY PAGE >\n");

		if (dp->nextpage == NULLPAGE) break;	/* no more overflow pages */
		READLEAF(vol, dp->nextpage, dp);	/* read next page */
		printf("  --------------------------------------------------------\n");
	}

	return(eNOERROR);

} /* dump_bkpages */


static dump_directory(vol)
int	vol;		/* volume (table) index */

/* Dump the entire file directory on the given volume.
This routines is intended for debuging purpose
*/
{
	register int	i;	/* index */
	LEAFPAGE	*dp;	/* leaf page buffer pointer */

	/* no need for shmAlloc as page is freed below */
	dp = (LEAFPAGE *)malloc(sizeof(PAGE));
	if (dp == NULL) return (eNOERROR);
	PIDCLEAR(dp->thispage);

	printf("===============================================================\n");
	printf("  <<- HASH TABLE DUMP (global depth:%d) ->>", GDEPTH(vol));
	for (i = 0; i < (1 << GDEPTH(vol)); i++)
		printf("%s  bucket[%d] = %d,", (i % 4 == 0) ? "\n" : "\0",
		i, (ROOT(vol))->bucket[i]);  /* 4 on a line */
	printf("\n");

	/* print each bucket, and avoid printing the same one twice */
	for (i = 0; i < (1 << GDEPTH(vol)); i++) {
		READLEAF(vol, BUCKET(vol, i), dp);
		if (i < (1 << dp->localdepth)) (void)dump_bkpages(vol, dp);
	}
	printf("===============================================================\n");

	free((char *)dp);

	return(eNOERROR);

} /* dump_directory */


st_listdir(volid)
int	volid;		/* (system-wide) volume id */

/* print the entire file directory on the given volume.
This routines is intended for debuging purpose
*/
{
	int	j;	/* file descriptor index */
	int	i;	/* index */
	int	vol;	/* vloume index */
	LEAFPAGE	*dp;	/* leaf page buffer pointer */
	register FILEDESC	*fd;
	int status;

	SetLatch(&smPtr->level2Latch, procNum, NULL);

	/* no need for shmAlloc as page is freed below */
	dp = (LEAFPAGE *)malloc(sizeof(PAGE));
	if (dp == NULL) return(eNOERROR);
	PIDCLEAR(dp->thispage);

	FINDVOL(volid, vol);	/* translate vloume ID into table index */
	/* print each bucket, and avoid printing the same one twice */
	printf("  FileID|Owner|Perm|Pages| Card |First PID|Last PID |");
	printf("Root PID | Type|File Name\n");
	printf("-----------------------------------------------------");
	printf("--------------------------\n");
	for (i = 0; i < (1 << GDEPTH(vol)); i++)
	{
		READLEAF(vol, BUCKET(vol, i), dp);	/* read in the bucket */
		if (i >= (1 << dp->localdepth)) continue;
		for (;;)  /* loop until end of overflow chain is reached */
		{
			for (j = 0; j < dp->filecount; j++)
			{  /* print one descriptor at a time */
				fd = &(dp->filedesc[j]);
				printf("%3d:%3d|",
					fd->fileid.Fvolid, fd->fileid.Ffilenum);
				printf("%5d| %3o|%5d|%6d|", fd->owner,
					fd->flags, fd->numpages, fd->card);
				printf("%4d:%4d|%4d:%4d|%4d:%4d|",
					fd->firstpage.Pvolid, fd->firstpage.Ppage,
					fd->lastpage.Pvolid, fd->lastpage.Ppage,
					fd->rootpage.Pvolid, fd->rootpage.Ppage);
				printf("%s\n", fd->filename);
			}
			if (dp->nextpage == NULLPAGE)
				break;		/* no more overflow pages */
			READLEAF(vol, dp->nextpage, dp);  /* read next page */
		}
	}

	free((char *)dp);
	ReleaseLatch(&smPtr->level2Latch, procNum);
	return(eNOERROR);

} /* st_listdir */

/****************************************************************************/
/*         INTERNAL ROUTINES FOR SUPPORTING EXTENDIBLE HASHING              */
/****************************************************************************/



/****************************************************************************/
/*	  	START OF EXPORT ROUTINES                                    */
/****************************************************************************/

ST_mount(volid)
TWO	volid;	/* volume id */
/* This mounts the specified volume:
     If this is a brand new volume, then create an empty file directory;
     otherwise, root of the file directory is read in for future reference.
     Note : the root of a mounted volume is always cache in core

   Returns:
	None

   Side Effects:
	volume mounted, a new file directory may be created.

   Errors:
	e2TOOMANYVOLUMES : too many volumes mounted
	e2NOMOREMEMORY	 : not enough memory for buffering 
*/
{
	register int 	i, j;	/* volume/hash table indices */
	register int 	e;	/* for returned errors */

#ifdef TRACE
	if (checkset(&Trace2,tFILEDIR))
		printf("ST_mount(volid=%d)\n", volid);
#endif

	for (i = 0; i < MAXVOLS; i++)
	{
		if (VOLID(i) == volid) 
		{
			MOUNTCNT(i)++;  
			return(eNOERROR); /* already mounted */
		}
	}
	/* find a empty entry in volume table for this volume */
	for (i = 0; VOLID(i) != NOVOLID; i++)
		if (i == MAXVOLS) return(e2TOOMANYVOLUMES);
	VOLID(i) = volid;	/* put down volume id in table */
	MOUNTCNT(i) = 1; /* mounted by one process only */

	/* get address of file directory from volume header */
	e = io_findfiledir(volid, &DROOTPID(i), &ROOTFID(i));
	CHECKERROR(e);

	/* allocate a buffer for the root page (hash table) and read it in.
	   if the volume is a new one, then create a empty directory for it */
	if ((ROOT(i) = (DIRROOT *) shmAlloc(sizeof(PAGE))) == NULL)
		return(e2NOMOREMEMORY);
	if ((LEAF(i) = (LEAFPAGE *) shmAlloc(sizeof(PAGE))) == NULL)
		return(e2NOMOREMEMORY);
	PIDCLEAR(LEAF(i)->thispage);	/* mark the page invalid */

	if (DROOTPID(i).Ppage == NULLPAGE)	/* brand new volume ? */
	{	/* create a empty directory with one bucket (leaf) */
		e = create_directory(i);
		CHECKERROR(e);
		e = io_setfiledir(volid, &DROOTPID(i));
		CHECKERROR(e);
	}
	else
		READROOT(&DROOTPID(i), ROOT(i)); /* read in the directory */

	for(j = 0; j < MAXDEPTH && BUCKET(i, (1 << j)) != NULLPAGE; j++);
	GDEPTH(i) = j;	/* calculate global depth */

#ifdef DEBUG
	if (checkset(&Trace2,tEXTHASH))
		dump_directory(i);	/* dump the file directory */
#endif

	return(eNOERROR);

} /* ST_mount */

ST_dismount(volid, DevName)
TWO	volid;	/* volume id */
char	*DevName;

/* This dismounts the specified volume - closing any remaining open files

   Returns:
	None

   Side Effects:
	Free the memory buffer used by the root page

   Errors:
	e2VOLNOTOPEN : this volume is not mounted
*/
{
	register int	i;	/* volume index */
	int	e, j;

#ifdef TRACE
	if (checkset(&Trace2,tFILEDIR))
		printf("ST_dismount(volid=%d)\n", volid);
#endif

	FINDVOL(volid, i);	/* find the table entry for this volume */

#ifdef DEBUG
	if (checkset(&Trace2,tEXTHASH))
		dump_directory(i);	/* dump the directory */
#endif
	MOUNTCNT(i)--; /* decrement number of users who have the same 
				volume mounted */
	if (MOUNTCNT(i) > 0) return (eNOERROR);
	else
	{
	    /* current user was the last one with the volume mounted,
	    really dismount it */

	    shmFree((char *) ROOT(i)); /* release the buffer for root page */
	    shmFree((char *) LEAF(i));	/* release the buffer for leaf page */
	    ROOT(i) = NULL;
	    VOLID(i) = NOVOLID;		/* mark it empty */

	    for (j = 0; j < MAXOPENFILES; j++)
	    {
		 if ((smPtr->files[j].ptr != NIL) && (F_VOLUMEID(j) == volid))
		 {
				(void) st_closefile(j, FALSE);
		 }
	    }
	    e = bf_dismount(DevName);	
	    CHECKERROR_RLATCH(e,&smPtr->level2Latch, procNum);

	    e = io_dismount(DevName);
	    CHECKERROR_RLATCH(e,&smPtr->level2Latch, procNum);

	    return(eNOERROR);
	}
       
} /* ST_dismount */


ST_accessfiledesc(volid, filename, descriptor, accessflag)
TWO		volid;		/* volume id */
char		*filename;	/* file name */
FILEDESC	*descriptor;	/* pointer to a file desctiptor */
int		accessflag;	/* READ or WRITE */

/* Fetch/update the file descriptor for the specified file

   Returns:
	file descriptor via parameter if accessflag is READ

   Side Effects:
	some file directory entry modified if accessflag is WRITE

   Errors:
	e2NOSUCHFILE : no file by this name
	e2VOLNOTOPEN : the volume is not mounted
*/
{
	register		i, j, k;	/* volume/file descriptor indices */
	register		h;	/* hashed value, bucket index */
	register LEAFPAGE	*dp;	/* leaf page pointer*/

#ifdef TRACE
	if (checkset(&Trace2,tFILEDIR))
	{
		printf("ST_accessfiledesc(volid=%d, ", volid);
		printf("filename=%s, ", filename);
		if (accessflag == WRITE)
		{	
			PRINTDESCRIPTOR(descriptor)
			printf(", accessflag=WRITE)\n");
		}
		else	printf("accessflag=READ)\n");
	}
#endif

	FINDVOL(volid, i);	/* find volume index */
	h = hash(filename, GDEPTH(i));
	dp = LEAF(i);	/* use the primary buffer for leaf pages */
	READLEAF(i, BUCKET(i, h), dp);	/* read in primary page */

	for (; ;)
	{  /* this loop go thru pages in the bucket till the entry is found */
		for (j = 0; j < dp->filecount; j++)  /* search on this page */
			if (!strcmp(filename, dp->filedesc[j].filename))
			{  /* descriptor found, access it */
				if (accessflag == READ)
				{  /* return descriptor and free buffer */
					if (descriptor != NULL)
						*descriptor = dp->filedesc[j];
				}
				else
				{  /* update descriptor and set buffer dirty */
					dp->filedesc[j] = *descriptor;
					WRITELEAF(&(dp->thispage), dp);
				}
				return(eNOERROR);  /* return with success */
			}

		if (dp->nextpage == NULLPAGE)
			return(e2NOSUCHFILE);	/* overflow chain ends */
		READLEAF(i, dp->nextpage, dp);	/* try next overflow page */
	}
	
} /* ST_accessfiledesc */


ST_newfiledesc(volid, filename, fid, pff)
TWO	volid;		/* volume id */
char	*filename;	/* file name */
FID	*fid;		/* level 0 file ID */
TWO	pff;		/* page fill factor in % */

/* Create a new file descriptor in the file directory

   Returns:
	None

   Side Effects:
	a new file descriptor add to the file directory

   Errors:
	e2FILEALREADYEXISTS: this file already exists
	e2VOLNOTOPEN : the volume is not mounted
*/
{

	int		e;	/* for returned errors */
	int		h;	/* hashed value, bucket index */
	int		d;	/* old hash table size */
	register int	i, j;	/* tables indices */
	LEAFPAGE	*dp;	/* leaf page buffer pointer */
	register FILEDESC	*fdptr;	/* file descriptor pointer */

#ifdef TRACE
	if ( checkset(&Trace2,tFILEDIR) )
	{
		printf("ST_newfiledesc(volid=%d,filename=%s", volid, filename);
		printf(",fid="); PRINTFIDPTR(fid); printf(",pff=%d)\n", pff);
	}
#endif

	FINDVOL(volid, i);	/* find volume index */
	h = hash(filename, GDEPTH(i));
	if (ST_accessfiledesc(volid, filename, (FILEDESC *)NULL,READ)>=eNOERROR)
		return(e2FILEALREADYEXISTS);	/* file already exists */

	dp = LEAF(i);	/* use the primary buffer for leaves */
	READLEAF(i, BUCKET(i, h), dp);	/* read in primary page */

	/* find a empty file directory entry for this new file */
	for (e = eNOERROR; dp->filecount >= MAXDESCRIPTORS; )
	{	/* bucket full, split it */
		if (dp->localdepth < GDEPTH(i))
			e = bucket_split(i, h, dp);  /* split primary page */
		else
			if (GDEPTH(i) < MAXDEPTH)
			{	/* expand directory */
				d = 1 << (GDEPTH(i)++);
				for (j = 0; j < d; j++)
					BUCKET(i,j + d) = BUCKET(i, j);
				h = hash(filename, GDEPTH(i)); /* rehash */
#ifdef DEBUG
	if ( checkset(&Trace2,tEXTHASH) )
		printf("expand directory to depth =%d\n", GDEPTH(i));
#endif
			}
			else	/* find or create overflow entry */
				e = overflow_split(i, &dp);

		CHECKERROR(e);	/* any errors */
	}

	/* add one new file descriptor to this page */
	fdptr = &(dp->filedesc[dp->filecount++]);
	(void) strcpy(fdptr->filename, filename);
	fdptr->fileid = *fid;
	fdptr->filesize = 0;
	fdptr->filetype = DATAFILE;
	fdptr->flags = DEFAULT_PERM;
	fdptr->owner = sys_getuser();
	PIDCLEAR(fdptr->firstpage);
	PIDCLEAR(fdptr->lastpage);
	PIDCLEAR(fdptr->rootpage);
	fdptr->pff = pff;
	fdptr->numpages = fdptr->card = 0;
	WRITELEAF(&(dp->thispage), dp);	/* update this page */
	return(eNOERROR);

} /* ST_newfiledesc */


ST_zapfiledesc(volid, filename, fid)
TWO	volid;		/* volume id */
char	*filename;	/* file name */
FID	*fid;		/* out parameter for returning  level 0 file ID */

/* Remove this file descriptor from the file directory

   Returns:
	level 0 file ID (via parameter) for destorying physical file

   Side Effects:
	delete the file descriptor from the file directory

   Errors:
	e2NOSUCHFILE : no file by this name
	e2VOLNOTOPEN : the volume is not mounted
*/

{
	register int 	i, j;	/* indices */
	int		e;	/* fo returned errors */
	int		h;	/* hashed value, bucket index */
	LEAFPAGE	*dp;	/* leaf page pointer */

#ifdef TRACE
	if ( checkset(&Trace2,tFILEDIR) )
	{
		printf("ST_zapfiledesc(volid=%d,filename=%s", volid, filename);
		printf("fid="); PRINTFIDPTR(fid); printf(")\n");
	}
#endif

	FINDVOL(volid, i);	/* find volume index */
	h = hash(filename, GDEPTH(i));
	dp = LEAF(i);	/* use the primary buffer for leaf pages */
	READLEAF(i, BUCKET(i, h), dp);	/* read in the primary page */

	for (;;)
	{  /* this loop go thru pages of the bucket till the descriptor found */
		for (j = 0; j < dp->filecount; j++)
			if (!strcmp(filename, dp->filedesc[j].filename)) break;
		if (j < dp->filecount)
		{ /* entry found, remove it and exit the loop */
			*fid = dp->filedesc[j].fileid;  /* return fid first! */
			/* check owner */
			dp->filedesc[j] = dp->filedesc[--(dp->filecount)];
			break;
		}

		if (dp->nextpage == NULLPAGE)
			return(e2NOSUCHFILE);	/* end of overflow chain */
		READLEAF(i, dp->nextpage, dp);	/* try next overflow page */
	}
		
	WRITELEAF(&(dp->thispage), dp);	/* update this page */

	if (dp->filecount >= LOWBOUND)
		return(eNOERROR);	 /* bucket still full enough */

	/* merge pages if possible: if current page is primary without overflow
	   chain, try merging buckets; otherwise compress overflow pages */
	if(BUCKET(i, h) == (dp->thispage).Ppage && dp->nextpage == NULLPAGE)
		e = bucket_merge(i, h, dp);
	else 	e = overflow_compress(i, dp);

	return(e);

} /* ST_zapfiledesc */


