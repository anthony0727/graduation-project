
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


#include <d_error.h>

/* #define READ_DEPTH	       	2 */	/* until first level tree leaves */
/* #define UPDATE_DEPTH	       	2 */	/* until first level tree leaves */

#define READ_DEPTH	       	6	/* until first level subtree leaves */
#define UPDATE_DEPTH	       	6	/* until first level subtree leaves */
#define	MAX_READ_NODES		((1 << (READ_DEPTH+1))-1)

/* the structure of the pile to cover trees */	  	
typedef struct {
  int	d;	/* depth (usefull for depth_first only) */
  RID	rid;
} t_rid;

#define NB_NODES(d)	((1 << (depth +1)) -1)	/* number of nodes  
						   accordingto the depth */
