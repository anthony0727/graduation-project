
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



/* Module st_deletehash - delete an hash entry from a hash file (index)

   IMPORTS :
    io_freepage(fileid, pid)
    bf_readbuf(trans_id, filenum, fid, pageid, returnpage)
    bf_freebuf(filenum, pageid, pageptr)
    bf_discard(filenum, pageid, pageptr)
    bf_setdirty(filenum, pageid, pageptr)
    bt_binary_search(key_len, key_value, pageptr, slotnum@)
    bt_insertentry(filenum, pageptr, length, buff, slotnum)
    bt_zaprid(filenum, pageptr, slotnum, ridptr, trans_id, lockup, cond)
    h_hash(key_value, key_length, num_bits)

   EXPORTS:
    st_deletehash(filenum, key, h_rid, trans_id, lockup, cond)
*/

#include    <wiss.h>
#include    <st.h>
#include    <lockquiz.h>

#define    PAGELOWBOUND(dp)	(MAXSPACE(dp)/2)
#define    CHECKRELEASE(a,b)   if ((int)(a) < eNOERROR) \
       printf ("in deletehash, m_release_page of page %d.%d got error %d\n",\
           (b).Pvolid, (b).Ppage, (a));

st_deletehash(filenum, key, h_rid, trans_id, lockup, cond)
int    filenum;	/* which hash file to delete */
KEY    *key;	    /* key of the hash entry */
RID    *h_rid;	    /* which record ID */
int    trans_id;
short    lockup;
short    cond;

/* Delete an entry from a hash file (index).

   Returns:
    None

   Side Effects:
    None

   Errors:
    e2KEYNOTFOUND
*/
{
    register int 	i, j;	/* indices */
    TWO	    k;
    int	    e, er;	/* for returned errors */
    int	    h;	/* hashed value, bucket index */
    TWO	    slotnum; /* where on the page */
    PID	    hpid, pid; /* page id templete */
    ROOTPAGE	*ht;	/* the hash table */
    BTREEPAGE 	*dp, *np; /* bucket page buffer pointers */
    PID	currentPid; 	/* when a page is freed, the pid must be
    	    	         kept to release the lock on this page  */
    FID fid;



#ifdef TRACE
    if ( checkset(&Trace2,tINTERFACE) ) {
    	printf("st_deletehash(filenum=%d, key=", filenum);
    	PRINTKEY(key); printf("rid=");
    	PRINTRIDPTR(h_rid); printf(")\n");
    }
#endif

    CHECKOFN(filenum);
    CHECKWP(filenum);

    fid=F_FILEID(filenum);

    /* get the hash table */
    hpid = F_ROOTPID(filenum);

    /* lock the root page in S mode is sufficient, as long as the rootpage
     * is not modified. If the rootpage must be modified, the 
     * lock will be upgraded.
     */
    if (lockup) {
        e = lock_page(trans_id, fid, hpid, l_S, MANUAL, cond);
        if (e == ABORTED) return (ABORTED);
    }

    e = bf_readbuf(trans_id, filenum, FC_FILEID(filenum), &hpid, (PAGE **)&ht);
    if (e < eNOERROR) goto error;

    currentPid.Ppage = NULLPAGE;
    
    /* locate the key */
    h = h_hash(key->value, key->length, GLOBALDEPTH(filenum));

    for (pid.Pvolid = hpid.Pvolid, pid.Ppage = ht->bucket[h]; 
    	pid.Ppage != NULLPAGE; pid.Ppage = dp->btcontrol.next) 
    {
       	if (lockup) {
      	    e = lock_page(trans_id, fid, pid, l_X, MANUAL, cond);
      	    if (e == ABORTED) return (ABORTED);
	    if (currentPid.Ppage != NULLPAGE) 
	    {
	    	er = m_release_page(trans_id, currentPid);
		CHECKRELEASE(er, currentPid);
	    }
	    currentPid = pid;
      	}

    	e = bf_readbuf(trans_id, filenum, FC_FILEID(filenum), &pid, 
		(PAGE **)&dp);
    	if (e < eNOERROR) break; /* something wrong */

    	e = bt_binary_search( key->length, key->value, dp, &slotnum);
    	if (e >= eNOERROR) break;	/* key found */

    	(void) bf_freebuf(filenum, &pid, (PAGE *)dp);
    }

    if (e < eNOERROR) 
    {
    	(void) bf_freebuf(filenum, &pid, (PAGE *)dp);
	if (lockup)
	{
	    er = m_release_page(trans_id, currentPid);
	    CHECKRELEASE(er, currentPid);
	}
	goto error;
    }
	
    /* unfix the hash table buffer */
    (void) bf_freebuf(filenum, &hpid, (PAGE *)ht);

    if (pid.Ppage == NULLPAGE) 
    {
	if (lockup)
	{
	    if (currentPid.Ppage != NULLPAGE) 
	    {
		er = m_release_page(trans_id, currentPid);
		CHECKRELEASE(er, currentPid);
	    }
	    er = m_release_page(trans_id, hpid);
	    CHECKRELEASE(er, hpid);
	}
	return(e2KEYNOTFOUND);
    }

    /* delete the index from a bucket */
    e = bt_zaprid(filenum, dp, slotnum, h_rid, trans_id, lockup, cond);

    if (e < eNOERROR) 
    {
    	(void) bf_freebuf(filenum, &pid, (PAGE *)dp);
	if (lockup)
	{
	    er = m_release_page(trans_id, currentPid);
	    CHECKRELEASE(er, currentPid);
	}
	goto error;
    }
	
    if ((LOCALDEPTH(dp) == 0) || (FREESPACE(dp) <= PAGELOWBOUND(dp))) 
    {
    	e = bf_freebuf(filenum, &(dp->btcontrol.thispage), (PAGE *)dp);
	if (e < eNOERROR) printf("at point 1 bf_freebuf returned error %d\n",e);
	if (lockup)
	{
	    /* release locks on root pid and current datapage */
	    er = m_release_page(trans_id, hpid);
	    CHECKRELEASE(er, hpid);
	    er = m_release_page(trans_id, currentPid);
	    CHECKRELEASE(er, currentPid);
	}
    	return(eNOERROR);
    }
    else 
    { 
	/* going to reorganize the data structure so upgrade lock on the
	root page */
       	if (lockup) {
            e = lock_page(trans_id, fid, hpid, l_X, MANUAL, cond);
            if (e == ABORTED) return (ABORTED);
        }
	/* read the hash table back again */
    	e = bf_readbuf(trans_id, filenum, FC_FILEID(filenum), &hpid, 
		(PAGE **)&ht);
    	if (e < eNOERROR) 
	{
    	    (void) bf_freebuf(filenum, &dp->btcontrol.thispage, (PAGE *)dp);
	    if (lockup)
	    {
	        /* release locks on root pid and current datapage */
	        er = m_release_page(trans_id, currentPid);
	        CHECKRELEASE(er, currentPid);
	    }
    	    goto error;
    	}
    }

    /* assertion.  at this point we are hold an exclusive locks on the root 
    page and the page from which the index value was deleted identified
    by hpid and currentPid respectively (pid and currentPid should be equal 
    at this pt) */

    /* storage reorganization */
    if (ht->bucket[h] == pid.Ppage && dp->btcontrol.next == NULLPAGE) 
    {
        /* merge two primary buckets (if possible) */

    	/* lock and read in the buddy of the current bucket */
    	pid.Ppage = ht->bucket[BUDDY(h, LOCALDEPTH(dp))];

	if (lockup)
	{
            e = lock_page(trans_id, fid, pid, l_X, MANUAL, cond);
            if (e == ABORTED) return (ABORTED);
	}

    	e = bf_readbuf(trans_id, filenum, FC_FILEID(filenum), &pid, 
		(PAGE **) &np);
    	if (e < eNOERROR) 
	{
    	    (void) bf_freebuf(filenum, &dp->btcontrol.thispage, (PAGE *)dp);
	    if (lockup)
	    {
	        er = m_release_page(trans_id, pid);
	        CHECKRELEASE(er,pid);
	        er = m_release_page(trans_id, currentPid);
	        CHECKRELEASE(er,currentPid);
	    }
	    /* there is still a lock on hpid */
    	    goto error;
    	}

    	if (np->btcontrol.next!=NULLPAGE || LOCALDEPTH(dp) !=
    	  LOCALDEPTH(np) || FREESPACE(dp)+FREESPACE(np)<=MAXSPACE(dp)) 
	{
    	    (void) bf_freebuf(filenum, &dp->btcontrol.thispage, (PAGE *)dp);
    	    (void) bf_freebuf(filenum, &np->btcontrol.thispage, (PAGE *)np);
	    if (lockup)
	    {
	        er = m_release_page(trans_id, pid);
	        CHECKRELEASE(er,pid);
	        er = m_release_page(trans_id, currentPid);
	        CHECKRELEASE(er,currentPid);
	    }
	    /* there is still a lock on hpid */
    	    goto done;	/* not mergable */
    	}

    	/* move all entries to buddy */
    	for(i = dp->btcontrol.numoffsets - 1; i >= 0; i--) 
	{
    	    (void)bt_binary_search(KEYLEN(dp, i), KEYVALUE(dp, i), np, &slotnum);
    	    KEYRIDLEN(dp, i, k);
    	    (void) bt_insertentry(filenum, np, k, ENTADDR(dp, i), slotnum);
    	}

    	e = bf_discard(filenum, &dp->btcontrol.thispage, (PAGE *)dp);
	if (e<0) printf("discard of pid %d.%d got error %d\n",
		dp->btcontrol.thispage.Pvolid, dp->btcontrol.thispage.Ppage, e);
	if (lockup)
	{
	    er = m_release_page(trans_id, currentPid);
	    CHECKRELEASE(er,currentPid);
	}
    	e = io_freepage(&(F_FILEID(filenum)), &dp->btcontrol.thispage);
    	if (e < eNOERROR) 
	{
		printf("freepage of pid %d.%d got error %d\n",
		dp->btcontrol.thispage.Pvolid, dp->btcontrol.thispage.Ppage, e);
		goto error;
	}
    	F_NUMPAGES(filenum)--, F_STATUS(filenum) = DIRTY;

	/* decrement local depth of the buddy */
    	k = 1 << (LOCALDEPTH(np))--, j = 1 << GLOBALDEPTH(filenum);
    	pid.Ppage = np->btcontrol.thispage.Ppage;

    	/* update hash table */
    	for (i = h % k; i < j; i += k) ht->bucket[i] = pid.Ppage;
	/* mark  page containing the hash table as dirty */
    	(void) bf_setdirty(filenum, &hpid, (PAGE *)ht);

	/* mark buddy dirty and then unfix it */
/* following call is new -  dewitt */
    	(void) bf_setdirty(filenum, &np->btcontrol.thispage, (PAGE *)np);
    	e = bf_freebuf(filenum, &np->btcontrol.thispage, (PAGE *)np);
	if (e<0) printf("discard of pid %d.%d got error %d\n",
		np->btcontrol.thispage.Pvolid, np->btcontrol.thispage.Ppage, e);
	if (lockup)
	{
	    er = m_release_page(trans_id, pid);
	    CHECKRELEASE(er,pid);
	}

	/* there is still a lock on hpid */
    }
    else 
    { 
	/* merge overflow pages if possible */
    	if ((pid.Ppage = dp->btcontrol.next) == NULLPAGE) 
	{
    	    (void) bf_freebuf(filenum, &dp->btcontrol.thispage, (PAGE *)dp);
	    if (lockup)
	    {
	        er = m_release_page(trans_id, currentPid);
	        CHECKRELEASE(er, currentPid);
	    }
    	    goto done;	/* no next page */
    	}

        if (lockup) 
	{
            e = lock_page(trans_id, fid, pid, l_X, MANUAL, cond);
            if (e == ABORTED) return (ABORTED);
        }

    	/* read in next overflow page */
    	e = bf_readbuf(trans_id, filenum, FC_FILEID(filenum), &pid, (PAGE **) &np);
    	if (e < eNOERROR) 
	{
    	    (void) bf_freebuf(filenum, &dp->btcontrol.thispage, (PAGE *)dp);
	    if (lockup)
	    {
	        er = m_release_page(trans_id, currentPid);
	        CHECKRELEASE(er, currentPid);
	        er = m_release_page(trans_id, pid);
	        CHECKRELEASE(er, currentPid);
	    }
    	    goto error;
    	}
    	if (FREESPACE(dp) + FREESPACE(np) <= MAXSPACE(dp)) {
    	    (void) bf_freebuf(filenum, &dp->btcontrol.thispage, (PAGE *)dp);
    	    (void) bf_freebuf(filenum, &np->btcontrol.thispage, (PAGE *)np);
	    if (lockup)
	    {
	        er = m_release_page(trans_id, currentPid);
	        CHECKRELEASE(er, currentPid);
	        er = m_release_page(trans_id, pid);
	        CHECKRELEASE(er, pid);
	    }
    	    goto done; /* not compressible */
    	}
    
    	/* move all entries to the previous page */
    	for(i = np->btcontrol.numoffsets - 1; i >= 0; i--) 
	{
    	    (void)bt_binary_search(KEYLEN(np, i), KEYVALUE(np, i), dp, &slotnum);
    	    KEYRIDLEN(np, i, k);
    	    (void) bt_insertentry(filenum, dp, k, ENTADDR(np, i), slotnum);
    	}
    	dp->btcontrol.next = np->btcontrol.next;
/* following call is new -  dewitt */
    	(void) bf_setdirty(filenum, &dp->btcontrol.thispage, (PAGE *)dp);
    	(void) bf_freebuf(filenum, &dp->btcontrol.thispage, (PAGE *)dp);
    	(void) bf_discard(filenum, &np->btcontrol.thispage, (PAGE *)np);

	if (lockup)
	{
	    er = m_release_page(trans_id, currentPid);
	    CHECKRELEASE(er, currentPid);
	    er = m_release_page(trans_id, pid);
	    CHECKRELEASE(er, pid);
	}
    	e = io_freepage(&(F_FILEID(filenum)), &np->btcontrol.thispage);
    	F_NUMPAGES(filenum)--, F_STATUS(filenum) = DIRTY;
    }

done:
error:
    /* unfix the buffer for the hash table */
    (void) bf_freebuf(filenum, &hpid, (PAGE *)ht);

    /* release locks on root pid */
    if (lockup)
    {
        er = m_release_page(trans_id, hpid);
        CHECKRELEASE(er,hpid);
    }
    return(e);

} /* st_deletehash */

