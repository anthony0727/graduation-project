
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
/* Module : st_createindex 
  	The module creates a new index file (<FileName>.i<3-digit index #>)
  	for the file <FileName>. The index file is structured as a B-tree.
  	A sorted key/RID (or "keyrid") file is created from the data file
  	which is then used to construct the B-tree from bottom up.
  
  	The general flow of B-tree creation is as follows:
  	1.  The keys and rids of data records are extracted
  	from a data file and stored in a temporary keyrid file.
  	2.  Each key and  all of its rids are read from the keyrid
  	file and formatted into proper index entries for insertion
  	onto a leaf page.
  	3.  The index entries are then appended onto the leaf pages
  	of the B-tree.  When a leaf page is filled, another is allocated
  	and filled in the same manner.  Bt_insertentry is the utility
  	that actually enters the index entry onto the leaf page.
  	Bt_allocpage allocates a new page and initializes it appropriately.
  	4.  When a new leaf is allocated for more key/rid entries,
  	the bt_insertkey routine is called to insert the first
  	key of the new leaf into its parent node.  

   IMPORTS:
  	bf_setdirty(filenum, pageid, pageptr)
  	bf_freebuf(filenum, pageid, pageptr)

  	int st_openfile(VolID, filename, accessmode)
  	st_closefile(filenum, TRUE);
  	st_createfile (VolID, FileName, NumPages, ExtentFF, PageFF);
  	st_destroyfile(VolID, Filename); 
  	st_firstfile(filenum, FirstRID, trans_id, lockup, cond);
  	st_nextfile(filenum, CurrentRID, NextRID, trans_id, lockup, mode, cond);
  	st_appendrecord(filenum, Recadr, Len, NewRID, trans_id, lockup, cond);
  	st_createlong(filenum);
  	st_insertframe(filenum, RID, offset, RecAdr, length)
  	st_sort(Volid, filename, keyinfo, suffix);	from sort.c
  	r_getrecord(filenum, RID, page@, recptr@, trans_id, lockup, mode, cond);
  	bt_allocpage(trans_id, filenum, newpid, newpage, 
  			keytype, pagetype, treetype,unique);	from btutil.c
  	bt_insertentry(filenum, page, key_len, buf, slotnum); from btutil.c
  	bt_insertkek(filno, parentlist, parentindex, key_len, key, pid);
  	int compare_key(key1, key2, type, length);
  
   EXPORTS:
  	st_createindex(VolID, FileName, IndexNo, KeyAttr, 
  				FillFactor, Unique, SortFile, trans_id, lockup)
*/

#include <wiss.h>
#include <st.h>
#include        <lockquiz.h>

#define EXTENTFF 90 		/* extent fill factor */
#define	ERRORHANDLER(err)	if((e = err) < eNOERROR) goto error
  

static
extract_key(volid, filename, krfile, k_attr, trans_id, lockup, cond)
int    	volid;			/* volume id */
char    	*filename;		/* name of the data file */
char    	*krfile;		/* name of the key rid file */
KEYINFO    	*k_attr;		/* pointer to key attribute */
int             trans_id;
int           	lockup;
int           	cond;

/* This routine extracts the key of each record in the data file and
stores a record (consisting of the key and rid of the data record)
in the keyrid file. It assumes the key-rid file is initially empty.

RETURNS:
None

SIDE EFFECTS:
None

ERRORS:
None
*/
{
	int		e;		/* error code return		*/
	RID		rid;		/* rid of the current data record */
	RID		dummyrid;	/* rid for calling st_appenrecord */
	RECORD  	*recptr;	/* record address		*/
	int		filenum;	/* open file num of the data file */
	int		krofn;	/* open file num of the keyrid file */
	DATAPAGE	*pageptr;
	char	buff[MAXKEYLEN + sizeof(RID)];	/* keyrid rec buffer */

#ifdef    TRACE
	if (checkset(&Trace2, tEXTRACTKEY)) {
		printf("extract_key(volid=%d, filename=%s, ", volid, filename);
		printf("krfile=%s)\n", krfile);
	}
#endif

	/* open both the data file and the keyrid file */
	filenum = st_openfile(volid, filename, READ);
	CHECKERROR(filenum);

	/* lock the actual input file in S mode */
	if (lockup)
	{
		e = lock_file(trans_id, FC_FILEID(filenum), l_S, COMMIT, cond);
		if (e < eNOERROR) return(e);
	}

	krofn = st_openfile(volid, krfile, WRITE);
	CHECKERROR(krofn);

	/* lock the actual output file in X mode */
	/* lock krfile no matter what since stcreateidx just created the file */

	/* krfile is a temporary file so lock it in X mode with MANUAL mode */
	if (lockup)
	{
		e = lock_file(trans_id, FC_FILEID(krofn), l_X, MANUAL, cond);
		if (e < eNOERROR) return(e);
	}

	/* extract each data record and generate a keyrid record */
	for (e = st_firstfile(filenum, &rid, trans_id, FALSE, l_NL, cond);
		e >= eNOERROR;
		e = st_nextfile(filenum, &rid, &rid, trans_id, FALSE, l_NL, cond))
	{
		e = r_getrecord(filenum, &rid, &pageptr, &recptr, trans_id,
			FALSE, l_NL, cond);
		CHECKERROR(e);
		movebytes(buff, &(recptr->data[k_attr->offset]), k_attr->length);
		MOVERID(&(buff[k_attr->length]), (char *)(&rid));
		(void)bf_freebuf(filenum, &pageptr->thispage, (PAGE *)pageptr);
		e = st_appendrecord(krofn, buff, k_attr->length + sizeof(RID),
			&dummyrid, trans_id, FALSE, cond);
		CHECKERROR(e);
	}

#ifdef DEBUG
	if (checkset(&Trace2, tEXTRACTKEY))
		bt_print_keyridfile(krofn, k_attr, -1, FALSE);
#endif
	e = st_closefile(filenum, TRUE);
	CHECKERROR(e);
	e = st_closefile(krofn, TRUE);
	return(e);

} /* extractkey */


st_createindex(VolID, FileName, IndexNo, KeyAttr, FillFactor, Unique, 
	SortFile, trans_id, lockup, cond)
short	VolID;		/* volume id */
char	*FileName;	/* file on which index is to be created */
TWO	IndexNo;	/* IndexNo reflects how many indexes existed before 
			   this one.  It is also used to create 
			   unique filenames for the index */
KEYINFO	*KeyAttr; 	/* pointer to the attributes of the key */
short	FillFactor;	/* page fillfactor for leaves */
ONE	Unique;		/* if TRUE then the keys are unique */
short	SortFile;	/* if TRUE then the file is to be sorted also */
int     trans_id;
short   lockup;		/* if TRUE locking is performed */
short   cond;		/* if TRUE conditional locking is used */

/* (See module description.)
  	
   RETURNS:
  	eNOERROR if successful
  
   SIDE EFFECTS:
  	if SortFile==TRUE, the data file is returned sorted on the key
  
   ERRORS:
  	e2INDEXNUMTOOLARGE if IndexNo >= 1000
  	e2KEYLENGTHTOOLONG if KeyAttr->keylength >= MAXKEYLEN
*/
{
    char 		krfile[MAXFILENAMELEN];
        char    	indexfilename[MAXFILENAMELEN];
    int		e;		/* return codes from called modules */	
    int		krofn;	/* open file # of the kye-rid file */
    int		indexofn;	/* open file # of the index file */
    KEYINFO		keyridattr; 	/* attributes of keys for keyrid file */

#ifdef TRACE
    if (checkset (&Trace2, tINTERFACE))  {
    	printf("st_createindex(");
    	printf("VolID = %d, IndexNo = %d,", VolID, IndexNo); 
    	printf("FileName = %s,", FileName);
    	printf(" KeyAttr="); PRINTATTR(KeyAttr);
    	printf(", FillFactor = %d, Unique = %d, SortFile = %d)\n", 
    	        FillFactor, Unique, SortFile);
    }
#endif
    /* ensure a 3-digit index number. */
    if (IndexNo >= 1000) return(e2INDEXNUMTOOLARGE);
    /* ensure a buffer length which is <= MAXKEYLEN bytes. */
    if (KeyAttr->length >= MAXKEYLEN) return(e2KEYLENGTHTOOLONG);

    /* create files for sorted key/rid pairs and the index */
    (void)suffixname(krfile, FileName, KEYRIDSUF, IndexNo);

    /* (void) dummy_destroyfile(VolID, krfile, trans_id, FALSE); */
    (void) st_destroyfile(VolID, krfile, trans_id, FALSE, cond);

    e = st_createfile(VolID, krfile, 1, 100, 100);
    if (e < eNOERROR) 
	printf ("st_createfile point 1 failled with error %d.\n",e);
    ERRORHANDLER(e);
    (void)suffixname(indexfilename, FileName, INDEXSUF, IndexNo);

    e = st_createfile(VolID, indexfilename, 2, EXTENTFF, FillFactor);
    if (e < eNOERROR) 
	printf ("st_createfile point 2 failed with error %d.\n",e);
    ERRORHANDLER(e);

    /* Create a sorted keyrid file:
       If SortFile=TRUE, the data file is sorted first. Then, the keyrid 
       pairs are extracted in sequential order from the data file and 
       placed in the keyrid file. If SortFile=FALSE, the keyrid pairs 
       are first extracted from the data file and placed into the keyrid
       file. Then, the keyrid file is sorted. */
    
    if (SortFile == TRUE) 
    { 
        /* sort data file, then extract key-rid pairs */

        e = st_sort(VolID,FileName,KeyAttr,IndexNo, trans_id, lockup, cond);
        if (e < eNOERROR) 
    		printf ("st_sort point 1 failed with error %d.\n",e);
    	ERRORHANDLER(e);

        /* SHERROR, logically at this stage krfile is not being locked.
         * this is an error, but since there is only one krfile per
         * actual file I am safe for now but this may require more
         * attention later on.
         * by the end of the else stmt the krfile is locked in X mode.
         * by now the FileName is locked in X mode.
         */

        /* krfile is locked no matter what is inside extract_key */

        e = extract_key(VolID, FileName, krfile, KeyAttr, trans_id, 
    		FALSE, cond);
        if (e < eNOERROR) 
    	printf ("extract_key point 1 failed with error %d.\n",e);
        ERRORHANDLER(e);
    }
    else 
    { 
        /* extract the keyrid pairs then sort the key-rid file */
        e = extract_key(VolID, FileName, krfile, KeyAttr, 
    		trans_id, lockup, cond);
        if (e < eNOERROR) 
    		printf ("extract_key point 2 failled with error %d.\n",e);
        ERRORHANDLER(e);

        keyridattr = *KeyAttr;	/* key attr for sorting keyrid file */
        keyridattr.offset = 0;

        e = st_sort(VolID,krfile,&keyridattr,IndexNo, trans_id, FALSE, cond);

        if (e < eNOERROR) 
           printf ("st_sort point 2 failed with error %d.\n",e);
        ERRORHANDLER(e);
    }

    /* open the key-rid file and the index file, then fill the leaves */
    /* both files are by now acquired in exclusive mode */

    krofn = st_openfile(VolID, krfile, READ);
    if (krofn < eNOERROR) 
    	printf ("st_openfile point 1 failed with error %d.\n",krofn);
    ERRORHANDLER(krofn);

    indexofn=st_openfile(VolID, indexfilename, WRITE);
    if (indexofn < eNOERROR) 
    	printf ("st_openfile point 2 failed with error %d.\n",indexofn);
    ERRORHANDLER(indexofn);

    /* lock the indexfile in eXclusive mode no matter what */
    if (lockup)
    {
        e = lock_file (trans_id, FC_FILEID(indexofn), l_X, COMMIT, cond);
        if (e < eNOERROR) return (e);
    }

    e = fill_leaf(krofn, indexofn, KeyAttr, FillFactor, Unique, trans_id,
	lockup, cond);
    if (e < eNOERROR) 
           printf ("fill_leaf point 1 failed with error %d.\n",e);
    ERRORHANDLER(e);
    F_FILETYPE(indexofn) = INDEXFILE, F_STATUS(indexofn) = DIRTY;
 
    /* close the files and destory the key rid file */
    e = st_closefile(krofn, TRUE);
    ERRORHANDLER(e);
    e = st_closefile(indexofn, TRUE);
    ERRORHANDLER(e);

    e = st_destroyfile(VolID, krfile, trans_id, FALSE, cond);
    ERRORHANDLER(e);

    return(eNOERROR);

/* error detected, clean up the mess and terminate */
error:
    (void)st_closefile(krofn, TRUE);
    (void)st_destroyfile(VolID,krfile, trans_id, FALSE, cond);
    (void)st_closefile(indexofn, TRUE);
    (void)st_destroyfile(VolID,indexfilename, trans_id, FALSE, cond);
    return(e);

} /* end stcreateidx.c */


fill_leaf(krofn, indexofn, k_attr, fillfactor, unique, trans_id, lockup, cond)
int	krofn;			/* open file number of keyrid file */
int	indexofn;		/* open file number of index file */
KEYINFO	*k_attr;		/* key information structure	*/
int	fillfactor;		/* percent of page to fill	*/
ONE	unique;			/* primary file ? */
int	trans_id;
short	lockup;
short	cond;

/* This routine reads each key-rid pair from the sorted keyrid file and 
      enter each key and its corresponding rids on leaf pages. When one leaf 
      page becomes full, this routine allocates a new leaf page and proceeds 
      in the same manner. In addition, overflow detection begins within this 
      routine when a key and all of its rids do not fit into a page. 
      An overflow entry (who has more than MAXRIDCOUNT RIDs on its list)
      is stored in a long data item. The RID list part of an overflow entry
      (on the leaf) is the negated RID count followed by the directory RID
      of the long data item.
  
   RETURNS:
    None
  
   SIDE EFFECTS:
      New leaf pages are created along the splitting of higher level
      node and root pages when necessary.
  
   ERRORS:
      None
*/
{
    int		e;		/* standard error return variable */
    RID		currid;		/* current RID of keyrid record */
    TWO		length; 	/* length of key & rid list */
    PID		pid;		/* PID of current page */
    TWO		ridcount;	/* number of rids for one key */
    int		result;		/* result of comparison */
    short	overflow;	/* current RID list overflow ? */
    RID		dirrid;		/* RID of a long item */
    TWO		prev_count;	/* for handling overflow entry */
    DATAPAGE	*dp;
    RECORD	*recptr;	/* pointer to record on data page */
    RID		trid;
    BTREEPAGE	*page;		/* pointer to current page */
    BTREEPAGE	*prev_page;	/* pointer to previous leafpage */
    PARENTLIST	pl;		/* path from the root to current leaf */
    PARENTINDEX	pi;	
    char	entrybuf[ENDDATA]; /*buffer to collect key and rids */

#ifdef    TRACE
    if (checkset(&Trace2, tFILLLEAF)) {
    	printf("fill_leaf(krofn=%d, indexofn=%d, fillfactor=%d,", 
    		krofn, indexofn, fillfactor);
    	printf("unique = %s)\n", unique?"YES" : "NO");
    }
#endif

    /* create an empty B-tree with a root and a node */
    e = bt_allocpage(trans_id, indexofn, &pl[0], &prev_page, (ONE)k_attr->type, 
	ROOTPG, INDEX, unique);
    CHECKERROR(e);
    ((INDEXHEADER *)prev_page)->keyattr= *k_attr;
    e = bt_allocpage(trans_id, indexofn, &pl[(pi=1)], &page, (ONE)k_attr->type, 
	LEAFPG, INDEX, unique);
    prev_page->btcontrol.pid0 = pl[1].page_id.Ppage;
    (void) bf_freebuf(indexofn, &pl[0], (PAGE *)prev_page);
    CHECKERROR(e);

    /* record the root page in file descriptor */
    F_ROOTPID(indexofn) = pl[0].page_id;
    F_NUMPAGES(indexofn) = 2;
    F_STATUS(indexofn) = DIRTY;

    /* convert the fill factor from % into # of bytes */
    if (fillfactor == 0) fillfactor = 100;
    fillfactor = ( (100 - fillfactor) * page->btcontrol.numfree) / 100;

    /* construct the leaf pages from the key/rid file */
    for (e = st_firstfile(krofn, &currid, trans_id, FALSE, l_NL, cond ); 
	  !TESTRIDCLEAR(currid);) 
    {
    	/* construct the key and its first RID in the buffer */
    	e = r_getrecord(krofn, &currid, &dp, &recptr, 
		trans_id, FALSE, l_NL, cond);
    	if (e < eNOERROR) goto error;
    	MAKEKEYRID(entrybuf, k_attr->length, recptr->data, 
    			&recptr->data[k_attr->length], length); 
	bcopy((char *)&(((RID *)(&recptr->data[k_attr->length]))->Rvolid),
	   (char *)&trid.Rvolid, 
	   sizeof(trid.Rvolid));
    	(void) bf_freebuf(krofn, &dp->thispage, (PAGE *) dp);
    	overflow = FALSE;
    	/* collect RIDs of the same key */
    	for (ridcount = 1, e = st_nextfile(krofn, &currid, &currid, 
		      trans_id, FALSE, l_NL, cond);
    	  !TESTRIDCLEAR(currid); 
	  e=st_nextfile(krofn,&currid,&currid, trans_id, FALSE, l_NL, cond)) 
	{
    		/* get address of the next record of keyrid file */
    		e = r_getrecord(krofn, &currid, &dp, &recptr,
			trans_id, FALSE, l_NL, cond);
    		if (e < eNOERROR) goto error;
		bcopy(
		 (char *)&(((RID *)(&recptr->data[k_attr->length]))->Rpage),
		 (char *)&trid.Rpage,
		 sizeof(trid.Rpage));
		bcopy(
		 (char *)&(((RID *)(&recptr->data[k_attr->length]))->Rslot),
		 (char *)&trid.Rslot,
		 sizeof(trid.Rslot));
    		result = compare_key(&entrybuf[2], recptr->data,
    			k_attr->type, k_attr->length);
    		(void) bf_freebuf(krofn, &dp->thispage, (PAGE *)dp);
    		if (result) break;	/* the beginning of a new key */

    		if (unique) {
    			e = e2DUPLICATEKEY;
    			goto error;
    		}

    		/* if adding the new rid to entrybuf will cause overflow
    	   	   then create an overflow entry, otherwise append
    		   the rid to the end of entrybuf 
    		*/

      		if ((length + sizeof(RID)) >= ENDDATA) {
    			length = k_attr->length + 4;	
    			if (!overflow) { 
    			    overflow = TRUE;
    			    e = st_createlong(indexofn, &dirrid, trans_id,
				lockup, cond);
    			    if (e < eNOERROR) goto error;
    			    prev_count = 0;
    			}
    			e = st_insertframe(indexofn, &dirrid,
    			     prev_count*sizeof(RID), &entrybuf[length],
    			     (ridcount - prev_count) * sizeof(RID),
			     trans_id, lockup, cond);
    			if (e < eNOERROR) goto error;
    			prev_count = ridcount;
    		}

    		MOVERID(&entrybuf[length],&trid); 
    		length += sizeof(RID);
    		ridcount++;

    	}	/* end of inner for */

    	if (e != e2ENDOFFILE && e < eNOERROR) goto error;

    	if (!overflow && ridcount >= MAXRIDCOUNT) { 
    		e = st_createlong(indexofn, &dirrid, trans_id, lockup, cond);
    		if (e < eNOERROR) goto error;
    		overflow = TRUE;
    		prev_count = 0;
    	}

    	/* for an overflow RID list, put the rest of the RIDs in buffer
    	   into the long item, and leave the negated RID count + 
    	   directory RID of the long item in place of an ordinary
    	   RID list */
    	if (overflow) {
    		length = k_attr->length + 4;
    		e = st_insertframe(indexofn, &dirrid, 
    			prev_count*sizeof(RID), &entrybuf[length], 
    			(ridcount - prev_count) * sizeof(RID), 
			trans_id, lockup, cond);
    		if (e < eNOERROR) goto error;
    		MOVERID(&entrybuf[length], (char *)&dirrid);
    		length += sizeof(RID);
    		ridcount = -ridcount;
    	}

    	/* write the rid count into the buffer */
    	MOVERIDCOUNT(&entrybuf[k_attr->length+2], (char *)&ridcount);

    	if (page->btcontrol.numfree <= fillfactor + length) {
      		/* this page is too full, allocate another leaf page */
    		prev_page = page;
    		e = bt_allocpage(trans_id, indexofn, &pid, &page, 
    			prev_page->btcontrol.keytype, 
    			prev_page->btcontrol.pagetype,
    			INDEX, prev_page->btcontrol.unique);
    		if (e < eNOERROR) goto error;
    		F_NUMPAGES(indexofn)++, F_STATUS(indexofn) = DIRTY;

    		/* adjust the links in the adjacent 2 or 3 pages */
    		page->btcontrol.prev = pl[pi].page_id.Ppage;
    		prev_page->btcontrol.next = pid.Ppage;
    		(void) bf_freebuf(indexofn, &pl[pi].page_id, (PAGE *)prev_page);
    		pl[pi].page_id = pid;

    		/* insert key of the new leaf into the parent node */
    		e = bt_insertkey(indexofn, pl, &pi, k_attr->length,
    			&entrybuf[sizeof(k_attr->length)], &pid, trans_id, 
			FALSE, cond);
    		if (e < eNOERROR) goto error;
            }

    	/* append key and rids to the leafpage */
    	(void) bt_insertentry(indexofn, page, length,
    		entrybuf, page->btcontrol.numoffsets);

    	F_CARD(indexofn)++, F_STATUS(indexofn) = DIRTY;

    } /* end of the outer loop */
    e = eNOERROR;
/*
    bt_print_levels(indexofn, &pl[0], -1, FALSE);
*/


#ifdef    DEBUG		/* print the whole file just created */
    if (checkset(&Trace2, tFILLLEAF) && checkset(&Trace2, tTREEDUMP)) {
    	printf("Newly created index file:\n");
    	bt_print_btfile(indexofn, &pl[0], -1, FALSE);
    }
#endif

error:
    (void) bf_freebuf(indexofn, &pl[pi], (PAGE *)page);

    return(e);

} /* fill_leaf */

