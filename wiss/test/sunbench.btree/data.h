
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



/*define types needed for the sun benchmark*/

#define NUMFROMSLOTS 5

typedef struct {
    RID		part;
    int		length;
    char	type[10];
} TOCONNECT;

typedef struct {
    int		partId;
    int		x,y;
    long	date;
    char	type[10];
    TOCONNECT	to[3];
    short	fromCnt;   /* actual number of from slots in use */
    short	fromSpace; /* actual number of from slots allocated */
    RID 	from[NUMFROMSLOTS];
} PART;

#define HNO		11

#define EXPANDAMOUNT NUMFROMSLOTS*sizeof(RID)
