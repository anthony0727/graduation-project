
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
/* Module st_sort : sort utilitites - disk-based external merge sort.

     The sort routine can be divided into two phases, sorting and merging phase.
     The sorting phase go through a loop in which pages are read in as much 
     as possible at a time to produce a sorted partition.
     A combined method of quick sort and straight insertion sort
     is used to sort the records in the page buffers.
     After each sorted partition is produced, records are gathered into a 
     output page buffer, which are written out to disk as soon as it is full.  
     After the sorting phase, partitions are merged to produce larger 
     partitions in successive passes until there is only one partition left.

   IMPORTS:
	io_readpage(&pid, returnpage);
	io_writepage(&pid, pageptr, flag, &status)
	io_allocpages(&fid, &nearpid, num_pages, &new_pids);
	io_createfile(VolID, num_pages, eff, fidptr);
	bf_invalidate(numpages, pids)
	ST_accessfiledesc(VolID, filename, &fd, READ); 
	st_createfile(VolID, into, num_pages, eff, pff);
	st_destroyfile(VolID, filename, transid, lockup, cond);
	st_rename(VolID, newfilename, oldfilename);
	int st_openfile(VolID, filename, READorWRITE);
	st_closefile(filenum, TRUE);
	int st_filepages(VolID, filename);

   EXPORTS:
	st_sort()
	st_sortinto()
*/

#include	<wiss.h>
#include	<st.h>

extern char *malloc();
#include        <lockquiz.h>

/* these stuff are for tracing purpose only */
#ifdef	DEBUGSORT
long	start;
int	compare_count;
#endif

#define	error_check(e)	if(e<0){printf(" error %d in st_sort\n",e);return(e);}
#define	check_memory(p) if(p==NULL)return(e2NOMOREMEMORY)

/* this is the # of input buffer, in addition there is one output page */
#define	BUFSIZE		500

int	buf_size = BUFSIZE;	/* # of input page buffers */
int	num_pages;	/* # of pages in the source file */
short	num_inpart;	/* # of input partitions (sub-files) */
short	num_outpart;	/* # of output partitions (sub-files) */
short	num_buffers;	/* # of available input buffers */
short	num_runs;	/* # of pages in the input buffers */
short	num_passes;	/* # of passes for merging phase */
short	cur_part;	/* index of the current output partition */
short	last_pass;	/* boolean flag - true when in the final pass */
KEYINFO	*keyattr;	/* the key attribute of the source file */

PID		*in_pids, *out_pids, *t_pids;	/* pool of disk pages */
DATAPAGE	*inpage, *outpage;		/* page buffers */
TWO		free_ptr;	/* start of free area in the output buffer */
TWO		numfree;	/* # of free bytes left in the output buffer */
TWO		ridcnt;		/* # of records in the output buffer */
char		**records;	/* record pointer array */

short		*heap;		/* heap for the merging phase */
short		heapsize;	/* conceptual size of the heap */
static	long	randx = 1;	/* for generating random #'s */


/* 
   The following data structure describes the sorted sub-files on disk. 
   The fields, front and rear, are indices into an arrays of PIDs, 
   which contain the page IDs from the temporary file.
*/

typedef struct
{
	int	front;
	int	rear;
} PART;
PART 	*in_part, *out_part, *t_part;


/* Flush out the current output page in buffer and reinitialized it */
static int flush_outpage()
{
	int	e, j;

	j = out_part[cur_part].rear++;
	/* set up page ID and link to neighbors */
	if (last_pass) { /* set up the links in data pages */
		outpage->thispage = out_pids[j];
		outpage->prevpage = (j == 0) ? NULLPAGE : out_pids[j - 1].Ppage;
		outpage->nextpage = out_pids[j + 1].Ppage;
	}

	outpage->free = free_ptr;
	outpage->ridcnt = ridcnt;
	free_ptr = ridcnt = 0, numfree = (int) sizeof(PAGE) - DPFIXED;

	return(io_writepage(&out_pids[j], outpage, SYNCH, &e));
}


static heap_top(winner)
register	winner;	/* the currnet champ */
{
	register	s;	/* a heap index */
	register	r;	/* a run # */
	register char	**rec;	/* local copy of records */

	rec = records;

	for (s = (heapsize + winner) >> 1; s > 0; s >>= 1)
	{
		if ((r = heap[s]) < 0) continue;
		if (rec[winner] == NULL) heap[s] = -1, winner = r;
		else if (key_compare(rec[winner], rec[r]) > 0)
			heap[s] = winner, winner = r;
	}
	return(winner);

} /* heap_top */


set_sortbuf(size)
/* set the buffer size for sorting */
int	size;
{
	if (size > 1) buf_size = size - 1;
	else printf(" unrealistic buffer size %d for sorting!\n", size);
}

reset_sortbuf()
/* reset the buffer size back to system's default */
{
	buf_size = BUFSIZE;
}

st_sort (VolID, filename, keyinfoptr, suffix, trans_id, lockup, cond)
TWO      	VolID;		/* volume ID */
char     	*filename;	/* filename to be sorted */
KEYINFO  	*keyinfoptr;	/* information about key attribute */
TWO      	suffix;		/* suffix of index */
int             trans_id;
short           lockup;
short           cond;
{

    int		e;
    char	tfilename[100];	/* for name of the temporary file */
    unsigned 	p;
    FILEDESC 	fd;		/* file descriptor */

#ifdef    TRACE
    if (checkset(&Trace2, tINTERFACE)) {
    	printf("st_sort(VolID=%d, filename=%s, transId=%d, lockup=%d)\n", 
	VolID, filename, trans_id, lockup);
    }
#endif

    SetLatch(&smPtr->level2Latch, procNum, NULL);
    e = ST_accessfiledesc(VolID, filename, &fd, READ); 
    CHECKERROR_RLATCH(e, &smPtr->level2Latch, procNum);

    /* check ownership and access permission */
    p = (fd.owner == sys_getuser()) ? OWNERWRITE : OTHERWRITE; 
    if (!(fd.flags & p)) 
    {
	ReleaseLatch(&smPtr->level2Latch, procNum);
        return(e2NOPERMISSION);
    }
    ReleaseLatch(&smPtr->level2Latch, procNum);

    /* create a temporary file name */
    (void) suffixname(tfilename, filename, TMPSUF, suffix);

    /* actually do the sort - no latches set during the sort  */
    e = st_sortinto(VolID, filename, tfilename, keyinfoptr, suffix, 
	trans_id, lockup, cond);
    error_check(e);

    /* discard the old file, and rename the temporary file */
    e = st_destroyfile(VolID, filename, trans_id, FALSE, cond);
    CHECKERROR_RLATCH(e, &smPtr->level2Latch, procNum);
    e = st_rename(VolID, filename, tfilename, trans_id, FALSE, FALSE);
    CHECKERROR_RLATCH(e, &smPtr->level2Latch, procNum);
    return(eNOERROR);
}

st_sortinto (VolID, filename, into, keyinfoptr, suffix, trans_id, lockup, cond)
TWO     VolID;    	/* volume ID */
char       *filename;	/* filename to be sorted */
char     *into;    	/* name of the sorted file */
KEYINFO  *keyinfoptr;    /* information about key attribute */
TWO      suffix;    /* suffix of index */
int      trans_id;
short    lockup;
short    cond;
{
    int	e;			/* for returned error codes */
    int	i, j, b, l;		/* loop indices */
    register k, s, r;		/* indices */
    int	sfilenum, rfilenum;	/* #'s of the source & dest. files */
    int	keyoffset;		/* offset of the key field */
    int	total_records;		/* total # of records */
    PID	inpid;			/* PIDs of current input/output pages */
    int	status;
    FID	fid, fid1;		/* file ID of the temporary file */
    register RECORD	*rptr;		/* record pointer */
    unsigned p;
    FILEDESC fd;		/* file descriptor */

#ifdef    TRACE
    if (checkset(&Trace2, tINTERFACE))
    {
    	printf("st_sortinto(volid=%d, source file=%s, dest file=%s)\n",
    		 VolID, filename, into);
        printf("\t trans_id %d, lockup %d.\n",trans_id, lockup);
    }
#endif
    /* INITIALIZATION */

    keyattr = keyinfoptr;		/* make it global to this file */
    keyoffset = keyinfoptr->offset;

    SetLatch(&smPtr->level2Latch, procNum, NULL);
    e = ST_accessfiledesc(VolID, filename, &fd, READ); 
    CHECKERROR_RLATCH(e, &smPtr->level2Latch, procNum);

    /* check ownership and access permission */
    p = (fd.owner == sys_getuser()) ? 
    	OWNERWRITE | OWNERREAD : OTHERWRITE | OTHERREAD;
    if (!(fd.flags & p)) 
    {
        ReleaseLatch(&smPtr->level2Latch, procNum);
	return(e2NOPERMISSION);
    }

    e = ST_accessfiledesc(VolID, into, &fd, READ); 
    if (e >= eNOERROR) 
    { 
	/* the file already exists */
    	/* check ownership and access permission */
    	p = (fd.owner == sys_getuser()) ? OWNERWRITE : OTHERWRITE;
    	if (!(fd.flags & p)) 
	{
        	ReleaseLatch(&smPtr->level2Latch, procNum);
               	return(e2NOPERMISSION);
	}
        ReleaseLatch(&smPtr->level2Latch, procNum);
    	(void) st_destroyfile(VolID, into, trans_id, lockup, cond);
    }
    else ReleaseLatch(&smPtr->level2Latch, procNum);

    num_pages = st_filepages(VolID, filename);
    error_check(num_pages);
    e = st_createfile(VolID, into, num_pages, 100, 100);
    error_check(e);
    if (num_pages == 0) 
    	return(eNOERROR); 	/* the source file is empty */
             
    sfilenum = st_openfile(VolID, filename, READ);
    error_check(e);
    if (lockup)
    {
/*
        printf ("Sort 1. trans_id %d filename %s.\n",trans_id, filename);
*/
	e = lock_file(trans_id, FC_FILEID(sfilenum), l_X, COMMIT, cond);
	if (e < eNOERROR) 
	{
    	   (void) st_destroyfile(VolID, into, trans_id, lockup, cond);
	    return(e);
	}
    }

    rfilenum = st_openfile(VolID, into, WRITE);
    error_check(rfilenum);
    fid = FC_FILEID(rfilenum);

    /* lock the into file in eXclusive mode */
    /* lock the file anyway since this procedure created it */
    /*
        printf ("Sort 2. trans_id %d filename %s.\n",trans_id, into);
    */
    if (lockup)
    {
	e = lock_file(trans_id, fid, l_X, COMMIT, cond);
	if (e < eNOERROR) 
	{
    	   (void) st_destroyfile(VolID, into, trans_id, lockup, cond);
	    return(e);
	}
    }

    /* calculate the #'s of buffers and passes needed for merging */
    num_buffers = (num_pages < buf_size) ? num_pages : buf_size;
    for (num_passes = 0, j = num_buffers; 
    	j < num_pages; j = j * num_buffers, num_passes++);
    num_outpart = (num_pages - 1) / num_buffers + 1;
    last_pass = (num_passes == 0);	/* only sorting phase ? */

#ifdef    DEBUGSORT
    printf("# of buffers required : %d\n", num_buffers);
    printf("# of passes : %d\n", num_passes);
    printf("# of pages : %d\n", num_pages);
#endif

    /* allocate the required memory and disk space */
    out_pids = (PID *) malloc ((unsigned) ((num_pages + 1) * sizeof(PID)));
    check_memory(out_pids);
    PIDCLEAR(out_pids[num_pages]);
    out_part = (PART *) malloc ((unsigned) ((num_outpart+1)*sizeof(PART)) );
    check_memory(out_part);

    inpage = (DATAPAGE *)malloc ((unsigned)(num_buffers*sizeof(DATAPAGE)));
    check_memory(inpage);
    outpage = (DATAPAGE *) malloc(sizeof(DATAPAGE));
    check_memory(outpage);
    outpage->fileid = fid;
    free_ptr = ridcnt = 0, numfree = (int) sizeof(PAGE) - DPFIXED;
    e = io_allocpages(&fid, (PID *) NULL, num_pages, out_pids);
    error_check(e);
    (void) bf_invalidate(num_pages, out_pids);

    /* SORTING PHASE */

#ifdef    TRACESORT
    start = time(0); compare_count = 0;
#endif

    /* split the source file into sorted partitions */

    inpid = F_FIRSTPID(sfilenum);

    total_records = 0;

    /* in the following, b is the # of processed input pages */
    for (i = 0, b = 0; i < num_outpart; i++, b += num_buffers) {
    	/* read in as many pages as possible */
    	num_runs = (i == num_outpart - 1) ? num_pages - b : num_buffers;
    	for (j = 0, r = 0 /* record count */; j < num_runs; j++) {
    		e = io_readpage(&inpid, (PAGE *) &inpage[j]);
    		error_check(e);
    		r += inpage[j].ridcnt;
    		inpid.Ppage = inpage[j].nextpage;
    	}
    	cur_part = i;
    	out_part[i].front = out_part[i].rear = b;

    	/* allocate memory and set up the record pointer array */
    	records = (char **) malloc ( (unsigned) (r * sizeof (char *)) );
    	check_memory(records);
    	for (j = 0, r = 0 /* record count */; j < num_runs; j++)
    		for (k = 0, l = inpage[j].ridcnt; k < l; k++) {
    			if ((s = inpage[j].slot[-k]) == EMPTYSLOT) 
    				continue;  /* skip deleted tuples */
    			rptr = (RECORD *) &(inpage[j].data[s]);
    			if (rptr->type == MOVED) continue; /* skip */
    			rptr->type = NOTMOVED;
    			records[r++] = rptr->data + keyoffset;
    		}
    	total_records += r;

    	q_sort(records, r); /* sort the records in buffers */

    	/* write the sorted partition out to disk */ 
    	for (k = 0, e = eNOERROR; k < r && e == eNOERROR; k++)
    	   e = add_record((RECORD *) (records[k]-HEADERLEN-keyoffset));
    	error_check(e);
    	e = flush_outpage();
    	error_check(e);
    	free((char *) records);
    }

    if (num_passes == 0) /* the file already sorted */
    	goto sorted;

#ifdef    TRACESORT
    printf(" # of comparisons in sorting phase %d\n", compare_count);
    printf(" sorting (%d records) : %ds\n", total_records, time(0) - start);
#endif

    /* MERGING PHASE */
#ifdef    TRACESORT
    start = time(0); compare_count = 0;
#endif
    in_pids = (PID *) malloc((unsigned)((num_pages+1)*sizeof(PID)));
    check_memory(in_pids);
    PIDCLEAR(in_pids[num_pages]);
    in_part = (PART *) malloc((unsigned) ((num_outpart+1)*sizeof (PART)) );
    check_memory(in_part);

    /* create a second file as buffer */
    e = io_createfile(VolID, num_pages, 100, &fid1);
    error_check(e);
    e = io_allocpages(&fid1, (PID *)NULL, num_pages, in_pids);
    error_check(e);
    (void) bf_invalidate(num_pages, in_pids);
    if (num_passes & 1) /* the 2nd file will have the result */
    	F_FILEID(rfilenum) = outpage->fileid = fid1;

    /* merge all the partitions until the entire file is sorted */
    records = (char **) malloc ((unsigned) (num_buffers * sizeof (char *)));
    check_memory(records);
    for (k = num_buffers - 1, heapsize = 1; k > 0; heapsize <<= 1, k >>= 1);
    heap = (short *) malloc((unsigned) (heapsize * sizeof(short)) );
    check_memory(heap);

    for (i = 0; i < num_passes; i++) {
    	/* switch the input/output buffers */
    	t_pids = in_pids; in_pids = out_pids; out_pids = t_pids;
    	t_part = in_part; in_part = out_part; out_part = t_part;
    	last_pass = (i == num_passes - 1);

    	/* recalculate #'s of input and output partitions */
    	num_inpart = num_outpart;
    	num_outpart = (num_inpart - 1) / num_buffers + 1;

    	for (j = 0, b = 0; j < num_outpart; j++, b += num_buffers) {
    	  /* b is the # of input runs merged so far */
    		num_runs = (j==num_outpart-1)?num_inpart-b:num_buffers;

    		cur_part = j;
    		out_part[j].front = out_part[j].rear = 
    			(j == 0) ? 0 : out_part[j-1].rear;

    		/* read in the first page of each input partition */
    		for (k = 0; k < num_runs; k++) {
    			inpid = in_pids[in_part[b + k].front++];
    			e = io_readpage(&inpid, (PAGE *)&(inpage[k]));
    			error_check(e);
    				
    			/* add 1st record on page to pointer array */
    			records[k] = inpage[k].data+HEADERLEN+keyoffset;
    			inpage[k].free = 1; 
    			/* 'free' is borrowed as a record couter that
    			   keeps track of how many has been examined */
    		}

    		if (num_runs == 1) /* only one run in this partition */
    			heap[0] = 0;
    		else { /* build up a heap as a selection tree for merging */
    			for (k = num_runs - 1, heapsize = 1; k > 0; 
    					heapsize <<= 1, k >>= 1); 
    			create_heap(num_runs);
    		}

    		/* keep merging until there is only one run left */
    		for (r = heap[0]; num_runs > 1; r = heap_top(r)) {
    			e = add_record((RECORD *) 
    				(records[r]-HEADERLEN-keyoffset));
    			error_check(e);

    			/* fetch the next record on input page s */
    			if (inpage[r].free == inpage[r].ridcnt) {
    			  /* use up all the records on current page */
    			      if(in_part[b+r].rear==in_part[b+r].front) {
    				/* no more pages in this run */
    					records[r] = NULL;
    					num_runs--;
    					continue;
    				}
    				/* get next page in this partition */
    				inpid = in_pids[in_part[r+b].front++];
    				e = io_readpage(&inpid, (PAGE *)&inpage[r]);
    				error_check(e);
    				inpage[r].free = 0;
    			}
    			s = inpage[r].slot[-(inpage[r].free++)];
    			records[r] = &(inpage[r].data[s]) + 
    				HEADERLEN + keyoffset;
    		}

    		/* add all the remaining record to outbuf buffer */
    		for (k = inpage[r].free-1, l = inpage[r].ridcnt; ;) {
    		 /* r:run #, k:current slot #, s:slot, l:limit */
    			s = inpage[r].slot[-(k++)];
    			e = add_record((RECORD *) &(inpage[r].data[s]));
    			error_check(e);
    			if (k < l) continue; /* more slot on page */
    			if (in_part[b+r].rear == in_part[b+r].front)
    				break;	/* no more pages in this run */

    			/* get the next page in this run */
    			inpid = in_pids[in_part[r+b].front++];
    			e = io_readpage(&inpid, (PAGE *)&inpage[r]);
    			error_check(e);
    			k = 0, l = inpage[r].ridcnt;
    		}
    		e = flush_outpage();
    		error_check(e);

    	} /* end for each partition */

    } /* end for each pass */

    /* release temporary memory buffers and one of the disk files */
    free((char *) in_part); 
    free((char *) in_pids);
    free((char *) records);
    free((char *) heap);
    io_destroyfile( (num_passes & 1) ? &fid : &fid1);

    /* FINAL CLEAN UP */
sorted:

    F_NUMPAGES(rfilenum) = num_pages;
    F_CARD(rfilenum) = total_records;
    F_FIRSTPID(rfilenum) = out_pids[0];
    F_LASTPID(rfilenum) = outpage->thispage;
    F_STATUS(rfilenum) = DIRTY;

    e = st_closefile(sfilenum, TRUE);
    error_check(e);
    e = st_closefile(rfilenum, TRUE);
    error_check(e);

#ifdef    TRACESORT
    printf(" # of comparisons in merging phase %d\n", compare_count);
    printf(" merging (%d records) : %ds\n", total_records, time(0) - start);
#endif

    /* release all the temporary memory buffers */
    free((char *) out_part);
    free((char *) out_pids);
    free((char *) inpage);
    free((char *) outpage);
    return(eNOERROR);

}

/* This routine constructs a heap to serve as a selection tree for 
   speeding the process of picking up the records with the smallest key
       heap[r] = -1 represents a very large key,
       heap[r] = heapsize means the node has not been touched yet.
*/

create_heap(num_runs)
short    num_runs;
{
    register	r;	/* a run # */
    register	s, k;	/* a heap index */
    register	winner;	/* the winner run */
    int		tmp;

#ifdef    DEBUGSORT
    printf("create_heap(heapsize=%d, # of runs=%d)\n", heapsize, num_runs);
#endif

    for (s = 0; s < heapsize; heap[s++] = heapsize);
    for (r = 0, k = heapsize >> 1; r < heapsize; r += 2, k++) {
      /* compare pairs of headers in runs */
    	if (r < num_runs - 1) { /* two real competitors */
    		if (key_compare(records[r], records[r+1]) > 0)
    			heap[k] = r, winner = r+1;
    		else	heap[k] = r+1, winner = r;
    	}
    	else {
    		heap[k] = -1; /* an artifical key */
    		winner = (r>=num_runs)?-1/* 2 phonies */:r/* seed */;
    	}

    	for (s = k >> 1;; s >>= 1) { /* propagate the winner upwards */
    		if (heap[s] == heapsize) { /* no one has reach here yet */
    			heap[s] = winner; break;
    		}
    		if (winner < 0) /* a dummy key */
    			winner = heap[s], heap[s] = -1;
    		else if (key_compare(records[winner], records[heap[s]]) > 0) {
    			/* a new winner */ 
    			tmp = winner; winner = heap[s]; heap[s] = tmp;
    		}
    	}
    } 

} /* end of create_heap */

    		
/* Append a record to the output buffer ;
   if it is full, flush it out and prepare a new one */
add_record(r)
RECORD    	*r;
{
    int		e;
    int		l;

    l = r->length + HEADERLEN;	/* total length of the record */
    if ((e = l & (ALIGN - 1))) l += (ALIGN - e);	/* make it aligned */

    if (numfree < l) { /* page full, flush it out, and set up a new one */
    	e = flush_outpage();
    	CHECKERROR(e);
    }

    outpage->slot[-(ridcnt++)] = free_ptr;
    movebytes(&(outpage->data[free_ptr]), (char *) r, l);
    free_ptr += l, numfree -= l + (int) sizeof(outpage->slot[0]);

    return(eNOERROR);
}


#define    MAXSTACKDEPTH	30
#define    LIMIT		10

/* 
   This routine implements an internal sorting algorithm which
   combines both quick sort and straight insertion sort algorithms.
   Quick sort is used initially to partition records into sorted fragements. 
   When the size of a segment falls below certain limit, straight insertion 
   sort is used to avoid the overhead of quick sort
*/

q_sort(a, num_records)
register char    *a[];			/* pointers to records */
int    	num_records;		/* number of records */
{
    int		stack[MAXSTACKDEPTH][2];	/* partition stack */
    int		sp = 0;		/* stack pointer */
    int		l, r;		/* range of the current partition */
    char		*tmp;		/* for swaping pointers */
    register int	i, j;		/* indices */
    register char	*pivot;		/* pointer to the pivot record */

    for(l = 0, r = num_records - 1; ; ) {
     /* l and r are the left and right bounds of the current partition */
    	if (r - l < LIMIT) {
    		/* for small partitions, use straight insertion sort 
    		(later, not here) */
    		if (sp-- <= 0) break;	 /* stack empty, done ! */
    		l = stack[sp][0], r = stack[sp][1];
    		continue;
    	}
    	
    	/* randomly select a pivot in the current partition */
    	randx = (randx * 1103515245 + 12345) & 0x7fffffff;
    	pivot = a[l + randx % (r - l)];
    	for (i = l, j = r; i <= j; ) {
    	 /* look for pairs of displaced records, and swap them */
    		while (key_compare(a[i], pivot) < 0) i++;
    		while (key_compare(pivot, a[j]) < 0) j--;
    		if (i < j) { tmp=a[i]; a[i]=a[j]; a[j]=tmp; }
    		if (i <= j) i++, j--;
    	}

    	/* push the larger partition on stack and work on the other */
    	if (j - l < r - i) { /* push the right, and work on the left */
    		if (i < r) stack[sp][0] = i, stack[sp++][1] = r;
    		r = j; 
    	}
    	else { /* push the left, and work on the right */
    		if (l < j) stack[sp][0] = l, stack[sp++][1] = j;
    		l = i; 
    	}
    } /* end of the main loop */

    /* final pass to sort the whole file */
    for (i = 1; i < num_records; a[j + 1] = pivot, i++)
    	for (j = i - 1, pivot = a[i]; j >= 0 && 
    	     key_compare(pivot, a[j]) < 0; a[j + 1] = a[j], j--);

} /* end of q_sort */

/* 
    This routine compares the key fields of two records and returns
    0 if they are equal, 1 if k1 > k2, -1 otherwise.
*/

key_compare(k1, k2)
    register char	*k1;	/* pointer to the 1st argument */
    register char	*k2;	/* pointer to the 2nd argument */
{ 
    enum data_type type;        /* type of the keys */
    int length;                 /* length of the keys */

#ifdef    TRACESORT
    compare_count++;
#endif

    type = keyattr->type;
    length = keyattr->length;
    return (compare_key(k1, k2, type, length));
} /* end key_compare */
