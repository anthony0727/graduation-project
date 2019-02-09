
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


#include	<d_error.h>
#include 	<st_fd.h>
/* definitions internal to level 2. - file directory and file table */

typedef struct	{
	TWO	status; 	/* status of table entry */
	TWO	volid; 		/* volume identifier */
	short	ref_count;	/* # of references to the file */
	short	accessmode;	/* READ or WRITE */
	FOUR	lockmode;	/* lock mode of the file */
	FILEDESC	desc; 	  /* actual file descriptor */
	LATCH	fileLatch;	/* latch for individual file */
} filetable_s;

typedef struct	{
	FOUR	user; 		/* who is using the file */
	short	mode;		/* READ or WRITE */
	short	ptr;		/* index to the open file table */
	PID     lastpage;       /* last page accessed by user */
	int     lastpinned;     /* TRUE or FALSE */
	int     lastlocked;     /* TRUE or FALSE */
	int     lockmode;       /* a valid lock mode */
	char     *pagebuf;     /* addr of pinned page in buffer pool */
} files_s;
 /* The  set of fields in file_s starting with lastpage were
    added in order to enhance the performance when appending or
    scanning a file.  The basic idea is that when a file is
    opened by a user, lastpage is set to NULLPID,
    and lastlocked, and lastpinned are set to FALSE.  As a page
    is accessed while appending or scanning a file,  instead of
    repeatedly pining and unpining a page for each record on the page,
    the page is pinned until the the scan or appendle open and 
    you don't want users to interfere with each other. 
    Ditto if one user has multiple scans open on the same
    file simultaneously */



/* some short hands for accessing the open file table */
#define F_USER(i)	smPtr->files[i].user
#define F_PERM(i)	smPtr->files[i].mode
#define F_LASTPIDPINNED(i)  smPtr->files[i].lastpage
#define F_LASTPINNED(i)  smPtr->files[i].lastpinned
#define F_LASTLOCKED(i) smPtr->files[i].lastlocked
#define F_LOCKMODE(i)   smPtr->files[i].lockmode
#define F_BUFADDR(i)    smPtr->files[i].pagebuf
#define F_ACCESSMODE(i)	smPtr->filetable[smPtr->files[i].ptr].accessmode
#define F_STATUS(i)	smPtr->filetable[smPtr->files[i].ptr].status
#define F_VOLUMEID(i)	smPtr->filetable[smPtr->files[i].ptr].volid
#define F_LATCHPTR(i)   &smPtr->filetable[smPtr->files[i].ptr].fileLatch
#define F_DESC(i)	smPtr->filetable[smPtr->files[i].ptr].desc
#define F_FILENAME(i)	smPtr->filetable[smPtr->files[i].ptr].desc.filename
#define F_FILESIZE(i)	smPtr->filetable[smPtr->files[i].ptr].desc.filesize
#define F_FILETYPE(i)	smPtr->filetable[smPtr->files[i].ptr].desc.filetype
#define F_FLAGS(i)	smPtr->filetable[smPtr->files[i].ptr].desc.flags
#define F_OWNER(i)	smPtr->filetable[smPtr->files[i].ptr].desc.owner
#define F_FIRSTPID(i)	smPtr->filetable[smPtr->files[i].ptr].desc.firstpage
#define F_LASTPID(i)	smPtr->filetable[smPtr->files[i].ptr].desc.lastpage
#define F_ROOTPID(i)	smPtr->filetable[smPtr->files[i].ptr].desc.rootpage
#define F_FILEID(i)	smPtr->filetable[smPtr->files[i].ptr].desc.fileid
#define FC_FILEID(i)	(i < 0) ? NullFID : smPtr->filetable[smPtr->files[i].ptr].desc.fileid
#define	F_PFF(i)	smPtr->filetable[smPtr->files[i].ptr].desc.pff
#define	F_NUMPAGES(i)	smPtr->filetable[smPtr->files[i].ptr].desc.numpages
#define	F_CARD(i)	smPtr->filetable[smPtr->files[i].ptr].desc.card
#define	F_REFCOUNT(i)	smPtr->filetable[smPtr->files[i].ptr].ref_count
#define	F_SERVEROFN(i)	smPtr->filetable[smPtr->files[i].ptr].desc.serverofn
