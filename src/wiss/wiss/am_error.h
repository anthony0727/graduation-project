
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




/* errors from level 3, */

#define	e3BADCURSOR	-132	/* 'bad scan cursor' */
#define	e3BADSCANTYPE	-232	/* 'scan type incompatible with operation' */
#define	e3BADRELOCATION	-332	/* 'incorrect relocation' */

#define	e3NOFILENO	-134	/* 'file number not found' */
#define	e3BADFILENO	-234	/* 'invalid file number' */
#define	e3BADSCANID	-334	/* 'invalid scanid' */
#define	e3SCANFILENOTMATCH	-434	/* 'the scanid does not belong with the file number' */
#define	e3NOMOREMEMORY	-534	/* 'no more memory space for scan table' */
#define	e3ACCESSVIOLATION	-634	/* 'attempt to update a read-only file' */
#define	e3NONEXTRID	-734	/* 'no more next RID - end of file' */
#define	e3NOPREVRID	-834	/* 'no more previous RID - end of file' */
