
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


#include        <wiss.h>
#include        <page.h>
#include        <sm.h>
#include        <lockquiz.h>

extern struct graph_bucket *find_trans();
extern struct node *findfile();
extern struct node *findpage();

extern int lmPageLocks;

extern SMDESC   *smPtr; /* ptr to common data structures in shared memory */
extern  int     procNum; /* index of the process in the smPtr->users table */



/*** print a resource node for trans_state() ***/
print_trans_node(resptr)
     struct node *resptr;
{
  struct wait_node *wptr, *optr;	/* points to wait_list and o_list */

  /* print the resource id */
  if (resptr->owner==FILER) 
    printf("   . File %d", resptr->owner_id.file_id.Ffilenum);
  else 
    printf("\n   . Page %d", resptr->owner_id.page_id.Ppage);

  /* print the main owner transaction */
  printf(" (lock %s) \n", LM_locknames[resptr->serving]);

  /* cover the o_list */
  printf("           . lock %s granted to trans %d \n", 
	 LM_locknames[resptr->serving], resptr->trans_id);
  for (optr = resptr->o_list; optr != NULL; optr = optr->flink)
    printf("           . lock %s granted to trans %d \n", 
	   LM_locknames[optr->serving], optr->trans_id);

  /* cover the wait_list */
  for (wptr = resptr->wait; wptr != NULL; wptr = wptr->flink)
    printf("           . lock %s waited by trans %d - duration %d \n", 
	   LM_locknames[wptr->serving], wptr->trans_id, wptr->duration);
  
} /* end of print_trans_node */


/*** given a transaction number, it displays all ***/
/*** the pages and files owned or waited ***/
trans_state(trans_id)
     int trans_id;
{
  struct graph_bucket *finger;	/* points to a transaction in trans[] */
  struct lockq *optr;		/* points to a pointer on a held resource */

  /* look for the trans_id, starting at trans[] */
  finger = find_trans(trans_id);
  if (finger == NULL) {
    printf("\n--> No transaction %d\n\n", trans_id);
    return;
  }

  /* print the resources held by trans_id */
  if (finger->locks_held == NULL)
    printf("\n--> No lock owned by trans. %d\n", trans_id);
  else {
    printf("\n--> Trans %d is owning:\n", trans_id);
    for (optr = finger->locks_held; optr != NULL; optr = optr->flink) 
      print_trans_node(optr->resptr);
  }

  /* print the resource waited by trans_id */
  if (finger->res_wait == NULL)
    printf(" No lock waited by trans. %d\n", trans_id);
  else {
    if (finger->res_wait != NULL) {
      printf(" And is waiting for:\n");
      print_trans_node(finger->res_wait);
    }
  }

  printf("\n");

} /* end of trans_state() */


/*** given a file id, it displays all the transactions ***/
/*** which are waiting for or are owning the file ***/
file_state(file_id)
     FID file_id;
{
  struct node *resptr;		      /* points to a file */
  struct wait_node *wptr, *optr;      /* points to wait_list and o_list */
  struct node *pptr;		      /* points to the locked page of the file */
  
  /* look for file_id */
  resptr = findfile(file_id); 
  if (resptr == NULL) {
    printf("\n--> No lock on file %d\n", file_id.Ffilenum);
    return;
  }

  /* print informations about the first owner */
  printf("\n--> File %d is owned by:\n", file_id.Ffilenum);
  printf("   . transaction %d (lock %s) \n", resptr->trans_id, 
	 LM_locknames[resptr->serving]);

  /* cover the o_list */
  for (optr = resptr->o_list; optr != NULL; optr = optr->flink)
    printf("   . transaction %d (lock %s) \n", optr->trans_id, 
	   LM_locknames[optr->serving]);

  /* cover the wait_list */
  if (resptr->wait != NULL) {
    printf(" And is waited by:\n");
    for (wptr = resptr->wait; wptr != NULL; wptr = wptr->flink)
      printf("   . transaction %d (lock %s - duration %d) \n", 
	     wptr->trans_id, LM_locknames[wptr->serving], wptr->duration);
  }
  
  /* cover the f_page list */
  if (resptr->f_page != NULL) {
    printf(" And has the page(s):\n");
    for (pptr = resptr->f_page; pptr != NULL; pptr = pptr->f_page)
      printf("   . page %d (lock %s) \n", pptr->owner_id.page_id.Ppage,
	     LM_locknames[pptr->serving]);
  }

  printf("\n");

} /* end of file_state() */

/*** given a page id, it displays all the transactions ***/
/*** which are waiting for or are owning the page ***/
page_state(page_id)
     PID page_id;
{
  struct node *resptr;			/* points to a page */
  struct wait_node *wptr, *optr;	/* points to wait_list and o_list */
  
  /* look for page_id */
  resptr = findpage(page_id); 
  if (resptr == NULL) {
    printf("\n--> No lock on page %d\n", page_id.Ppage);
    return;
  }

  /* print informations about the first owner */
  printf("\n--> Page %d is owned by:\n", page_id.Ppage);
  printf("   . transaction %d (lock %s) \n", resptr->trans_id, 
	 LM_locknames[resptr->serving]);

  /* cover the o_list */
  for (optr = resptr->o_list; optr != NULL; optr = optr->flink)
    printf("   . transaction %d (lock %s) \n", optr->trans_id, 
	   LM_locknames[optr->serving]);

  /* cover the wait_list */
  if (resptr->wait != NULL) {
    printf(" And is waited by:\n");
    for (wptr = resptr->wait; wptr != NULL; wptr = wptr->flink)
      printf("   . transaction %d (lock %s - duration %d) \n", 
	     wptr->trans_id, LM_locknames[wptr->serving], wptr->duration);
  }

  printf("\n");
  
} /* end of page_state() */


/* display current contents of lock table */

lm_lock_dump()
{
    int i;
    printf("number of lockpage calls = %d\n",lmPageLocks);
    for (i=0;i<MAXRES; i++) 
	if (smPtr->locktable[i].lockptr != NULL)
	{
		printf("locktable %d not empty\n",i);
	}
}

