
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
#include 	<stdio.h>

extern SMDESC   *smPtr; /* ptr to common data structures in shared memory */
extern  int     procNum; /* index of the process in the smPtr->users table */


#define LMMAXEVENTS    100
  
  struct lmentry
{
  char pname[10];
  char event[10];
  int  prio;
};

struct lmentry lm_table[LMMAXEVENTS];
int     lmhead;

LM_initevent()
{
  lmhead = -1;
}

LM_event(name, action, prio)
     char *name;
     char *action;
     int  prio;
{
  struct lmentry *e;
  if (++lmhead == LMMAXEVENTS) lmhead = 0;
  e = &lm_table[lmhead];
  strcpy(e->pname,name);
  strcpy(e->event,action);
  e->prio = prio;
}

LM_dumpevent()
{
  int i, j;
  struct lmentry *e;
  printf ("Process\tEvent\tprio\n\n");
  i = lmhead + 1;
  for (j=0; j < LMMAXEVENTS; j++, i++)
    {
      if (i == LMMAXEVENTS) i = 0;
      e = &lm_table[i];
      printf("%s\t%s\t%d\n",e->pname,e->event,e->prio);
    }
}
