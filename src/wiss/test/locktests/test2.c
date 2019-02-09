
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
#include <lockquiz.h>
#include <locktype.h>

#define NUMPIDS 10

main( argc, argv )
int argc;
char **argv;
{
	int i;
	PID pidarray[NUMPIDS], *pidPtr;
	FID fid;
	int volid;
	int transId;
	int status;
	int	cnt;

	wiss_init();

	/* fill out pidarray with valid pids */
	pidPtr = &pidarray[0];
	volid = 533;
	for (i=0; i<NUMPIDS; i++, pidPtr++)
	{
		pidPtr->Pvolid = volid;
		pidPtr->Ppage = i+1;
	}
	fid.Fvolid = 533;
	fid.Ffilenum = 0;

	printf("test1. start by locking %d pages\n", NUMPIDS);
	transId = begin_trans();
	printf("id of new transaction=%d\n",transId);
	status = lock_file(transId, fid,l_S, MANUAL, FALSE);
	printf("lock_file returned=%d\n",status);
	for (i=0;i<NUMPIDS;i++)
	{
	    status = lock_page(transId, fid, pidarray[i],l_S, 
		MANUAL, FALSE, FALSE); /* lock page i */
	    if (status != GRANTED) 
	  	printf("error status return from lock_page=%d\n", status);
	}

	printf("next unlock the even pages explicitly\n");
	for (i=0;i<NUMPIDS;i=i+2)
	{
	    status = m_release_page(transId, pidarray[i]); /* unlock page i */
	    if (status != OK) 
	  	printf("error status return from release_page=%d\n", status);
	}
	printf("next commit the transaction (w/o unlock odd pages)\n");
	/* now commit the transaction */
	status = commit_trans(transId);
	if (status != OK) 
	  printf("error status return from commit_trans = %d\n", status);
	else printf("commit ok\n");

	transId = begin_trans();
	printf("id of new transaction=%d\n",transId);
	status = lock_file(transId, fid,l_S, MANUAL, FALSE);
	printf("lock_file returned=%d\n",status);
	for (i=0;i<NUMPIDS;i++)
	{
	    status = lock_page(transId, fid, pidarray[i],l_S, MANUAL, FALSE,
		FALSE); /* lock page i */
	    if (status != GRANTED) 
	  	printf("error status return from lock_page=%d\n", status);
	}

	printf("next unlock the even pages explicitly\n");
	for (i=0;i<NUMPIDS;i=i+2)
	{
	    status = m_release_page(transId, pidarray[i]); /* unlock page i */
	    if (status != OK) 
	  	printf("error status return from release_page=%d\n", status);
	}
	printf("next commit the transaction (w/o unlock odd pages)\n");
	/* now commit the transaction */
	status = commit_trans(transId);
	if (status != OK) 
	  printf("error status return from commit_trans = %d\n", status);
	else printf("commit ok\n");
	printf("\n\n\n");

	wiss_final();

}

