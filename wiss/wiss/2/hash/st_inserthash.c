
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



/* Module st_inserthash - insert an hash entry into a hash file 

   IMPORTS :
    io_allocpages(fileid, nearpid, num_pages, pidarray)
    bf_readbuf(trans_id, filenum, fid, pageid, returnpage)
    bf_getbuf(trans_id, filenum, fid, pageid, pageptr)
    bf_setdirty(filenum, pageid, pageptr)
    bt_binary_search(key_len, key_value, pageptr, slotnum)
    bt_insertentry(filenum, pageptr, length, buff, slotnum)
    bt_addrid(filenum, pageptr, slotnum, ridptr, trans_id, lockup, cond)
    h_hash(key_value, key_length, num_bits)
    h_initpage(filenum, pid, pageptr, key_type, unique)

   EXPORTS:
    st_inserthash(filenum, key, h_rid, trans_id, lockup, cond)
    h_inserthash(filenum, key, h_rid, trans_id, lockup, cond)
*/

#include <wiss.h>
#include <st.h>
#include <lockquiz.h>

#ifdef    DEBUG
#undef    MAXDEPTH
#define    MAXDEPTH	2
#endif

st_inserthash(filenum, key, h_rid, trans_id, lockup, cond)
int    filenum;	/* which hash file to insert */
KEY    *key;	    /* key of the new hash entry */
RID    *h_rid;	    /* pointer of the new hash entry */
int    trans_id;
short   lockup;
short   cond;

/* Insert an entry into a hash file (index).

   Returns:
    None

   Side Effects:
    None

   Errors:
    e2NULLRIDPTR
    e2KEYALREADYEXISTS - duplicate keys for an index with unique keys
*/
{

#ifdef TRACE
    if ( checkset(&Trace2,tINTERFACE) ) {
    	printf("st_inserthash(filenum=%d, key=", filenum);
    	PRINTKEY(key); printf(", rid="); 
    	PRINTRIDPTR(h_rid); printf(")\n");
    }
#endif

    /* check file # and file permission */
    CHECKOFN(filenum);
    CHECKWP(filenum);
    if (h_rid == NULL) return(e2NULLRIDPTR);

    return(h_inserthash(filenum, key, h_rid, trans_id, lockup, cond));

} /* st_inserthash */


h_inserthash(filenum, key, h_rid, trans_id, lockup, cond)
int    filenum;	/* which hash file to insert */
KEY    *key;	    /* key of the new hash entry */
RID    *h_rid;	    /* pointer of the new hash entry */
int    trans_id;	
short   lockup;
short   cond;

/* Insert an entry into a hash file.
   Hash files are implemented by the extendible hashing algorithm
   with the hash table limited to one page in size.
   If the hash table has already taken up a whole page when a primary 
   bucket overflows, overflow pages are used to resolved the collision.
   Entries on a page are sorted in their key order.

   Returns:
    None

   Side Effects:
    None

   Errors:
    e2KEYALREADYEXISTS: dumplicate keys
*/
{
    register i, j;      /* tables indices */
    TWO	k;	        /* index */
    int	e,er;	        /* for returned errors */
    int	h;	        /* hashed key - bucket index */
    int	dup = 0;	/* Is the key already exists ? */
    int	global_depth;	/* global hash depth */
    int	length;	        /* total length of the key/RID pair */
    TWO	slotnum;	/* where to place the new entry */
    PID	pid;	        /* page id templete */
    ROOTPAGE *ht;	/* hash table of the hash file */
    BTREEPAGE *dp, *np;	/* bucket page buffer pointers */
    FID fid;		/* hash file */
    PID	currentPid; 	/* when a page is freed, the pid must be
    	    	         kept to release the lock on this page  */
    PID		oldpid;
    char	buf[MAXKEYLEN+2+2+sizeof(RID)];  /* buffer for packing key-RID pair */

#ifdef TRACE
    if ( checkset(&Trace2, tHASHFILE) ) {
    	printf("h_inserthash(filenum=%d, key=", filenum);
    	PRINTKEY(key); printf(", rid="); 
    	PRINTRIDPTR(h_rid); printf(")\n");
    }
#endif

    fid = FC_FILEID(filenum);

    /* read in the hash table of the file */
    pid = F_ROOTPID(filenum);
    
    /* a lock in S mode is sufficient, as long as the rootpage
     * is not modified. If the rootpage must be modified, the 
     * lock will be upgraded.
     */
    if (lockup) {
	/* modified by PHJ */
        e = lock_page(trans_id, fid, pid, l_X, MANUAL, cond);
        if (e < eNOERROR) return (e);
    }

    currentPid.Ppage = NULLPAGE;

    e = bf_readbuf(trans_id, filenum, fid, &pid, (PAGE **)&ht);
    CHECKERROR(e);
    global_depth = GLOBALDEPTH(filenum);

    /* look up the key */
    h = h_hash(key->value, key->length, global_depth);

    for (pid.Ppage = ht->bucket[h]; ; )
    {
       	if (lockup) {
      	    e = lock_page(trans_id, fid, pid, l_X, MANUAL, cond);
    	    if (e < eNOERROR) { /* modified by PHJ */
		if (currentPid.Ppage != NULLPAGE)
			m_release_page(trans_id, currentPid);
		m_release_page(trans_id, F_ROOTPID(filenum));
		return (e);
	    }
	    if (currentPid.Ppage != NULLPAGE) 
		m_release_page(trans_id, currentPid);
	    currentPid = pid;
      	}

    	e = bf_readbuf(trans_id, filenum, fid, &pid, (PAGE **)&dp);
    	if (e < eNOERROR) 
	{
	    if (lockup) m_release_page(trans_id, currentPid);
	    goto error;
	}
    	e = bt_binary_search(key->length, key->value, dp, &slotnum);
    	if ( (dup = e >= eNOERROR)) 
	{   
	    /* the key already exists */
    	    if (dp->btcontrol.unique) 
	    {
    	    	(void) bf_freebuf(filenum, &pid, (PAGE *)dp);
    	    	if (lockup) 
		{
    	    	    e = m_release_page(trans_id, currentPid);
    	    	    if (e != OK) 
    	    	    printf("\nh_inserthash, m_release_page returned error %d\n",
		       e);
    	    	}	    	      
    	    	e = e2KEYALREADYEXISTS;
    	    	goto error;
    	    }
    	    break;
    	}
    	if ((pid.Ppage = dp->btcontrol.next) == NULLPAGE) 
    	  break;
    	(void) bf_freebuf(filenum, &dp->btcontrol.thispage, (PAGE *)dp);
    }

    /* package the key-ptr pair into a buffer */
    if (dup) length = sizeof(RID) - sizeof(dp->slot[0]); /* compensate */
    else {	
    	MAKEKEYRID(buf, key->length, key->value, h_rid, length);
    	MAKEALIGN(length);	/* align to word boundary */
    }

    /* note at this point we are still holding locks on the root page
    of the index and the page pointed to by currentPid */

    if (FREESPACE(dp) >= length + sizeof(dp->slot[0])) goto insert;	

    do 
    { 
	/* current bucket full, split it or resort to overflow chain */
	if (LOCALDEPTH(dp) < global_depth) 
	{
       	    /* Split the current bucket and increase its local depth, 
    	     and update the hash table accordingly. 
    	    */

    	    /*
    	     * the rootpage lock must be upgraded
    	     */
    	    if (lockup) 
	    {
    	        e = lock_page(trans_id, fid, F_ROOTPID(filenum), l_X, 
    	    		MANUAL, cond);
    	        if (e < eNOERROR) { /* modified by PHJ */
			m_release_page(trans_id, currentPid);
			m_release_page(trans_id, F_ROOTPID(filenum));
			return (e);
		}
    	    }

    	    /* allocate and initialize a new bucket */
    	    e = io_allocpages(&(dp->btcontrol.fileid), &pid ,1, &pid);
    	    if (e < eNOERROR) goto error;
    	    if (lockup) 
	    {
    	         e = lock_page(trans_id, fid, pid, l_X, MANUAL, cond);
    	        if (e < eNOERROR) { /* modified by PHJ */
			m_release_page(trans_id, currentPid);
			m_release_page(trans_id, F_ROOTPID(filenum));
			return (e);
		}
    	    }

    	    e = bf_getbuf(trans_id, filenum, fid, &pid, (PAGE **)&np);
    	    if (e < eNOERROR) goto error;

    	    h_initpage(filenum, &pid, np, key->type, dp->btcontrol.unique);
    	    i = LOCALDEPTH(np) = ++(LOCALDEPTH(dp)); 
    	    F_NUMPAGES(filenum)++,
    	    F_STATUS(filenum) = DIRTY;
    
    	    /* update hash table, half of the entries previously pointing 
    	      to the old bucket are now pointing to the new bucket     */
    	     k = 1 << i;	/* distance to buddy (- i is the local depth)*/
    	     j = 1 << global_depth /* global table size */;
    	     for (i = BUDDY(h, i) % k; i < j; i += k) ht->bucket[i] = pid.Ppage;

    	     (void) bf_setdirty(filenum, &(F_ROOTPID(filenum)), (PAGE *)ht);

    	     /* relocate entries in the old bucket */
    	     for (slotnum = i = j = 0; i < dp->btcontrol.numoffsets; i++) 
	     {
    	         k = h_hash(KEYVALUE(dp,i), KEYLEN(dp,i), global_depth);
    	         if (ht->bucket[k] ==  ht->bucket[h]) 
	         { 
		     /* it stays */
    	    	     dp->slot[-j] = dp->slot[-i];
    	             j++;  /* j is the slot count for old bucket */
    	         }
    	         else 
	         { 
		     /* move to the new bucket */
    	    	     KEYRIDLEN(dp, i, k);	/* k <- entry length */
    	    	     bt_insertentry(filenum, np, k, ENTADDR(dp,i), slotnum++);
    	    	     MAKEALIGN(k);
    	    	     k += sizeof(dp->slot[0]);
    	    	     dp->btcontrol.numfree += k;
    	         }
    	     }
    	     dp->btcontrol.numoffsets = j;
    	     (void)bf_setdirty(filenum,&(dp->btcontrol.thispage),(PAGE *)dp);
    	     (void)bf_freebuf(filenum,&(np->btcontrol.thispage),(PAGE *)np);
    	     if (lockup) 
	     {
    	          er = m_release_page(trans_id, pid);
    	          if (er != OK) 
	              printf("inserthash, m_release_page had error %d\n", er);
    	     }

    	     /* recalculate the on-page position */
    	     (void) bt_binary_search(key->length, key->value, dp, &slotnum);

	} /* end of bucket splitting */

	else if (global_depth < MAXDEPTH) 
	{	
	    /* double the directory */

    	    /*
    	     * the rootpage lock must be upgraded
    	     */
    	    if (lockup) 
	    {
    	        e = lock_page(trans_id, fid, F_ROOTPID(filenum), l_X, MANUAL, 
			cond);
    	        if (e < eNOERROR) { /* modified by PHJ */
			m_release_page(trans_id, currentPid);
			m_release_page(trans_id, F_ROOTPID(filenum));
			return (e);
		}
    	    }

    	    i = 1 << global_depth; 	/* size of the old table */
    	    global_depth = ++GLOBALDEPTH(filenum);
    	    F_STATUS(filenum) = DIRTY;	/* descriptor modified */

    	    for (j = 0; j < i; j++) ht->bucket[j+i] = ht->bucket[j];
    	    (void) bf_setdirty(filenum, &F_ROOTPID(filenum), (PAGE *)ht);
    	    h = h_hash(key->value, key->length, global_depth); /* re-hash */

	} /* end of doubling the directory */
	else	
	{ 

	    /* we must resort to an overflow chain because the local depth */
	    /* is equal to the global depth and the global depth has reached */
	    /* its maximum */

	    /* find an overflow page that has enough room, if possible */
    	    oldpid = dp->btcontrol.thispage;

    	    while(((pid.Ppage = dp->btcontrol.next) != NULLPAGE) && 
    	            (FREESPACE(dp) < length + sizeof(dp->slot[0])))
	    {
    	        (void) bf_freebuf(filenum, &dp->btcontrol.thispage, (PAGE *)dp);

    	        if (lockup) 
		{
    	            e = lock_page(trans_id, fid, pid, l_X, MANUAL, cond);
    	            if (e < eNOERROR) { /* modified by PHJ */
			m_release_page(trans_id, currentPid);
			m_release_page(trans_id, F_ROOTPID(filenum));
			return (e);
		    }
    	            e = m_release_page(trans_id, currentPid);
    	            if (e != OK) 
		       printf("inserthash, m_release_page had error %d\n", e);
		    currentPid = pid;
    	        }	    	      

    	        e = bf_readbuf(trans_id, filenum, fid, &pid, (PAGE **)&dp);
    	        if (e < eNOERROR) goto error;
    	    }
	    /* still holding locks on currentPid and the file's root page */

    	    if (FREESPACE(dp) < length + sizeof(dp->slot[0])) 
	    {
    	        /* need a new overflow page */

    	        e = io_allocpages(&(F_FILEID(filenum)), &oldpid, 1, &pid);
    	        if (e < eNOERROR) 
		{
    	    	    (void) bf_freebuf(filenum, &dp->btcontrol.thispage, 
			(PAGE *)dp);
    	    	    if (lockup) 
		    {
    	                er = m_release_page(trans_id, currentPid);
    	                if (er != OK) 
		          printf("inserthash, m_release_page had error %d\n", 
				er);
    	    	    }
		    goto error;
    	        }
    	        dp->btcontrol.next = pid.Ppage;	/* chain it up */
    	        (void) bf_setdirty(filenum, &(dp->btcontrol.thispage),
			(PAGE *) dp);
    	        e = bf_getbuf(trans_id, filenum, fid, &pid, (PAGE **) &np);
    	        if (e < eNOERROR) 
		{
    	    	    (void) bf_freebuf(filenum, &dp->btcontrol.thispage, 
			(PAGE *)dp);
    	    	    if (lockup) 
		    {
    	                er = m_release_page(trans_id, currentPid);
    	                if (er != OK) 
		          printf("inserthash, m_release_page had error %d\n", 
				er);
    	    	    }
		    goto error;
    	        }

    	        if (lockup) 
		{
    	            e = lock_page(trans_id, fid, pid, l_X, MANUAL, cond);
    	            if (e < eNOERROR) { /* modified by PHJ */
			m_release_page(trans_id, currentPid);
			m_release_page(trans_id, F_ROOTPID(filenum));
			return (e);
		    }
    	        }

		/* at this point we have locks on the file's root page, 
		   currentPid, and pid */

    	        (void) h_initpage(filenum, &pid, np, key->type, 
			dp->btcontrol.unique);
    	        LOCALDEPTH(np) = LOCALDEPTH(dp);
    	        np->btcontrol.pagetype = OVFLPG;
    	        F_NUMPAGES(filenum)++;
    	        F_STATUS(filenum) = DIRTY;
    	        (void) bf_freebuf(filenum, &dp->btcontrol.thispage, (PAGE *) dp);
    	        if (lockup) 
		{
    	            er = m_release_page(trans_id, currentPid);
    	            if (er != OK) 
		         printf("inserthash, m_release_page had error %d\n", er);
		    currentPid = pid;
    	        }

		/* now we are down to locks on the file's root page and pid */

    	        /* move entry to new page for duplicate key */
    	        if (dup) 
		{
    	            if (lockup) 
		    {
    	    	        e = lock_page(trans_id, fid, oldpid, l_X, MANUAL, cond);
    	                if (e < eNOERROR) { /* modified by PHJ */
		    	    m_release_page(trans_id, currentPid);
			    m_release_page(trans_id, F_ROOTPID(filenum));
			    return (e);
		        }
    	    	    }

    	    	    e = bf_readbuf(trans_id, filenum, fid, &oldpid,(PAGE **)&dp);
    	    	    if (e < eNOERROR) 
    	    	    {
    	    	    	(void) bf_freebuf(filenum, &np->btcontrol.thispage, 
				(PAGE *)np);
    	    	    
    	    	    	if (lockup) 
			{
    	    	      	    er = m_release_page(trans_id, oldpid);
    	            	    if (er != OK) 
		             printf("inserthash, m_release_page had error %d\n",
				   er);
    	    	      	    er = m_release_page(trans_id, currentPid);
    	            	    if (er != OK) 
		             printf("inserthash, m_release_page had error %d\n",
				   er);
    	    	        }
			goto error;
    	    	    }

		    /* at this point locks are being held on pid, the root pid,
		      and the oldpid */

    	    	    i = slotnum; /* slot # on old page */
    	    	    slotnum = 0; /* slot # on new page */

    	    	    /* insert into new page */
    	    	    KEYRIDLEN(dp, i, k); 
    	    	    bt_insertentry(filenum, np, k, ENTADDR(dp, i), 0);

    	    	    /* remove from old page */
    	    	    j = dp->btcontrol.numoffsets;
    	    	    for (i++; i < j; i++) dp->slot[i-1] = dp->slot[i];
    	    	    MAKEALIGN(k);
    	    	    dp->btcontrol.numfree += k;
    	    	    dp->btcontrol.numoffsets--;
    	    	    (void) bf_setdirty(filenum, &oldpid, (PAGE *)dp);
    	    	    (void) bf_freebuf(filenum, &oldpid, (PAGE *)dp);

    	    	    if (lockup) 
		    {
    	    	        er = m_release_page(trans_id, oldpid);
    	            	if (er != OK) 
		         printf("inserthash, m_release_page had error %d\n", er);
    	    	    }
    	        } /* end of if(dup) */ 
    	        dp = np;
		currentPid = pid;  /* so that the correct page gets unlocked */
	    } 

	} /* end of overflow handling */

    } while( FREESPACE(dp) < length + sizeof(dp->slot[0]) );

    /* recalculate the on-page position */
    (void) bt_binary_search(key->length, key->value, dp, &slotnum);

insert:
    /* insert the entry into the current bucket */
    if (dup) 
	e = bt_addrid(filenum, dp, slotnum, h_rid, trans_id, lockup, cond);
    else 
    {	
    	e = bt_insertentry(filenum, dp, length, buf, slotnum);
    	F_CARD(filenum)++;	
    	F_STATUS(filenum) = DIRTY;
    }

    /* printf(" %d ", F_CARD(filenum)); */

    (void) bf_freebuf(filenum, &dp->btcontrol.thispage, (PAGE *)dp);

    if (lockup) 
    {
        if (currentPid.Ppage != dp->btcontrol.thispage.Ppage)
	  printf("bt_inserthash.  expected currentPid and dp to be same page\n");

        if (m_release_page(trans_id, currentPid) != OK) 
        printf("\nin h_inserthash, m_release_page(%d, (%d:%d) returns error\n", 
	     trans_id, (F_ROOTPID(filenum)).Ppage, (F_ROOTPID(filenum)).Pvolid);
    }

error:
    (void) bf_freebuf(filenum, &(F_ROOTPID(filenum)), (PAGE *)ht);
    if (lockup) 
    {
        if (m_release_page(trans_id, F_ROOTPID(filenum)) != OK) 
        printf("\nin h_inserthash, m_release_page(%d, (%d:%d) returns error\n", 
	trans_id, (F_ROOTPID(filenum)).Ppage, (F_ROOTPID(filenum)).Pvolid);
    }
    return(e);

} /* h_inserthash */
