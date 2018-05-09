#include 	<wiss.h>
#include	<st.h>
#include        <lockquiz.h>


st_getrecordptr(pageptr, slotno, recptr)
DATAPAGE	**pageptr;	/* out parameter - pointer to page buffer */
int			slotno;
char		**recptr;	/* out parameter - record address */
{
	int		e;	/* for returned errors */
	RECORD		*record;
	
	/* get a pointer to the WiSS record  in record */
	e = r_getrecordptr(pageptr, slotno, &record);

	if (e < eNOERROR)
	{
		return(e);
	}
	else
	{
		*recptr = &record->data[0];
		return (record->length);
	}
}
