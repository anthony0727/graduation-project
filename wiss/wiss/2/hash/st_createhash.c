
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


/* Module st_createhash - create a hash file (index)

   IMPORTS :
	io_allocpages(fileid, nearpid, num_pages, pidarray)
	bf_getbuf(trans_id, filenum, fid, pageid, returnpage)
	bf_freebuf(filenum, pageid, pageptr)
	h_initpage(filenum, pid, pageptr, key_type, unique)
	h_inserthash(filenum, key, rid, trans_id)
	r_getrecord(filenum, ridptr, returnpage, returnrecptr, trans_id, 
		lockup, mode, cond)
	st_recordcard(vol, filename)
	st_createfile(vol, filename)
	int st_openfile(vol, filename)
	st_closefile(filenum, TRUE)
	st_firstfile(filenum, ridptr, trans_id, lockup, mode, cond)
	st_nextfile(filenum, currentrid, nextrid, trans_id, lockup, mode, cond)

   EXPORTS:
	st_createhash(VolID, filename, hashno, keyattr, fillfactor, Unique,
		trans_id, lockup, cond)
    HISTORY:
	5/11/90 dewitt, replaced call to st_recordcard() to get number
		of records in filename with macro F_CARD() which accesses
		open file directly - need for filedesc consistency purposes
*/

#include	<wiss.h>
#include	<st.h>
#include        <lockquiz.h>

extern	char *malloc();
extern  free();

st_createhash(VolID, filename, hashno, KeyAttr, FillFactor, Unique, 
	trans_id, lockup, cond)
int	VolID;		/* volume identifier */
char	*filename;	/* name of the data file */
int	hashno;		/* hash (index) # */
KEYINFO *KeyAttr;	/* attributes of the key */
int	FillFactor;	/* how full should each bucket be initially? */
int	Unique;		/* is the key field unique? */
int     trans_id;        
short   lockup;
short   cond;

/* This creates a hash file (index) on a given data file 

   Returns :
	None

   Side Effects:
	None

   Errors:
	None
*/
{
	int		filenum;	/* file number of the data file */
	int		h_filenum;	/* file # of the hash index */
	register int	e;		/* for returned errors */
	register int	i, j;		/* indices */
	int		numbuckets;	/* # of buckets initially */
	int		numrecords;	/* # of records initially */
	int		depth;		/* how deep is the directory */
	PID		*pids;		/* for returned page ids */
	RID		rid;		/* record id */
	RECORD		*recptr;	/* record pointer */
	ROOTPAGE	*ht;		/* hash table */
	BTREEPAGE	*dp;		/* bucket page pointer */
	DATAPAGE	*page;
	KEY		key;		/* key for calling h_insert */
	char		h_filename[MAXFILENAMELEN];
	FID		fid;

int	count;

#ifdef TRACE
	if (checkset(&Trace2,tINTERFACE)) {
		printf("st_createhash(VolID=%d, filename=%s, hashno=%d", 
			VolID, filename, hashno);
		printf("keyattr="); PRINTATTR(KeyAttr);
		printf("fillfactor=%d, unique=%c\n",FillFactor, Unique?'T':'F');
	}
#endif

	/* make sure the data file does exist */
	filenum = st_openfile(VolID, filename, READ);
	CHECKERROR(filenum);
	fid = FC_FILEID(filenum);

	/* lock the source file in S mode */
	if (lockup)
	    e = lock_file(trans_id, fid, l_S, COMMIT, cond);
	CHECKERROR(e);

	/* create a file for the hash index, and open it */
	(void) suffixname(h_filename, filename, HASHSUF, hashno);
	if (FillFactor <= 0 || FillFactor > 100) FillFactor = 100;
	e = st_createfile(VolID, h_filename, 2, 75, FillFactor);
	CHECKERROR(e);
	h_filenum = st_openfile(VolID, h_filename, WRITE);
	CHECKERROR(h_filenum);

	/* lock the hash output file in X mode */
	if (lockup)
	    e = lock_file (trans_id, FC_FILEID(h_filenum), l_X, COMMIT, cond);
	CHECKERROR(e);

	/* estimate how many pages to use initially */
	numrecords =  F_CARD(filenum); 
	numbuckets = numrecords /
		(((PAGESIZE*FillFactor)/100) / (KeyAttr->length+8+sizeof(RID)));
	if (numbuckets <= 0) numbuckets = 1;
	for (depth = 0; numbuckets > 0; depth++, numbuckets >>= 1);
	numbuckets = 1 << depth; /* make it power of 2 */
	pids = (PID *) malloc( (unsigned) (sizeof(PID) * (1 + numbuckets)) );
	
	/* initialize the root page (hash table) and the file descriptor */
	e = io_allocpages(&F_FILEID(h_filenum), (PID *)NULL,1+numbuckets,pids);
	CHECKERROR(e);
	e = bf_getbuf(trans_id, h_filenum, FC_FILEID(h_filenum), &pids[0], (PAGE **) &ht);
	CHECKERROR(e);
	F_ROOTPID(h_filenum) = pids[0]; /* remember the hash table */
	GLOBALDEPTH(h_filenum) = depth;
	F_NUMPAGES(h_filenum) = numbuckets + 1;
	F_FILETYPE(h_filenum) = HASHFILE;
	F_STATUS(h_filenum) = DIRTY;
	for (i = 0; i < numbuckets; i++) ht->bucket[i] = pids[i+1].Ppage;
	for (; i < MAXBUCKETS; i++) ht->bucket[i] = NULLPAGE;
	(void) bf_freebuf(h_filenum, &pids[0], (PAGE *) ht);

	/* initialize the buckets */
	for (i = 0; i < numbuckets; i++) {
		e = bf_getbuf(trans_id, h_filenum, FC_FILEID(h_filenum), &pids[1+i], (PAGE **) &dp);
		CHECKERROR(e);
		h_initpage(h_filenum, &pids[i+1], dp, KeyAttr->type, Unique);
		LOCALDEPTH(dp) = depth; /* use this field as "local depth" */
		(void) bf_freebuf(h_filenum, &pids[1+i], (PAGE *) dp);
	}
	free ( (char *) pids);

	/* for each record in the file, put a hash index into the hash file */
	i = KeyAttr->offset;
	j = key.length = KeyAttr->length;
	key.type = KeyAttr->type;

	/* since the source file is already locked in S mode,  st_firstfile() 
	st_nextfile(), and r_getrecord() all get passed FALSE for lockup */

	for (count = 0, 
		e = st_firstfile(filenum, &rid, trans_id, FALSE, l_NL, cond); 
	   e >= eNOERROR; 
	   e = st_nextfile(filenum, &rid, &rid, trans_id, FALSE, l_NL, cond), 
		count++) 
	{
		e = r_getrecord(filenum, &rid, &page, &recptr, trans_id, 
			FALSE, l_NL, cond);
		if (e < 0) printf("error return from getrecord = %d\n",e);
		CHECKERROR(e);
		movebytes(key.value, &(recptr->data[i]), j);
		e = h_inserthash(h_filenum, &key, &rid, trans_id, FALSE, cond);
		if (e < 0) printf("error return from h_inserth = %d\n",e);
		CHECKERROR(e);
		(void) bf_freebuf(filenum, &page->thispage, (PAGE *)page);
		CHECKERROR(e);
	}

	e = st_closefile(filenum, TRUE);
	CHECKERROR(e);
	e = st_closefile(h_filenum, TRUE);
	CHECKERROR(e);

	return(eNOERROR);

} /* st_createhash */
