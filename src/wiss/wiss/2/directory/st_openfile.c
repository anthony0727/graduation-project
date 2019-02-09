
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
/* Module st_openfile : level 2 interface routine for opening a file 

   IMPORTS :
	sys_getuser();
	bf_openfile(filenum, mode)
	ST_accessfiledesc();

   EXPORTS :
	st_openfile (volid, filename, accessmode)

*/

#include <wiss.h>
#include <st.h>
#include <lockquiz.h>

st_openfile(volid, filename, accessmode)
TWO	volid;		/* volume id */
char	*filename;	/* name of the file */
int	accessmode; 	/* READ or WRITE */

/* 
   Open the specified file and return an open file number.
   A entries in the open file table is allocated first, then the
   descriptor of that file is fetched from the file directory on volume.

   Side Effects:
	None

   Returns:
	open file number

   Errors:
	e2TOOMANYOPENFILES
	e2UNKNOWNMODE
	e2MODECONFLICT
*/
{
	register int i;		/* table index */
	register int e;		/* for returned errors */
	FOUR	user;		/* who am i */
	int	my_file;	 
	int	new_ofn;	/* the new open file # */

#ifdef TRACE
	if (checkset(&Trace2,tINTERFACE))
		printf("st_openfile(volid=%d,filename=%s,accessmode=%d)\n",
			volid, filename, accessmode);
#endif

	if(accessmode != WRITE && accessmode != READ) return(e2UNKNOWNMODE);

	SetLatch(&smPtr->level2Latch, procNum, NULL);
	user = sys_getuser();

	/* find an empty table entry */
	for (i = 0; i < MAXOPENFILES && smPtr->files[i].ptr >= 0; i++);
	if (i == MAXOPENFILES) 
        { 
	      ReleaseLatch(&smPtr->level2Latch, procNum);
              return(e2TOOMANYOPENFILES);
        }
	new_ofn = i;	/* open file # for this instance */

	/* Is the file already open by me ? */
	my_file = -1;
	for (i = 0; i < MAXOPENFILES; i++)
	{
		if (smPtr->files[i].ptr < 0) continue;
		if (F_STATUS(i) == NOTINUSE) continue;	/* an invalid entry */
		if (F_VOLUMEID(i) != volid) continue;	/* different volume */
		if (strcmp(F_FILENAME(i),filename)) continue;/* not this file */
		if (F_USER(i) == user)
		{
			my_file = i;	/* I have open the file before */
			continue;	/* but keep checking */
		}
		if (F_ACCESSMODE(i) == WRITE || accessmode == WRITE)
                {
			ReleaseLatch(&smPtr->level2Latch, procNum);
			return(e2MODECONFLICT);	 /* too much writers ! */
                }
	}

	if (my_file >= 0)
	{ 
	    /* increment reference count, and adjust the permission if needed */
	    if (F_ACCESSMODE(my_file) == READ && accessmode == WRITE)
			F_ACCESSMODE(my_file) = WRITE;	/* update table */
	    F_REFCOUNT(my_file) += 1;	/* one more instance */
	    smPtr->files[new_ofn].ptr = smPtr->files[my_file].ptr;
	}
	else /* open it for the 1st time by me */
	{
	    for (i = 0; i < MAXOPENFILES; i++)
		if (smPtr->filetable[i].status == NOTINUSE) break;
	    if (i == MAXOPENFILES)		/* table full ? */
            {
		ReleaseLatch(&smPtr->level2Latch, procNum);
		return(e2TOOMANYOPENFILES);
            }

	    smPtr->files[new_ofn].ptr = i;

	    /* get file descriptor from file directory, and open the file */
	    e = ST_accessfiledesc(volid, filename, &F_DESC(new_ofn), READ); 
	    CHECKERROR_RLATCH(e,&smPtr->level2Latch, procNum);
	    e = bf_openfile(new_ofn, accessmode);
	    CHECKERROR_RLATCH(e,&smPtr->level2Latch, procNum);

	    /* fill in table entry */
	    F_STATUS(new_ofn) = CLEAN; 		
	    F_VOLUMEID(new_ofn) = volid;
	    F_REFCOUNT(new_ofn) = 1;
	    F_ACCESSMODE(new_ofn) = accessmode;
    /*
	    F_LOCKMODE(new_ofn) = l_NL; 
    */
	    InitLatch(F_LATCHPTR(new_ofn)); /* initialize the file's latch */
		/* this latch is currently used only by appendpage but */
		/* the directory routines more concurrency could be obtained */
		/* by having each the directory routines latch individual */
		/* files rather than just setting the level2Latch.  It is */
		/* not clear whether this is worthwhile */
	}

	smPtr->files[new_ofn].mode = accessmode;
	F_USER(new_ofn) = user;

	/* check permission */
	if (user == F_OWNER(new_ofn))
		i = (accessmode == READ) ? OWNERREAD : OWNERWRITE;
	else /* others */
		i = (accessmode == READ) ? OTHERREAD : OTHERWRITE;

	if ((i & F_FLAGS(new_ofn)) == 0) /* permission violation */
	{
		if (F_REFCOUNT(new_ofn) == 1)
		{
			(void) bf_closefile (new_ofn);
			F_STATUS(new_ofn) = NOTINUSE;
		}
		else
			F_REFCOUNT(new_ofn)--;
		smPtr->files[new_ofn].ptr = -1;
		ReleaseLatch(&smPtr->level2Latch, procNum);
		return(e2PERMISSIONDENIED);
	}
	/* finally, mark all pinning information invalid */
	F_LASTPINNED(new_ofn)=FALSE;
	F_LASTLOCKED(new_ofn)=FALSE;
	PIDCLEAR(F_LASTPIDPINNED(new_ofn));
	ReleaseLatch(&smPtr->level2Latch, procNum);

	return(new_ofn);	/* return open file number */

}  /* st_openfile */

