
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



/********************************************************/
/*							*/
/*		Wisconsin Storage System		*/
/*		Version 2.0 July, 1984			*/
/*							*/
/*		COPYRIGHT (C) 1984			*/
/*		Computer Sciences Department		*/
/*		University of Wisconsin			*/
/*		Madison, WI 53706			*/
/*							*/
/********************************************************/


#include <wiss.h>

/* an utility for print a datum */

print_data(type, length, value)
enum data_type	type;
int		length;
register char	*value;
{

  if (type != TSTRING && type != TBITS)
    {
      double		z;

      movebytes((char *)&z, value, length);
      switch(type)
	{ 
	case TINTEGER : 
	  printf("(int)%4.4d", *(int *)&z); 
	  break;
	case TLONG :	
	  printf("(long)%4.4ld", *(long *)&z); 
	  break;
	case TSHORT :   
	  printf("(short)%2.2d", *(short *)&z); 
	  break;
	case TFLOAT:	
	  printf("(float)%4.4f", *(float *)&z); 
	  break;
	case TDOUBLE :  
	  printf("(double)%8.8f", *(double *)&z); 
	  break;
	} /* end switch */
    }
  else
    if (type == TBITS)
      {
	printf("0x \"");
	while ((length--) > 0) 
	  {
	    if (!(*value & 0xf0)) printf("0"); 
	    printf("%x ",(*(value++))&0xff);
	  }
	printf("\"");	
      }
    else
      {
	printf("\"");
	while ((length--) > 0) printf("%c", *(value++));
	printf("\"");	
      }

} /* print_data */


/*  Function Header 
 *
 * Name         : tbitscmp
 * Purpose      : compare two bitstrings lexicographically
 * Arguments    : b1, b2 pointers to the bitstrings to be compared,
 *		  l length at which to stop to compare (in bytes)
 * Return value : -1, 0 or 1 (b1<b2, b1==b2, b1>b2)
 * Side effects : none
 * Comments     : same as strncmp, but doesn't stop on a null character.
 */
/*--------------------------------------------------------------------*/
int tbitscmp(b1, b2, l)
     register char *b1, *b2;
     register int l;
{
/* removed the following as it won't work on a mips or sun4
  while(l > 3) {
    if(*(int *)b1 < *(int *)b2) return(-1);
    if(*(int *)b1++ > *(int *)b2++) return(1);
    l -= sizeof(int);
  }
*/

  while(l-- > 0) { /* modified by PHJ & JU */
    if((unsigned char)(*b1) < (unsigned char)(*b2)) return(-1);
    if((unsigned char)(*b1) > (unsigned char)(*b2)) return(1);
    b1++; b2++;
  }

  return(0);
}


/* 
    This routine compares the key fields of two records and returns
    0 if they are equal, 1 if k1 > k2, -1 otherwise.
*/

int
compare_key(k1, k2, type, length)
register char		*k1;	/* first key */
register char		*k2;	/* second key */
register enum data_type	type;	/* type of the keys */
register int		length;	/* length of the keys */
{
	char	*pz1, *pz2;
	double	z1, z2;	
	int	ri, rui;
	short	rs, rus;
	long	rl, rul;
	double	rd, rf;	
	int	e;

	if (type != TSTRING && type != TBITS)
	{ 
		/* copy arguments which *might* not be aligned 
			into local buffers */ 
 		pz1 = (char *) (&z1), pz2 = (char *) (&z2);
                for (; length > 0; length--)
                         *(pz1++) = *(k1++), *(pz2++) = *(k2++);
                k1 = (char *) (&z1), k2 = (char *) (&z2);
	}

	switch (type)
	{
		case TINTEGER:	
			ri = *(int *)k1 - *(int *)k2;
			return((ri == 0) ? 0 : (ri > 0) ? 1 : -1); 
	 	case TSHORT: 	
			rs = *(short *)k1 - *(short *)k2;
			return((rs == 0) ? 0 : (rs > 0) ? 1 : -1); 
	 	case TLONG: 	
			rl = *(long *)k1 - *(long *)k2;
			return((rl == 0) ? 0 : (rl > 0) ? 1 : -1); 
	 	case TFLOAT:	
			rf = *(float *)k1 - *(float *)k2; 
			return((rf == 0.0) ? 0 : (rf > 0) ? 1 : -1);
	 	case TDOUBLE: 	
			rd = *(double *)k1 - *(double *)k2; 
			return((rd == 0) ? 0 : (rd > 0) ? 1 : -1);
	 	case TSTRING:		
			e = strncmp(k1, k2, length);
			return(strncmp(k1, k2, length)); 
	 	case TBITS:		
			return(tbitscmp(k1, k2, length)); 
		case TUINTEGER:	
			rui = *(unsigned int *)k1 - *(unsigned int *)k2;
			return((rui == 0) ? 0 : (rui > 0) ? 1 : -1);
	 	case TUSHORT: 	
			rus = *(unsigned short *)k1 - *(unsigned short *)k2;
			return((rus == 0) ? 0 : (rus > 0) ? 1 : -1);
	 	case TULONG: 	
			rul = *(unsigned long *)k1 - *(unsigned long *)k2;
			return((rul == 0) ? 0 : (rul > 0) ? 1 : -1);
		default :
			return	-2221;	/* e2BADDATATYPE */
	} /* end switch */

#ifdef lint
	return(0);	/* to make lint happy */
#endif

} /* compare_key */

/*------------- to anable to apply the rcs command: what -----------*/
#ifndef lint
static char rcsid[] =
"@(#)$Header: data.c,v 1.4 89/06/05 11:21:30 futter Exp $ GIP-ALTAIR";
#endif
