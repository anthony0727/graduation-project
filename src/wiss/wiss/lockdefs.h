
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


#define DEADLOCK    -1000     /* Returned value in case of deadlock */
#define WAITING     -1001     /* Returned value in case of a phantom trans */
#define REACQUIRE   -1004
#define FILEDELETED -1003     /* The file is deleted, no longer in existance
	
#define MAX_RES_LOG  15
/* (MAXRES -1) computed  for hash_page and hash_file   */
#define	MAXRES	        32768  
#define MAXRES_1	32767	

#define MAXNODE	     	16384	/* (1 << 14)   */
#define MAXWAITNODE  	1024	/* (1 << 10)   */

/* The hash function for hashing resources in their appropriate place */
#define hash_page(page_id)  ((page_id.Ppage * page_id.Pvolid) & (MAXRES_1))
#define hash_file(file_id)  ((file_id.Fvolid * file_id.Ffilenum) & (MAXRES_1))

#define	FREECELL  0	       /* Where free list starts */ 
#define	FREEWAIT  0            /* Where header of free wait list is */

/*
 *  The lock node is designed specifically so that a latch can be
 *  placed at the head of each linked list of resource.  Its main purpose
 *  is that everytime one wants to enter a node into linked list of
 *  one specific location of the array the whole array won't have to be
 *  locked.  Instead only the head of that linked list is locked. In fact
 *  I am trying to increase concurrency, avoiding unnecessary waits.
 */
struct locknode {
	LATCH		concurrent;
	struct node     *lockptr;    /* The head of linked list */
};

extern short LM_conv[MAXLOCKTYPES][MAXLOCKTYPES];
extern short LM_compat[MAXLOCKTYPES][MAXLOCKTYPES];
extern short LM_supr[MAXLOCKTYPES][MAXLOCKTYPES];
extern char LM_locknames[MAXLOCKTYPES][4];
extern short LM_leaftype[MAXLOCKTYPES];
