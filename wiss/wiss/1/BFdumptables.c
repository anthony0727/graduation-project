
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


# 
/* Module BF_dumpbuftable:
 	This routine prints the state of the buffer pool.
	and locktable 
   
   Imports :
	buftable[]

   Exports :	
	BF_dumpbuftable()
	BF_dumpfixed()

   Returns :
	None

   Side Effects:
	None

   Errors:
	None
*/

#include <bf.h>

BF_dumpbuftable()
{
	register	i, j;

printf("\n buf |  pageid | file# | refbit | dirty | Bfixcnt | Bvalid ");
printf("\n -----------------------------------------------------------------\n");
	for(i = 0; i < smPtr->bf_num_bufs; i++) 
	{
		smPtr->buftable[i];
		printf(" %3d | %3d:%3d | %5d |", i,
			smPtr->buftable[i].Bpageid.Pvolid, 
			smPtr->buftable[i].Bpageid.Ppage, 
		  	smPtr->buftable[i].Bfilenum);
		printf("   %3.3s  |  %3.3s  |   %2.2d   | %3.3s |\n",
			smPtr->buftable[i].Brefbit?" ON":"OFF", 
			smPtr->buftable[i].Bdirty?"YES":"NO", 
			smPtr->buftable[i].Bfixcnt,
			smPtr->buftable[i].Bvalid?"YES":"NO");
	} 
	
	printf("[%d buffers in total, %d available]\n\n",
		smPtr->bf_num_bufs, smPtr->bf_num_free);
		
} /* BF_dumpbuftable */

BF_dumpbufpool()
{
	register int	i;
	DATAPAGE	*bufptr;


	printf("\nBUFFERPOOL PAGES");
	printf("\n buf |  thispage | nextpage|");
	printf("\n----------------------------");
	for (i = 0; i < smPtr->bf_num_bufs; i++){
		bufptr = (DATAPAGE *)&smPtr->bufferpool[i];
		printf ("\n %d %d %d ",i,bufptr->thispage.Ppage,bufptr->nextpage);
		printf("\n----------------------------");
	}
}
BF_dumpfixed()
/* This routine dumps the buffers that are fixed '
*/
{
	register	i, j;

	for(j = i = 0; i < smPtr->bf_num_bufs; i++) 
		if (smPtr->buftable[i].Bvalid && smPtr->buftable[i].Bfixcnt > 0) j++;
	if (j == 0) {
		return(0);
	}
	printf("\n A total of %d buffers fixed", j);
	printf("\n buf |  pageid | file# | refbit | dirty | fixcnt |");
	printf("\n -------------------------------------------------\n");
	for(i = 0; i < smPtr->bf_num_bufs; i++) 
	{
		if (!smPtr->buftable[i].Bvalid || 
			smPtr->buftable[i].Bfixcnt <= 0) continue;
		printf(" %3d | %3d:%3d | %5d |", i,
		  smPtr->buftable[i].Bpageid.Pvolid, 
		  smPtr->buftable[i].Bpageid.Ppage, 
		  smPtr->buftable[i].Bfilenum); 
		printf("   %3.3s  |  %3.3s  |   %2.2d   |\n",
			smPtr->buftable[i].Brefbit?" ON":"OFF", 
			smPtr->buftable[i].Bdirty?"YES":"NO", 
			smPtr->buftable[i].Bfixcnt);
	} 
	return(j);
	
} /* BF_dumpfixed */

/* initializes event table */

#ifdef EVENTS
BF_initevent()
{
	smPtr->eventhead = -1;
}

BF_event(procNum,action,pageid,entry)
int 	procNum;  /* process number */
char 	*action;/* action about to be performed */
PID	*pageid;
int	entry;
{
	struct	evententry	*e;
	int	j;

	if (++smPtr->eventhead == MAXEVENTS) smPtr->eventhead=0;
	j = smPtr->eventhead;
	e = &smPtr->event_table[j]; 
	e->process = procNum;
	strcpy(e->event,action); 
	e->pageid = *pageid;
	e->entry = entry;  
}

BF_dumpevent ()
{
	int i,j;
	struct	evententry	*e;

	printf("Process\tEvent\tPageID\tEntry\n\n");
	i = smPtr->eventhead + 1;
	for (j = 0; j < MAXEVENTS; j++, i++)
	{
		if (i == MAXEVENTS) i = 0;
		e = &smPtr->event_table[i];
		printf("%d\t%s\t",e->process,e->event);
		PRINTPID(e->pageid);
		printf("\t%d\n",e->entry);
	}
}
#endif
