#include	<wiss.h>
#include	<st.h>

st_getdevaddr(volid)
TWO volid;
{
	return io_getdevaddr(volid);	/* mount the device */
} /* st_getdevaddr */
