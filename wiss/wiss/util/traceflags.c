
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


/* module traceflags: eat trace options for WiSS */

#include	<wiss.h>

/* EXPORTS:
	Trace0
	Trace1
	Trace2
	Trace3
	Traceu
	wiss_checkflags()
*/

	/* first two characters, indicating this is a trace option */
#define		FLAG		'-'
#define		TRACEFLAG	'T'
	/* first trace char is level number */
#define		IOLEVEL		'0'
#define		BFLEVEL		'1'
#define		STLEVEL		'2'
#define		AMLEVEL		'3'
#define		UTILLEVEL	'u'
	/* character indicating all options */
#define		ALL		'+'

	/* translation, traceflag character to real value
	   0-9 => 0-9; a-v => 10-31 */
	/* if TRACEMAP becomes 2 words long, must extend the definition
	   of POS.  Code between ':' and ')' becomes
		(((c) >= 'a' && (c) <= 'z') ?
			(c)-'a'+10 :
			(c)-'A'+36)
	*/
#define		POS(c)		(((c) >= '0' && (c) <= '9') ? \
						(c)-'0'    : \
						(c)-'a' + 10)


TRACEFLAGS	Trace0, Trace1, Trace2, Trace3, Traceu;

static setflags (arg)
register char	*arg;
/* Set the flags according to the given argument string.  The address
   passed is a pointer to the character indicating the level; subsystem
   characters follow immediately.

   Imports:
	Trace0, Trace1, Trace2, Trace3, Traceu

   Returns:
	NONE

   Side Effects:
	Trace variables are modified

   Errors:
	Fatal error if bad trace level specified

   CROCKS:
	Hard coded limits for subsystem names.  If anyone can figure
	out a way to automate checking for the validity of these limits
	(failure at compile time if TRACEMAPSIZE!=BITSPERWORD), please do so!
*/
{
	register TRACEFLAGS	*t;	/* for abbreviation */

	switch (*arg)			/* which level? */
	{
	  case IOLEVEL:
		t = &Trace0;
		break;

	  case BFLEVEL:
		t = &Trace1;
		break;

	  case STLEVEL:
		t = &Trace2;
		break;

	  case AMLEVEL:
		t = &Trace3;
		break;

	  case UTILLEVEL:
		t = &Traceu;
		break;

	  default:			/* oooh, nasty */
		printf ("wiss_checkflags: illegal level %c\n", *arg);
		exit(1);
	}

	for (arg++;			/* point past level indicator */
	     (*arg >= '0' && *arg <= '9') ||
	     (*arg >= 'a' && *arg <= 'v') ||
	     (*arg == ALL);
	     arg++)
	{
		if (*arg == ALL)
			wsetmap(t, TRACEMAPSIZE);
		else
			setbit(t, POS(*arg));
	}
} /* setflags */


wiss_checkflags(argc,argv)
register int	*argc;
char		***argv;
/* Check the arguments to this program for traceflags for WiSS.  Remove
   these arguments from the list.  This routine will usually be called
   from main(), as follows:
	wiss_checkflags(&argc,&argv);

   Imports:
	setflags()	(static, defined in this module)

   Returns:
	*argc		updated count of arguments
	*argv		modified (in-place) argument list

   Side Effects:
	Sets trace flags, modifies argument list to remove trace arguments

   Errors:
	Fatal error if bad trace level specified
*/

{
	register int	count;
	char		**list;
	int		diff;
	register char	*arg;

		/* clear all the traceflags */
	clearmap (&Trace0, TRACEMAPSIZE);
	clearmap (&Trace1, TRACEMAPSIZE);
	clearmap (&Trace2, TRACEMAPSIZE);
	clearmap (&Trace3, TRACEMAPSIZE);
	clearmap (&Traceu, TRACEMAPSIZE);

	diff = 0;			/* no discrepancies yet */

	for (count = *argc, list = *argv;
	     count;
	     count--, list++)
	{
		arg = *list;
		if (arg[0] == FLAG && arg[1] == TRACEFLAG)
		{
			(*argc)--;		/* one less argument */
			diff++;			/* one more discrepancy */
			setflags (arg+2);	/* set the trace flags */
		}
		else		/* not a trace argument */
		{
			*(list - diff) = *list;	/* copy it over */
		}
	}
}
