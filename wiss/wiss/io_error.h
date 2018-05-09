
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




/* error codes returned by level 0 */

#define	e0DEVSEEKERROR	-101	/* device seek error */
#define	e0DEVREADERROR	-201	/* device read error */
#define	e0DEVWRITEERROR	-301	/* device write error */
#define	e0MOUNTFAILED	-401	/* device mount failed */
#define	e0DISMOUNTFAILED	-501	/* device dismount failed */

#define	e0VOLNOTMOUNTED	-102	/* referenced volume not mounted */
#define	e0TOOMANYVOLS	-202	/* too many volumes mounted */
#define	e0TOOMANYFILES	-302	/* too many files created */
#define	e0NOSPACEONDISK	-402	/* insufficient space on disk */
#define	e0NOMOREMEMORY	-502	/* insufficient main memory */
#define	e0VOLMOUNTED	-602	/* referenced volume is mounted  */

#define	e0FIDPIDNOTMATCH	-103	/* volume IDs in the FID and PID are inconsistent */
#define	e0FILENOTINUSE	-203	/* reference to a non-existent file */
#define	e0INVALIDPID	-303	/* invalid page number */
#define	e0INVALIDFID	-403	/* invalid file number */
#define	e0BADHEADER	-503	/* bad volume header */

#define	e0NULLPIDPTR	-104	/* null PID pointer */
#define	e0NULLFIDPTR	-204	/* null FID pointer */
#define	e0NULLBUFPTR	-304	/* null memory buffer pointer */
