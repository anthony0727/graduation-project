#include 	<wiss.h>
#include	<st.h>
#include        <lockquiz.h>


r_getrecordptr(pageptr, slotno, recptr)
DATAPAGE	**pageptr;	/* out parameter - pointer to page buffer */
int			slotno;
RECORD		**recptr;	/* out parameter - record address */
{
	int		e;	/* for returned errors */
	DATAPAGE	*dp;	/* pointer to the page buffer */

	/* BF_event(CurrentTask->ncb_name,"getrec",&pid,-1); */

	dp = *pageptr;
	e = eNOERROR;

	if (slotno < 0 || slotno >= dp->ridcnt)
		e = e2BADSLOTNUMBER;	/* bad slot number */
	else if (dp->slot[-slotno] == EMPTYSLOT)
		e = e2BADSLOTNUMBER;	/* bad slot number */
	else /* find the location of the record on the page */
		*recptr = (RECORD *)&(dp->data[dp->slot[-slotno]]);

	return(e);

}	/* r_getslot */