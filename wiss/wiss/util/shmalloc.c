
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


#include <wiss.h>
#include <page.h>
#include <sm.h>

extern SMDESC	*smPtr;	/* pointer to basic System V shared memory structures */

/* These routines are for allocating space out of System V shared memory */

/*	Heap management routines - circular first-fit algorithm 
  
  	Blocks are multiples of words aligned according to ALIGN.
  	Each block is tagged by a pointer to the next block.
  	The least significant bit of a tag pointer is a status bit
  	which tells whether a block is free (0) or busy (1).
   	The heap is bounded by two dummy blocks (with busy bit set).
  	One dummy block is located at the end of heap (pointed to 
  	by heap_top) and contains a pointer to the other dummy block 
   	located at the start of the heap.
  	Free blocks are coalesced when possible during space search.
  	ALIGNTTYPE is integer type to which a pointer can be cast.
  
*/

#define WSIZE	sizeof(union heapWord)
#define BUSY 	1
#define NULL	0
#define testbusy(p) 	((ALIGNTYPE)(p)&BUSY)
#define setbusy(p) 	(union heapWord *)((ALIGNTYPE)(p)|BUSY)
#define clearbusy(p) 	(union heapWord *)((ALIGNTYPE)(p)&~BUSY)

char *shmAlloc(size)
size_t size;
{
	size_t nwords;
	int	count;
	register union heapWord *p, *q;
	size_t	heapsize;

	if(smPtr->search_ptr == NULL) /* the first time */
	{ /* break heap into 2 dummy blocks at ends and a huge free block */
		smPtr->search_ptr = &smPtr->heap[0];
		heapsize = smPtr->heapSize;
		smPtr->heap_top = &smPtr->heap[heapsize - 1];
		smPtr->search_ptr->ptr = setbusy(&smPtr->heap[1]);
			/* bottom block */
		smPtr->heap[1].ptr = clearbusy(smPtr->heap_top);
			/* middle free block */
		smPtr->heap_top->ptr = setbusy(smPtr->search_ptr);	
			/* top block */
	}
	nwords = (size+2*WSIZE-1)/WSIZE;	/* how many words needed ? */
	for(p=smPtr->search_ptr, count = 0; ; )
	{
		if(!testbusy(p->ptr)) 
		{ /* search for a block and coalesce blocks whenever possible */
			while(!testbusy((q = p->ptr)->ptr)) p->ptr = q->ptr;
			if(q >= p+nwords && p+nwords >= p) break; /* found ! */
		}
		q = p;
		p = clearbusy(p->ptr);
		if(p < q) /* hit the top of heap, check for rounding up */
			if(q != smPtr->heap_top || p != smPtr->heap) 
				return(NULL);	/* what's wrong ? */
			else if(++count > 1)
				return(NULL);	/* no more space */
	}
	/* break the free block into a busy block and possibly a leftover */
	smPtr->search_ptr = p + nwords;
	if(q > smPtr->search_ptr) 	/* some leftover */
		smPtr->search_ptr->ptr = p->ptr; /* make it a free block */
	p->ptr = setbusy(smPtr->search_ptr);	/* mark the 1st portion busy */
	return((char *)(p + 1));	/* hide the tag and return the block*/
}

/*	
  Mark the block free and leave the dirty work to malloc.
*/
shmFree(p)
char *p;
{
	smPtr->search_ptr = ((union heapWord *) p) - 1;	/* back up to tag */
	smPtr->search_ptr->ptr = clearbusy(smPtr->search_ptr->ptr);
}

/* Allocate space for "num" objects each of which is "size"-byte long;
   the space is filled with 0's before handing to the requester */
char *shmCalloc(num, size)
unsigned num, size;
{
	char	*allocated;
	register int	*q;
	register int	count;

	num *= size;	/* convert to # of bytes */
	allocated = shmAlloc(num);
	count = (num+sizeof(int)-1)/sizeof(int);	/* how many ints ? */
	if(allocated != NULL)	/* null out the whole block with 0's */
		for (q = (int *) allocated; --count >= 0; *q++ = 0);
	return(allocated);
}

shmCfree(p, num, size)
char *p;
{
	shmFree(p);
}


