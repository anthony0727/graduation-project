
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



/* module sufname.c : routine for appending suffixus to a name */

/* EXPORTS: 
	suffixname(sname, oname, stype, snum)

   IMPORTS: NONE
*/

suffixname(sname, oname, stype, snum)
char	*sname;		/* the name with suffix */
char	*oname;		/* the original name */
char	stype;		/* type of the new name - a character */
int	snum;		/* suffix number */	

/* 
   This appends two suffixus to a name.
   The first suffix is a character, and the second is a number (< 1000).

   Imports:
	NONE

   Returns:
	A suffixed name.

   Errors:
	NONE
*/
{

	char		digits[4];
	register	i;

	while (*oname) 
		*(sname++) = *(oname++);
	*(sname++) = '.';
	*(sname++) = stype;

	if (snum == 0)
		*(sname++) = '0';
	else
	{
		for (i = 0; snum > 0; i++, snum /= 10)
			digits[i] = (char) (snum % 10 + '0');
		for (--i; i >= 0; i--)
			*(sname++) = digits[i];
	}
	*(sname++) = '\0';

}


