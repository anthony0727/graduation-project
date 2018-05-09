
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


#include <wiss.h>
#include <wiss_r.h>
#include <lockquiz.h>
#include <locktype.h>


/* this version of build.h is for 4Kbyte wiss pages */

/* error handler */
#define	WISSERROR(p,c)	if((int)(c)<0) fatalerror(p,(int)(c))

#define FIFTY_FIFTY (random() % 2)

/* asume a random choice of 80% */
#define	RANDOM_80P ((random() % 10) < 8)

/* print rid's with format page/slot */
#define	DUMPRID(r)	printf(" %d/%d ", r.Rpage, r.Rslot)

/* database caracteristics */
#define TREE_DEPTH	2	/* 7 records per tree */
#define NB_TREE		1500	/* a database of 10500 records (300 pages) */
#define NB_REC_PER_PAGE	35	/* 5 trees per page */

/* involved caracteristics */
#define NB_LEAVES_PER_TREE (TREE_DEPTH * TREE_DEPTH)	     /* 4 leaves */
#define NB_LEAVES	   (NB_TREE * NB_LEAVES_PER_TREE)    /* 6000 leaves */
#define NB_REC_PER_TREE	   ((1 << (TREE_DEPTH+1)) -1) 	     /* 7 nodes */
#define NB_TREE_PER_PAGE   (NB_REC_PER_PAGE / NB_REC_PER_TREE) /* 5 trees */
#define NB_PAGES	   (NB_TREE / NB_TREE_PER_PAGE)	     /* 300 */
#define NB_INT_NODES	   (NB_REC_PER_TREE - NB_LEAVES_PER_TREE -1) * NB_TREE

/* An object is clustered within another object if it is within
a range of CLUST_SPREAD objects around the object */

#define CLUST_SPREAD	   25	/* 25 root objects */
#define SMEAR_RATE	   4	/* 1/4th of each node level is smeared */

/* the record length is defined according to NB_REC_PER_PAGE */
#define ALIGN      ((int)sizeof (int)) 			     /* see st.h */
#define RECSIZE	   108
#define RECFIXED   ((int)sizeof(int)+ 2*(int)sizeof(RID))
  		
/* the record structure */	  	
typedef struct {
  int	num;
  RID	left, right;
  char 	dummy[RECSIZE - (RECFIXED)];
} t_rec;
