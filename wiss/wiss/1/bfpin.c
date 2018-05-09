
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
/* Module bf_pin:
	This routine pins a page known to be in the buffer pool

   Imports :
	bf_num_free, buftable[], bufferpool[]

   Exports :	
	bf_pin(pagebuf)

   Returns :
	None
*/

#include <bf.h>


bf_pin(pid, pagebuf)
PID	*pid;		/* pid of page to be pinned */
PAGE	*pagebuf;	/* address the buffer to be pinned */
{
	register int i;
	PID	spageid;

	/* check the input parameters */
	if (pid == NULL) return(e1NULLPIDPARM);


#ifdef TRACE
	if (checkset (&Trace1, tINTERFACE))
		printf("bf_pin(pagebuf=0x%x)\n", pagebuf);
#endif

	spageid = *pid;
	BF_lock(&spageid);
	i = pagebuf - smPtr->bufferpool;
	if (i >= 0 && i < smPtr->bf_num_bufs) 
	{
	    /* one more user of this buffer */
	    if (smPtr->buftable[i].Bfixcnt <=0) 
	    {
	        smPtr->buftable[i].Bfixcnt = 1;
	        smPtr->bf_num_free--;
	    }
	    else smPtr->buftable[i].Bfixcnt++;
	}
	else 
        { 
	   	BF_unlock(&spageid);
		printf ("Couldn't find the page in the buffer pool.\n");
		printf("bf_pin(pagebuf=0x%x)\n", pagebuf);
		printf ("buffer index is %d.\n",i);
/*
		BF_dumpbuftable();
		BF_dumpbufpool();
		BF_dumpevent ();
*/
           	return(e1WRONGBUFFER);
        }
	BF_unlock(&spageid);
	return(eNOERROR);
}
