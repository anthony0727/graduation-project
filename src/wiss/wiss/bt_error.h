
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




/* errors from level 2, b tree section */

#define	e2NONEXTKEY	-122	/* No next index */
#define	e2NOPREVKEY	-222	/* No previous index */
#define	e2NOFIRSTINDEX	-322	/* No first index */
#define	e2NOLASTINDEX	-422	/* No last index */
#define	e2NONEXTRID	-522	/* CurrRID is the last RID for this key */
#define	e2NOPREVRID	-622	/* CurrRID is the first RID for this key */

#define	e2DUPLICATEKEY	-123	/* Duplicate key found during construction of a primary index */
#define	e2DUPLICATEKEYPTR	-223	/* Duplicate key found in node pages of the B-tree */

#define	e2KEYNOTFOUND	-323	/* cannot find the key */

#define	e2INDEXNUMTOOLARGE	-128	/* Index number too large (createindex) */
#define	e2KEYLENGTHTOOLONG	-228	/* Key too long (createindex) */
#define	e2BADSLOTCOUNT	-328	/* negative slot count in bt_move_entries */

#define	e2KEYALREADYEXISTS	-428	/* Key already exists. */
#define	e2FILENAMETOOLONG	-528	/* File name too long. DUPLICATE ERROR CODE. */
#define	e2NORIDMATCH	-628	/* the index to be deleted is not found */
#define	e2ILLEGALCURSOR	-728	/* bad cursor for index scan */

