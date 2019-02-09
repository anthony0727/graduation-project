
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


#
/* Module st_chmod : the the protection mode of a file

   IMPORTS :
	sys_getuser();
	ST_accessfiledesc();

   EXPORTS :
	st_chmod (volid, filename, accessmode)

*/

#include <wiss.h>
#include <st.h>

st_chmod(volid, filename, mode)
int	volid;
char	*filename;
int	mode;		/* the new mode */
{
	FILEDESC	fd;
	int		e;
	register int	i;

#ifdef TRACE
	if (checkset(&Trace2,tSTAT))
		printf("st_chmod(volid=%d,filename=%s,mode=%d)\n",
			volid, filename, mode);
#endif

	if (mode & ~(ALL_PERM))
		return(e2UNKNOWNPROT);	/* unknown protection mode */

	SetLatch(&smPtr->level2Latch, procNum, NULL);

	for (i = 0; i < MAXOPENFILES; i++)
	{
		if (smPtr->files[i].ptr < 0) continue;
		if (F_VOLUMEID(i) != volid)
			continue;	/* different volumes */
		if (!strcmp(F_FILENAME(i),filename)) /* same file ? */
			break;	/* found */
	}

	if (i < MAXOPENFILES)
	{ 
		/* the descriptor is cached in table */
		if (sys_getuser() != F_OWNER(i))
                { 
			ReleaseLatch(&smPtr->level2Latch, procNum);
			return(e2PERMISSIONDENIED);
                }
		F_STATUS(i) = DIRTY;
		F_FLAGS(i) = mode;
	}
	else
	{ /* have to update the descriptor on disk */
		/* get file descriptor from file directory */
		e = ST_accessfiledesc(volid, filename, &fd, READ); 
		CHECKERROR_RLATCH(e,&smPtr->level2Latch, procNum);
		if (sys_getuser() != fd.owner)
                { 
			ReleaseLatch(&smPtr->level2Latch, procNum);
			return(e2PERMISSIONDENIED);
                }
		fd.flags = mode;
		e = ST_accessfiledesc(volid, filename, &fd, WRITE);
		CHECKERROR_RLATCH(e,&smPtr->level2Latch, procNum);
	}
	ReleaseLatch(&smPtr->level2Latch, procNum);
	return(eNOERROR);
}
