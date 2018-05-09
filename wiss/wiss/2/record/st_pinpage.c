#include 	<wiss.h>
#include	<st.h>
#include        <lockquiz.h>

st_pinpage(filenum, pidptr, pageptr, trans_id, lockup, mode, cond)
int		filenum;	/* open file number */
PID		*pidptr;	/* *PID */
DATAPAGE	**pageptr;	/* out parameter - pointer to page buffer */
int             trans_id;
short           lockup;
int		mode;
short           cond;
{
	int		e;	/* for returned errors */
	DATAPAGE	*dp;	/* pointer to the page buffer */

	/* get a pointer to the WiSS page  in page */
	e = r_pinpage(filenum, pidptr, &dp, trans_id, lockup, mode, cond);

	if (e >= eNOERROR)
	{
		*pageptr = dp;
	}

	return e;
}   /* st_getpage */