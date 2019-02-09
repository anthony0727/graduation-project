#include	<wiss.h>
#include	<io.h>

extern	char *strcpy();

io_getdevaddr(volid)
TWO volid;
{
	register int	i;		/* index into VolDev of info */

	/* see if it's already mounted ? */
	for (i = 0; i < MAXVOLS; i++)
		if (smPtr->VolDev[i].VDvolid == volid)
		{
			return openFileDesc[i];
		}
} /* io_mount */
