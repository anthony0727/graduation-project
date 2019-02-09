
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


 
/*************************************/
/*     Multimark benchmark : build.v3    */
/*************************************/

#include "build.h"
#include <sys/time.h>
#include <sys/resource.h>


/* 

	Usage:	build <deviceName> <fileName> <mode> 

	<deviceName> is the name of the wiss volume to use
	<fileName> is the name of the wiss file to use
	<mode> can assume one of 3 values:

		0  if neither phase of PASS 3 is to be executed

		1  if the first phase of PASS 3 is to be executed
			but not the second phase 

		2  if the second phase of PASS 3 is to be executed
			but not the first phase 

		3  if both phases of PASS 3 are to be executed



This version of the program creates two files.  The first file contains
all the objects.  The second file contains the oids of the root objects 
*/

/*
 PASS 1
 ------
 Let's the value of an object be a binary tree of 7 records (TREE_DEPTH = 2):
 
                         --------------------
                         |                  |
                         --------------------
                             /         \
                          /               \
          ---------------------       --------------------- 
          |                   |       |                   |
          ---------------------       --------------------- 
                 /       \                    /     \
              /             \                /       \
 ----------------    -----------------  ---------- ------------
 |              |    |               |  |        | |          |
 ----------------    -----------------  ---------- ------------

 And here is a record structure:
       ------------------------------------------------------------------
       | rec   | key | left | right |             <datas>               |
       |header |     |      |       |                                   |
       -------------------------------------------------------------------

 The record header is the room reserved by Wiss for the record managment 
 (4 bytes).
 The key is a sequential number given by the record creation order. 
 The data field is a dummy field to simulate a real record size. This
 field size is calculated according to the number of records we want per
 page (35 records per page ==> record length = 115 bytes).

 PASS 2
 ------
  forms complex records

PASS1 and PASS2 are always executed.   the 

 PASS 3
 ------
 Some records are smeared as explain below. that simulates updated records 
 which might be moved by wiss if they have grown.

 roots children switches:

 	an array of 50% of the tree roots is randomly made. 
	then we randomly switch or not 50% of the left children 
	rids and/or of the right children rids, by picking sequential 
	rid pairs in the tree root array.

 interior-node children switches:
 	we do the same as above on an array of interior nodes.

 */

RID create_tree();	/* recursive function */
char *strcat();

RID *a_roots;		/* array of roots */
int i_roots =0;		/* index for a_roots */
RID *a_ints;		/* array of interior nodes */
int i_ints =0;		/* index for a_ints */
RID *a_leaves;		/* array of all the leaves */
int i_leaves =0;		/* index for a_leaves */

int transId;
char    *deviceName;

main(argc, argv)
int	argc;	
char	**argv;
{
	int	e;		/* for returned flags */
	int	vol;		/* volume id */
	int	ofn;		/* open file number */
	t_rec 	rec, rec1;	/* buffers to describe one level of an object */
	RID	rid;		/* physical record id */
	int	n;		/* to iterate on trees */
	int	key;		/* single record key */
	PID	nil_pid;	/* a usefull null page id */
	int 	i, j;		/* to iterate on a_leaves */
	int	rnd;
	int	mode;
	char    *fileName;
	long	startTime, endTime;
	int	ofnrids;
	RID	rootrid;
	char	ridfile[256];
	int	clust_lower_bound;	/* clustered roots area lower bound */
	int	clust_higher_bound;	/* clustered roots area higher bound */
	int	clusterSpread;
	int	clusterPercent;	/* percentage of children nodes that
		are clustered near their parent object.  if 0, then
		the children of a leaf node will be randomly chosen
		from the full range of NB_TREE objects.  if 100% then
		the children of a leaf node will be chosen from one 
		of the objects within the CLUST_SPREAD objects around the
		object */

	struct rusage ru;

	int leafstart;
	int status;
	int	verboseFlag;

	/* array of the tree roots, of interior nodes, and of all the leaves */
	a_roots = (RID *)malloc(NB_TREE*sizeof(RID));
	if (!a_roots) {
	  printf("not enough room for malloc \n");
	  exit(1);
	}
	a_ints = (RID *)malloc(NB_INT_NODES * sizeof(RID)); 
	if (!a_ints) {
	  printf("not enough room for malloc \n");
	  exit(1);
	}
	a_leaves = (RID *)malloc(NB_LEAVES * sizeof(RID));
	if (!a_leaves) {
	  printf("not enough room for malloc \n");
	  exit(1);
	}

	if (argc < 6) {
	  printf("usage: build device filename mode(0-3) %%clustered verboseFlag\n");
	  exit();
	}

	deviceName = argv[1];
	fileName = argv[2];
	mode = atoi(argv[3]);
	clusterPercent = atoi(argv[4]);
	if (strcmp(argv[5],"TRUE") == 0) verboseFlag = TRUE;
	else verboseFlag = FALSE;


	printf("DevName=%s FileName=%s Mode=%d %%Clustered=%d \n",
	 	deviceName, fileName, mode, clusterPercent);

	/* warm up */
	(void) wiss_init();

	transId = begin_trans();
	printf("new transaction id = %d\n",transId);

	/* mount the device called deviceName */
	vol = wiss_mount(deviceName);	
	WISSERROR("build/wiss_mount", vol);

	strcpy (ridfile, fileName);
	strcat(ridfile,".rids");
	wiss_destroyfile(vol, fileName, transId, FALSE, FALSE);
	wiss_destroyfile(vol, ridfile, transId, FALSE, FALSE);

	startTime = time(NULL);

	/* create a wiss file which has NB_PAGES pages initially, both */
	/* the extent fill factor and page fill factor are 100% */
	e = wiss_createfile(vol, fileName, NB_PAGES, 100, 100);
	WISSERROR("build/wiss_createfile", e);


	/* create a wiss file for hold the object ids of the root objects */
	e = wiss_createfile(vol, ridfile, ((NB_TREE * sizeof(RID))/PAGESIZE)+1,
		100, 100);
	WISSERROR("build/wiss_createfile", e);

	/* open the file where the root rids are to be stored */
	ofnrids = wiss_openfile(vol, ridfile, WRITE);
	WISSERROR("build/wiss_openfile", ofnrids);

	e = wiss_lock_file(transId, ofnrids, l_IX,  COMMIT,  FALSE);
	WISSERROR ("build/wiss_lock_file", e);

	/* open the file where the trees are to be stored */
	ofn = wiss_openfile(vol, fileName, WRITE);
	WISSERROR("build/wiss_openfile", ofn);

	e = wiss_lock_file(transId, ofn, l_IX,  COMMIT,  FALSE);
	WISSERROR ("build/wiss_lock_file", e);

	/* create NB_TREE trees on disk */
	/* PASS #1 */

#ifdef TRACE
	  printf("create %d trees\n:", NB_TREE);
#endif

	for (n=1, key=0; n<=NB_TREE; n++){
/*
	  printf("\n\n%d: root key = %d \n", n, key+1);
*/
	  PIDCLEAR(nil_pid);
	  rid = create_tree(ofn, &key, 2, &nil_pid);
	  e = wiss_appendfile(ofnrids, &rid, sizeof(RID), &rootrid, 
		transId, TRUE, FALSE);
	  if (e < eNOERROR) printf("append file error, n=%d, e=%d\n",
		n, e);
	    
	  WISSERROR("build/wiss_appendfile", e);
	}
	e = wiss_closefile(ofnrids);
	WISSERROR("test1/wiss_closefile", e);

	/* open the file where the root rids are stored */
	ofnrids = wiss_openfile(vol, ridfile, WRITE);
	WISSERROR("build/wiss_openfile", ofnrids);

	/*  END OF PASS #1 */

	/*  PASS #2 */

	/* attach clustered tree(s) and/or unclustered tree(s) to the leaves */
/*
	printf("\nattach trees to the leaves\n");
*/
	leafstart = 0;   /* leafstart points to the lower bound of the
		leaves associated with a given root */
	clusterSpread = CLUST_SPREAD;

	/* for each root */
	for (i = 0; i < NB_TREE; i++, leafstart += 4)
	{
	    /* for each of the root's leaves */
	    for (j = leafstart; j < (leafstart + NB_LEAVES_PER_TREE); j++)
	    {
	        /* read the leaf from disk */
	        e = wiss_readrecord(ofn, &a_leaves[j], &rec, sizeof(rec), transId,
		TRUE,  l_X, FALSE);
	        WISSERROR("build/wiss_readrecord", e);

	        /* calculate the lower bound of the clustered roots area */
	        clust_lower_bound = i - (clusterSpread/2);
		if (clust_lower_bound < 0) clust_lower_bound = 0;
		
	        clust_higher_bound = clust_lower_bound + clusterSpread -1;

	        if (clust_higher_bound >= NB_TREE)
		{
		    clust_higher_bound = NB_TREE - 1;
	            clust_lower_bound = clust_higher_bound - clusterSpread + 1;
		}
/*
		printf("i=%d, clust_lower_bound=%d, clust_upper_bound=%d\n",
			i, clust_lower_bound, clust_higher_bound);
*/

		/* first assign an object to the left child of the
		leaf.  Do this by generating a random value between
		0 and 99.  If this value is less than the value of 
		clusterPercent,  the child is assigned an object which
		is clustered nearby.  Otherwise, random object
		is selected */

	        if ((random() % 100) < clusterPercent) 
		{
		    /* left child is to receive a clustered object */
		    rnd = clust_lower_bound +(random() % clusterSpread);
	      	    rec.left = a_roots[rnd];
	    	}
	    	else 
		{
	      	    rnd = random() % NB_TREE;
	      	    rec.left = a_roots[rnd];
	    	}

		/* now go through the same process for the right child */

	        if ((random() % 100) < clusterPercent) 
		{
		    /* left child is to receive a clustered object */
		    rnd = clust_lower_bound +(random() % clusterSpread);
	      	    rec.right = a_roots[rnd];
	    	}
	    	else 
		{
		    /* right child is to receive an unclustered object */
	      	    rnd = random() % NB_TREE;
	      	    rec.right = a_roots[rnd];
	    	}

	        /* rewrite the record on disk */
	        e = wiss_writerecord(ofn, &a_leaves[j], &rec, sizeof(rec), 
			transId, TRUE, 
		FALSE);
	        WISSERROR("build/wiss_writerecord", e);
	    }
	} /* end for (i = 0; i < NB_TREE; i++) */

	/* END OF PASS #2 */
	/* PASS #3 */

	if ((mode == 1) || (mode == 3))
	{
	    /* PHASE #1 of PASS #3 */
	    /* smear some leave nodes */
	
	    /* squeeze a_roots[] to 50% of the roots */
	    while (i_roots > (NB_TREE/4)*2) {
	      rnd = random() % ((NB_TREE/4)*2);
	      a_roots[rnd] = a_roots[--i_roots];
	    }
	    /* switch 50% of chosen-root children */
#ifdef TRACE
	    printf("\nswitch half of %d root children:", i_roots);
	    printf("(left<-->left and/or rigth<-->right)\n");
#endif
	    for (i = 0; i < i_roots; i += 2) {
	      e = wiss_readrecord(ofn, &a_roots[i], &rec, sizeof(rec), 
		transId, TRUE, l_X, FALSE);
	      WISSERROR("build/wiss_readrecord  (2)", e);
	      e = wiss_readrecord(ofn, &a_roots[i+1], &rec1, sizeof(rec1), 
		transId, TRUE, l_X, FALSE);
	      WISSERROR("build/wiss_readrecord (3)", e);
#ifdef TRACE
	      printf("\n %d-%d: ", i, i+1);
	      DUMPRID(a_roots[i]);
	      printf("  <--> ");
	      DUMPRID(a_roots[i+1]);
	      printf("  >>>> ");
#endif
	      if (FIFTY_FIFTY) {
#ifdef TRACE
	        printf(" left ");
#endif
	        rid = rec.left;
	        rec.left = rec1.left;
	        rec1.left = rid;
	        }
	      if (FIFTY_FIFTY) {
#ifdef TRACE
	        printf(" right ");
#endif
	        rid = rec.right;
	        rec.right = rec1.right;
	        rec1.right = rid;
	        }
	      e = wiss_writerecord(ofn, &a_roots[i], &rec, sizeof(rec),
		transId, TRUE, FALSE);
	      WISSERROR("build/wiss_writerecord (2)", e);
	      e = wiss_writerecord(ofn, &a_roots[i+1], &rec1, sizeof(rec1),
		transId, TRUE, FALSE);
	      WISSERROR("build/wiss_writerecord (3)", e);
	    }
	}
	if ((mode == 2) || (mode == 3))
	{
	    /* PHASE #2 of PASS #3 */
	    /* smear some children nodes */

	    /* squeeze a_ints[] to 50% of the interior nodes */
	    while (i_ints > (NB_INT_NODES/4)*2) {
	      rnd = random() % ((NB_INT_NODES/4)*2);
	      a_ints[rnd] = a_ints[--i_ints];
	    }

	    /* switch 50% of the chosen interior-nodes children */
#ifdef TRACE
	    printf("\n\n switch half of %d interior-node children: \n", i_ints);
#endif
	    for (i = 0; i < i_ints; i += 2) {
	      e = wiss_readrecord(ofn, &a_ints[i], &rec, sizeof(rec), transId, 
		TRUE, l_X, FALSE);
	      WISSERROR("build/wiss_readrecord (3)", e);
	      e = wiss_readrecord(ofn, &a_ints[i+1], &rec1, sizeof(rec1), 
		transId, TRUE, l_X, FALSE);
	      WISSERROR("build/wiss_readrecord (4)", e);
#ifdef TRACE
	      printf("\n %d-%d: ", i, i+1);
	      DUMPRID(a_ints[i]);
	      printf("  <--> ");
	      DUMPRID(a_ints[i+1]);
	      printf("  >>>> ");
#endif
	      if (FIFTY_FIFTY) {
#ifdef TRACE
	        printf(" left ");
#endif
	        rid = rec.left;
	        rec.left = rec1.left;
	        rec1.left = rid;
	        }
	      if (FIFTY_FIFTY) {
#ifdef TRACE
	        printf(" right ");
#endif
	        rid = rec.right;
	        rec.right = rec1.right;
	        rec1.right = rid;
	        }
	      e = wiss_writerecord(ofn, &a_ints[i], &rec, sizeof(rec), 
		transId, TRUE, FALSE);
	      WISSERROR("build/wiss_writerecord (4)", e);
	      e = wiss_writerecord(ofn, &a_ints[i+1], &rec1, sizeof(rec1), 
		transId, TRUE, FALSE);
	      WISSERROR("build/wiss_writerecord (5)", e);
	   }
	}

	/* close the object file */
	e = wiss_closefile(ofn);
	WISSERROR("test1/wiss_closefile", e);

	e = wiss_closefile(ofnrids);
	WISSERROR("test1/wiss_closefile", e);

	/* now commit the transaction */
	e = commit_trans(transId);
	if (e != 1) 
	  printf("error status return from commit_trans = %d\n", e);
	else printf("commit ok\n");

	/* dismount the device */
#ifdef TRACE
	printf("\n\nwiss_dismount ...\n");
#endif
	e = wiss_dismount(deviceName);  
	WISSERROR("build/wiss_dismount", e);

#ifdef TRACE
	printf("wiss_final ...\n");
#endif
	(void) wiss_final();

	endTime = time(NULL);
	if (verboseFlag)
	{
	    printf("Time to build the database = %d seconds\n",
		endTime - startTime);
	}


#ifdef TRACE
	printf("bye \n");
#endif

} /* end main */


/* Create a binary tree of Wiss records. A record is of type 't_rec'. The */
/* value 'key' is incremented of the created record number. 		  */

RID create_tree(ofn, key, depth, page)
     int ofn;	/* ofn of the file where to build the tree */
     int *key;  /* single record key */
     int depth; /* depth of the tre eto build */
     PID *page; /* current page to check if the whole tree fits one 1 page */
{
  int e;	/* returned error code */
  t_rec rec;	/* record to fill and to store on the disk */
  RID new_rid;	/* physical record id */
  PID new_pid;	/* to convert a rid into a pid */
  RID nil_rid;	/* usefull null physical record */

  RIDCLEAR(nil_rid);

  /* test to stop recursive calls */
  if (depth < 0) return(nil_rid);

  rec.num = ++(*key);

  /* create left and right children */
  rec.left = create_tree(ofn, key, depth -1, page);
  rec.right = create_tree(ofn, key, depth -1, page);

  bzero(rec.dummy, sizeof(rec.dummy));

  /* create the current level record on disk */
  e = wiss_appendfile(ofn, &rec, sizeof(rec), &new_rid, transId, TRUE, FALSE);
  WISSERROR("build/wiss_appendfile", e);
/*
  printf("\n");
  DUMPRID(new_rid);
  printf("\n");
*/

  /* add the roots in a_roots[], interior nodes in a_ints, */
  /* and all the leaves in a_leaves[] 			   */
  switch(depth) {
  case TREE_DEPTH: 
    a_roots[i_roots++] = new_rid;
    break;
  case 0:
    a_leaves[i_leaves ++] = new_rid;
    break;
  case 1:
      a_ints[i_ints++] = new_rid;
  }

  /* initialize 'page' for this tree or check if the record is created on the */
  /* current page (intra-cluster is respected) if 'page is already initialized */
  GETPID(new_pid, new_rid);
  if (TESTPIDCLEAR(*page))
    *page = new_pid;
  else
    if (!(PIDEQ(*page, new_pid))) {
      printf("a tree does not fit completely on the current page (");
      DUMPRID(new_rid);
      printf(")\n");
      exit(1); 
    }

  return(new_rid);
}

fatalerror(p, e)
char *p; int e;
{
   int ex;

   printf("Fatal wiss error,  abort the transaction\n");
   wiss_abort_trans(transId);

   /* dismount the device */
   (void) wiss_dismount(deviceName);  

   wiss_final();  /* clean up processes and shared memory segments */

   wiss_fatalerror(p,(int) e);
}
