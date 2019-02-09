
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


#include	<wiss.h>
#include	<wiss_r.h>
#include <lockquiz.h>
#include <locktype.h>


#define		VOLUME		"hp0a"
#define		NONCLUSTERED	1
#define		CLUSTERED	2
#define		NONUNIQUE	3

typedef struct 
	{ 
		int 	unique1;
		int 	unique2;
		int 	two;
		int 	four;
		int	ten;
		int	twenty;
		int	onePercent;
		int	tenPercent;
		int	twentyPercent;
		int	fiftyPercent;
		int	unique3;
		int	oddOnePercent;
		int	evenOnePercent;
	 	char	string1[52];
	 	char	string2[52];
	 	char	string3[52];
	} TUPLE;

#define RECSIZE sizeof(TUPLE)

extern	long time();
