
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



/* errors from level 2, records sections */

#define	e2NAMEINUSE	-121	/* file name already in use */
#define	e2BADSLOTNUMBER	-221	/* an invalid slot number */
#define	e2LISTOFEMPTYSLOTS	-321	/* empty slots at end of page */	/* an inconsistency */
#define	e2NOROOMONPAGE	-421	/* no room on page for expansion */

#define	e2NULLRIDPTR	-521	/* null RID pointer */
#define	e2NULLPIDPTR	-621	/* null PID pointer */
#define	e2NULLFIDPTR	-721	/* null FID pointer */
#define	e2NULLPAGEPTR	-921	/* null page buffer pointer  */
#define	e2NULLRECADDR	-821	/* null record buffer address */
#define	e2DELNOEXTREC	-1021	/* delete a non-existing record */
#define	e2PAGENOTINFILE	-1121	/* the page referenced is not in the file */

#define	e2ENDOFFILE	-1621	/* end of file in st_nextfile/st_prevfile */
#define	e2RECWONTFIT	-1721	/* no room for record on this page */
#define	e2CANTINSERTREC	-1821	/* st_insertrecord fails for unknown reason */
#define	e2VOLUMESTILLACTIVE	-1921 	/* attempt to dismount a volume with open files */

#define	e2NOMORESLICES	-2121	/* long data item grows exceed limit */
#define	e2BADDATATYPE	-2221	/* bad data type in st_compare */
#define	e2ILLEGALOP	-2321	/* illegal operator found in st_compare */
