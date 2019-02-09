
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



/*
 * These are the declarations needed for the table-driven lock manager
 * The lock modes in this file describe the tables for the hierarchical
 * locks described by Gray's notes.
 */

typedef short LOCKTYPE;

typedef enum {INSTANT, MANUAL, COMMIT} DURATION;
enum    belongsto  {FILER, PAGER, NOBODY};     /* Possible owners of resource */

#define MAXLOCKTYPES 6
#define LEGAL 0
#define ILLEGAL -1
#define LM_ILLEGALCONV -2
#define LM_ILLEGALMODE -3

