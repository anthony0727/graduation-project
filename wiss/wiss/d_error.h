
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



/* errors from level 2,  file directory and open file table, sections */

#define	e2VOLNOTOPEN	-4021	/* volume not mounted at level 2 */
#define	e2NOSUCHFILE	-4121	/*  no file by that name  */
#define	e2VOLALREADYOPEN	-4221	/* volume already open at level 2  */
#define	e2FILEALREADYEXISTS	-4321	/*  this file already exist */
#define	e2NOMOREMEMORY	-4421	/*  calloc failed  */
#define	e2TOOMANYVOLUMES	-4521	/*  too many volumes currently in use  */
#define	e2FILESTILLOPEN	-4621	/*  attempt to remove a currently open file  */

#define	e2BADOPENFILENUM	-3021	/*  openfilenum is out of range  */
#define	e2WRONGUSER	-3121	/*  this openfilenum belongs to another user  */
#define	e2TOOMANYOPENFILES	-3221	/*  too many open files  */
#define	e2UNKNOWNMODE	-3321	/* unknown access mode */
#define	e2MODECONFLICT	-3421	/* access modes conflict */
#define	e2UNKNOWNPROT	-3521	/* unknown protection mode */
#define	e2PERMISSIONDENIED	-3621	/* file permission denied */
#define	e2NOPERMISSION	-3721	/* no file permission */

