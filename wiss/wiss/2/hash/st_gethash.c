
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
/* Module st_gethash - locate an hash entry and return the associated pointer

   IMPORTS :
	bf_readbuf(trans_id, filenum, fid, pageid, returnpage)
	bf_freebuf(filenum, pageid, pageptr)
	bt_binary_search(key_len, key_value, pageptr, slotnum@)
	h_hash(key_value, key_length, num_bits)
	bt_getrid(filenum, pageptr, slotnum, offset, rirdptr, trans_id, lockup,
		cond)

   EXPORTS:
	st_gethash(filenum, key, Xcursor, h_rid, trans_id, lockup, cond)
*/

#include	<wiss.h>
#include	<st.h>
#include	<lockquiz.h>

/* Locking is assumed that the index is just being read and will
not be updated.  If the the st_gethash is followed by a delete,
the delete code will lock the index appropriately 
*/

st_gethash(filenum, key, Cursor, h_rid, trans_id, lockup, cond)
int	filenum;	/* which hash file to search */
KEY	*key;		/* key of the hash entry */
XCURSOR	*Cursor;	/* a cursor to a hash file */
RID	*h_rid;		/* for returning the pointer */
int	trans_id;
short	lockup;
short	cond;

/* Look up a key in a hash file

   Returns:
	the pointer (RID) assoicated with the key

   Side Effects:
	None

   Errors:
	e2NULLRIDPTR
	e2KEYNOTFOUND
*/
{
	register int	length;	/* key length */
	register char	*value;	/* key value */
	int		e;	/* for returned errors */
	int		h;	/* hashed value, bucket index */
	TWO		slotnum;/* which entry on the page */
	PID		hpid, pid, oldpid; /* page id templete */
	ROOTPAGE	*ht;	/* the hash table */
	BTREEPAGE	*dp;	/* leaf page pointer*/
	FID		hfid;	/* hash file fid */
	PID		lockpid; /* the data pageid to lock */
	RID		tmprid;

#ifdef TRACE
	if ( checkset(&Trace2,tINTERFACE) ) {
		printf("st_gethash(filenum=%d, key=", filenum);
		PRINTKEY(key); printf(", ridptr=0x%x)\n", h_rid); 
	}
#endif
	
	/* check file # */
	CHECKOFN(filenum);
	if (h_rid == NULL) return(e2NULLRIDPTR);

	hfid = FC_FILEID(filenum);

	/* get the hash table */
	hpid = F_ROOTPID(filenum);

	/* lock the root of the index in S mode */
	if (lockup) {
	  	e = lock_page(trans_id, hfid, hpid, l_S, MANUAL, cond);
	  	if (e < eNOERROR) return (e);
	}
	e = bf_readbuf(trans_id, filenum, hfid, &hpid, (PAGE **)&ht);
	CHECKERROR(e);

	oldpid.Ppage = NULLPAGE;

	/* locate the entry */
	value = key->value;
	length = key->length;
	h = h_hash(value, length, GLOBALDEPTH(filenum));

	for (pid.Pvolid = hpid.Pvolid, pid.Ppage = ht->bucket[h]; 
		pid.Ppage != NULLPAGE; pid.Ppage = dp->btcontrol.next) 
	{
	    if (lockup) 
	    {
		/* first lock the new page */
	        e = lock_page(trans_id, hfid, pid, l_S, MANUAL, cond);
		/* modified by PHJ */
	        if (e < eNOERROR) {
			if (oldpid.Ppage != NULLPAGE)
				m_release_page(trans_id, oldpid);
			m_release_page(trans_id, hpid);
			return (e);
		}

		/* then release the lock on the old page */
	        if (oldpid.Ppage != NULLPAGE) 
		{
	            e = m_release_page(trans_id, oldpid);
	            if (e != OK) 
    	    	    printf("\nin st_gethash, m_release_page returned error %d\n", e);
	        }
		oldpid = pid;
	    }

	    e = bf_readbuf(trans_id, filenum, hfid, &pid, (PAGE **)&dp);

	    if (e < eNOERROR) break;
	    e = bt_binary_search(length, value, dp, &slotnum);

	    if (e >= eNOERROR) break;
	    (void) bf_freebuf(filenum, &pid, (PAGE *)dp);
	}
	(void) bf_freebuf(filenum, &hpid, (PAGE *)ht);
	CHECKERROR(e);

	if (pid.Ppage == NULLPAGE) 
	{
	    if (lockup) {
	       e = m_release_page(trans_id, hpid);
	       if (e != OK) 
    	        printf("\nin st_gethash, m_release_page returned error %d\n", e);
	    }
	    return(e2KEYNOTFOUND);
	}

	/* prepare the cursor position of the first index */
	if (Cursor != NULL) {
		Cursor->pageid = dp->btcontrol.thispage;
		Cursor->slotnum = slotnum;
		Cursor->offset = 0;
	}

	/* get the first RID of the key we just located */
	e = bt_getrid(filenum, dp, slotnum, 0, h_rid, trans_id, lockup,
		cond);

	(void) bf_freebuf(filenum, &dp->btcontrol.thispage, (PAGE *)dp);

        /* Finally lock the datapage containing the record pointed at by
	   h_rid. We don't know what file it is in at this point so
	   we pass lock_page a NullFid. Note, we do not release the
	   lock on the index page pointed at by Cursor->pageid at this 
	   point as st_nexthash's correct operation depends on it being
	   locked 
	 */
/*
	  I deleted the following call in the systemV version of
	  wiss, because it really is not correct to lock a page with
	  a nullfid.  the o2 version of wiss did support this form of
	  locking (ie. a NullFID) in order to handle objects referencing
	  other objects without knowing what file they were in.

	  Probably the really correct solution would be to modify
	  wiss so that the the index knows what file the records reside
	  in.  THe problem with this solution is that it then indices
	  cannot span multiple files!!
*/

/*
	if(lockup) {
	    tmprid = *h_rid;
	    GETPID(lockpid, tmprid);
	    if ((e = lock_page(trans_id, NullFID, lockpid, l_S, COMMIT, cond))
		< eNOERROR) return (e);
	}
*/
	/* modified by PHJ */
	if (lockup) {
	       e = m_release_page(trans_id, pid);
	       if (e != OK) 
    	        printf("\nin st_gethash, m_release_page returned error %d\n", e);
	       e = m_release_page(trans_id, hpid);
	       if (e != OK) 
    	        printf("\nin st_gethash, m_release_page returned error %d\n", e);
	}
	return(e);
	
} /* st_gethash */

