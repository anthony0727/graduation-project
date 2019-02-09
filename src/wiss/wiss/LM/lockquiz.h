
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


#define NOT_OK -1
#define OK 1
#define MAXNAME 256

#define GOTPAGETRUE 1
#define GOTPAGEFALSE 0

#define ABORTED  	-1250
#define GRANTED  	0
#define NOT_GRANTED  	-1450
#define WAIT  		-1550
#define COND_ABORTED 	-1650
#define COND_WAIT  	-1750

#define l_NL 0
#define l_IS 1
#define l_IX 2
#define l_S 3
#define l_SIX 4
#define l_X 5

#ifdef HIERARC_LOCK
#define WAIT_AND_REASK  -1850
#endif
