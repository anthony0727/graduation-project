
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

#define NUMPIDS 8

main( argc, argv )
int argc;
char **argv;
{
	int i,j;
	PID pidarray[NUMPIDS], *pidPtr;
	FID fid;
	FID nullFid;
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

	transId = begin_trans();
	printf("id of new transaction=%d\n",transId);

	printf("start by locking the file in IS mode\n");
        status = lock_file(transId, fid,l_IS, MANUAL, FALSE);

	printf("test1. next by locking %d pages in l_S mode \n", NUMPIDS);
	for(j=0;j<10000;j++)
	{
	    i = random()%NUMPIDS;
/*
	    printf("lock page %d.%d\n",pidarray[i].Pvolid, pidarray[i].Ppage);
*/
	    status = lock_page(transId, fid, pidarray[i],l_S, 
		MANUAL, FALSE, FALSE); /* lock page i */
	    if (status != GRANTED) 
	  	printf("error status return from lock_page=%d\n", status);
/*
	    printf("next unlock the page explicitly\n");
*/
	    status = m_release_page(transId, pidarray[i]); /* unlock page i */
	    if (status != OK) 
	  	printf("error status return from release_page=%d\n", status);
	}
	printf("next commit the transaction \n");
	/* now commit the transaction */
	status = commit_trans(transId);
	if (status != OK) 
	  printf("error status return from commit_trans = %d\n", status);
	else printf("commit ok\n");

	wiss_final();

}

