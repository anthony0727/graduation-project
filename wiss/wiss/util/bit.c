
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



/* Module bit.c : routines for bit maps manipulation

   IMPORTS: NONE

   EXPORTS: Bit map manipulation routines

	clearmap(mapptr, mapsize) : clear all the bits in the map
	wsetmap(mapptr, mapsize) : set all the bits in the map
	countmap(mapptr, mapsize) : count and return the number of bit set 
	mapempty(mapptr, mapsize) : check if all the bits in the bit map 
		are cleared, return TRUE if so, FALSE otherwise
	setbit(mapptr, pos) : set the (pos)th bit in the bit map
	clearbit(mapptr, pos) : clear the (pos)th bit in the bit map
	checkset(mapptr, pos) : check if the (pos)th bit in the bit map is set,
		return TRUE if so, FLASE otherwise
	nextset(mapptr, mapsize, last) : given the position of the last bit 
		found, find the next bit set and return its position 
	nextclear(mapptr, mapsize, last) : given the position of the last bit 
		found, find the next bit cleared and return its position 

   NOTES: Bits are numbered from 0 through (mapsize - 1).
*/

#include	<wiss.h>

#define	tBITMAP	0

extern TRACEFLAGS Traceu;

printmap (mapptr,mapsize)
register unsigned char *mapptr;		/* start of bitmap */
int		mapsize;		/* size of bitmap (bits) */
/* print a given bit map (on the standard output) in hexdecimal form

   Returns:
	NONE

   Side Effects:
	bit map printed on the standard output decive in hexdecimal 

   Errors:
	NONE
*/
{
	register int i;
	int	bytes;

	bytes = (mapsize-1)/BITSPERBYTE + 1;
	printf("{");
	for (i = 0; i < bytes; i++)
		printf(i==0 ? "%03x" : " %03x", mapptr[i]);
	printf("}");
}


/*
    **************************************************************
    *            map-wise operations                             *
    **************************************************************
*/


clearmap(mapptr, mapsize)
register unsigned char	*mapptr;
register int		 mapsize;	/* start and size of the bit map */
/* clear all the bits in the given bit map

   Returns:
	NONE

   Side Effects:
	All the bits in the bit map cleared

   Errors:
	NONE
*/
{
	register int	count;

#ifdef TRACE
	if (checkset(&Traceu, tBITMAP))
	{
		printf("~clearmap (mapsize=%d, mapptr=", mapsize);
		printmap(mapptr, mapsize);
		printf(")\n");
	}
#endif

	for (count = (--mapsize / BITSPERBYTE) + 1; count--;  mapptr++)
		*mapptr = 0;	/* clear word */
}

wsetmap(mapptr, mapsize)
register unsigned char	*mapptr;
register int		 mapsize;	/* start and size of the bit map */
/* set all the bits in the given map

   Returns:
	NONE

   Side Effects:
	All the bits in the bit map cleared

   Errors:
	NONE
*/
{
	register int	count;

#ifdef TRACE
	if (checkset(&Traceu, tBITMAP))
	{
		printf("~wsetmap (mapsize=%d, mapptr=", mapsize);
		printmap (mapptr,mapsize);
		printf(")\n");
	}
#endif

	for (count = (--mapsize / BITSPERBYTE) + 1; count--;  mapptr++)
		*mapptr = ~0;	/* set word */
}


countmap(mapptr, mapsize)
register unsigned char	*mapptr;
register int		 mapsize;	/* start and size of the bit map */
/* count and return the number of bit set in the bit map

   Returns:
	Number of bit set in the bit map

   Side Effects:
	NONE

   Errors:
	NONE
*/
{
	register int	count, mask;

#ifdef TRACE
	if (checkset(&Traceu, tBITMAP))
	{
		printf("~countmap (mapsize=%d, mapptr=", mapsize);
		printmap(mapptr,mapsize);
		printf(")\n");
	}
#endif

	for (count = 0, mask = 1; mapsize; mapsize--)
	{
		if (*mapptr & mask)	/* bit set */
			count++;

		if ((mask<<=1) == (1<<BITSPERBYTE)) /* advance to next bit */
		{	/* cross word boundary */
			mask = 1;	/* reset mask to 1     */
			mapptr++;	/* advance map pointer */
		}
	}

	return(count);
}

mapempty(mapptr, mapsize)
register unsigned char	*mapptr;
register int		 mapsize;	/* start and size of the bit map */
/* check if all the bits in the given bit map are cleared

   Returns:
	TRUE if all the bits are cleared,
	FALSE otherwise

   Side Effects:
	NONE

   Errors:
	NONE
*/
{
	register int	 mask;

#ifdef TRACE
	if (checkset(&Traceu, tBITMAP))
	{
		printf("~mapempty (mapsize=%d, mapptr=", mapsize);
		printmap (mapptr,mapsize);
		printf(")\n");
	}
#endif

	for (mask = 1; mapsize; mapsize--)
	{
		if (*mapptr & mask)	/* bit set */
			return(FALSE);

		if ((mask<<=1) == (1<<BITSPERBYTE)) /* advance to next bit */
		{	/* cross word boundary */
			mask = 1;	/* reset mask to 1     */
			mapptr++;	/* advance map pointer */
		}
	}

	return(TRUE);
}


/*
    **************************************************************
    *            bit-wise operations                             *
    **************************************************************
*/


setbit(mapptr, pos)
register unsigned char	*mapptr;
register int	pos;		/* bit position */
/* set the (pos)th bit in the bit map

   Returns:
	NONE

   Side Effects:
	the (pos)th bit in the bit map set

   Errors:
	NONE
*/
{

#ifdef TRACE
	if (checkset(&Traceu, tBITMAP))
	{
		printf("~setbit (pos=%d, mapptr=", pos);
		printmap (mapptr, pos);
		printf("...)\n");
	}
#endif

	/* adjust word pointer and bit offset (pos) */
	mapptr += pos / BITSPERBYTE;
	pos %= BITSPERBYTE;

	*mapptr |= 1 << pos; 		/* set bit */
}

clearbit(mapptr, pos)
register unsigned char	*mapptr;
register int	pos;		/* bit position */
/* clear the (pos)th bit in the bit map

   Returns:
	NONE

   Side Effects:
	the (pos)th bit in the map cleared

   Errors:
	NONE
*/
{
#ifdef TRACE
	if (checkset(&Traceu, tBITMAP))
	{
		printf("~clearbit (pos=%d, mapptr=", pos);
		printmap(mapptr,pos);
		printf("...)\n");
	}
#endif

	/* adjust word pointer and bit offset (pos) */
	mapptr += pos / BITSPERBYTE;
	pos %= BITSPERBYTE;

	*mapptr &= ~(1 << pos);		/* clear bit */
}

checkset(mapptr, pos)
register unsigned char	*mapptr;
register int	pos;		/* bit position */
/* check if the (pos)th bit in the bit map is set

   Returns:
	TRUE if the (pos)th bit in the map is set
	FLASE otherwise

   Side Effects:
	NONE

   Errors:
	NONE
*/
{
#ifdef DEBUG
	if (Traceu != 0)
	{
		printf("~checkset (pos=%d, mapptr=", pos);
		printmap(mapptr,pos);
		printf("...)\n");
	}
#endif

	/* adjust word pointer and bit offset (pos) */
	mapptr += pos / BITSPERBYTE;
	pos %= BITSPERBYTE;
	
	return((*mapptr & (1 << pos)) ? TRUE : FALSE);
}

nextset(mapptr, mapsize, last)
register unsigned char	*mapptr;
int		 mapsize;	/* start and size of the bit map */
register int	last;		/* last bit position */
/* given the position of the last bit found, find the next bit set in the map

   Returns:
	the position of the next set bit in the map
		-1 if no more, or if bad "last found" value passed in

   Side Effects:
	NONE

   Errors:
	NONE
*/
{
	register int	mask;

#ifdef TRACE
	if (checkset(&Traceu, tBITMAP))
	{
		printf("~nextset(last=%d, mapsize=%d, mapptr=", last, mapsize);
		printmap(mapptr,mapsize);
		printf(")\n");
	}
#endif

	if (last < -1) 
		last = -1;
	else if (last >= mapsize)
		return(-1);

	/* set up starting bit position */
	mapptr += (++last) / BITSPERBYTE;
	mask = 1 << (last % BITSPERBYTE);

	for (mapsize -= last;		/* mapsize = # checkable bits left */
	     mapsize;			/* any bits left to check? */
	     last++, mapsize--)		/* inc position, dec counter */
	{
		if (*mapptr & mask)	/* next bit found */
			return (last);

		if ((mask<<=1) == (1<<BITSPERBYTE)) /* advance to next bit */
		{	/* cross word boundary */
			mask = 1;	/* reset mask to 1     */
			mapptr++;	/* advance map pointer */
		}
	}

	return(-1);
}


nextclear(mapptr, mapsize, last)
register unsigned char	*mapptr;
int		mapsize;	/* size of the bit map */
register int	last;		/* last bit position */
/* given position of the last bit found, find the next bit cleared in the map

   Returns:
	the position of the next cleared bit in the map

   Side Effects:
	NONE

   Errors:
	NONE
*/
{
	register int	mask;

#ifdef TRACE
	if (checkset(&Traceu, tBITMAP))
	{
		printf("~nextclear (last=%d, mapsize=%d, mapptr=",last,mapsize);
		printmap(mapptr, mapsize);
		printf(")\n");
	}
#endif

	if (last < -1 || last >= mapsize)
		return(-1);

	/* set up starting bit position */
	mapptr += (++last) / BITSPERBYTE;
	mask = 1 << (last % BITSPERBYTE);

	for (mapsize -= last;		/* mapsize = # checkable bits left */
	     mapsize;			/* any bits left to check? */
	     last++, mapsize--)		/* inc position, dec counter */
	{
		if ((*mapptr & mask) == 0)	/* next bit found */
			return (last);

		if ((mask<<=1) == (1<<BITSPERBYTE)) /* advance to next bit */
		{	/* cross word boundary */
			mask = 1;	/* reset mask to 1     */
			mapptr++;	/* advance map pointer */
		}
	}

	return(-1);
}
