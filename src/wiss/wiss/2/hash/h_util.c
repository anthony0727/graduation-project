
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



/* Module h_util - hash file utility

   IMPORTS :
	bf_setdirty(filenum, pageid, pageptr)

   EXPORTS:
	int h_hash(key_value, key_length, bitlength)
	h_initpage(filenum, pid, pageptr, key_type, unique)
*/

#include	<wiss.h>
#include	<st.h>

int
h_hash(key, length, bitlength)
register char	*key;		/* the hash key */
register int	length;		/* length of the key */
int		bitlength;	/* how many bits to use */

/* hash fouction : h(key) = sum(k1, k2,...) mod (2 ** bit length)
			where key = k1.k2.k3...			 */
{
	register int	value;	/* accumulator */

	for (value = 0; length > 0; length--)  
		value += (int)(*(key++));
	if (value < 0) value = -value;
	return(value % (1 << bitlength));	/* return rightmost bits */

} /* h_hash */
	

h_initpage(filenum, pageid, bpage, keytype, unique)
int		filenum;	/* which file */
PID		*pageid;	/* which page */
BTREEPAGE	*bpage;		/* buffer pointer of the new page */
ONE		keytype;	/* key type */
ONE		unique;		/* primary or secondary key */

/* This routine initializes the control field entries of a new bucket.  

   RETURNS:
	None
  
   SIDE EFFECTS:
	None

   Errors:
	None 
*/
{
	register BTCONTROL *p;		/* ptr to beginning of control info */

#ifdef	TRACE
	if (checkset(&Trace2, tHASHFILE)) {
		printf("h_initpage(filenum=%d, pageid=%d:%d, ", 
			filenum, pageid->Pvolid, pageid->Ppage);
		printf("bpage=0x%x, keytype=%d, unique=%d)\n",
			bpage, (int)keytype, unique);
	}
#endif

	/* set a pointer p to the control structure of the new page
	   and initialize the control information fields */
	p = &(bpage->btcontrol);

	p->startdata = p->enddata = p->numoffsets = 0;
	p->fileid = F_FILEID(filenum);
	p->thispage = *pageid;
	p->numfree = MAXSPACE(bpage);
	p->keytype = keytype;
	p->pagetype = LEAFPG;
	p->treetype = HASH;
	p->unique = unique;

	/* clear all page links */
	p->next = p->prev = p->overflow = NULLPAGE;

	(void) bf_setdirty(filenum, pageid, (PAGE *) bpage);

} /* h_initpage */

