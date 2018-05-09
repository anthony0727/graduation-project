
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

#define NUMPIDS 1000
#define NUMFIDS 1000

main( argc, argv )
int argc;
char **argv;
{
	int i;
	PID pidarray[NUMPIDS], *pidPtr;
	FID fidarray[NUMFIDS], *fidPtr;
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

	/* fill out fidarray with valid fids */
	fidPtr = &fidarray[0];
	volid = 533;
	for (i=0; i<NUMFIDS; i++, fidPtr++)
	{
		fidPtr->Fvolid = volid;
		fidPtr->Ffilenum = i;
	}

	printf("test1. begin & commit a transaction without setting any locks\n");
	transId = begin_trans();
	printf("id of new transaction=%d\n",transId);
	status = commit_trans(transId);
	if (status != OK) 
	  printf("error status return from commit_trans = %d\n", status);
	else printf("commit ok\n");

	printf("test2. lock a single page\n");

	transId = begin_trans();
	printf("id of new transaction=%d\n",transId);
	status = lock_file(transId, fidarray[0],l_IS, MANUAL, FALSE);
	printf("lock_file returned=%d\n",status);
	status = lock_page(transId, fidarray[0], pidarray[0],l_S, 
		MANUAL, FALSE, FALSE);
	printf("lock_page returned=%d\n",status);
	status = m_release_page(transId, pidarray[0]);
	printf("m_release_page returned=%d\n",status);
	/* now commit the transaction */
	status = commit_trans(transId);
	if (status != OK) 
	  printf("error status return from commit_trans = %d\n", status);
	else printf("commit ok\n");

	printf("test3. lock 2 pages. Then unlock them in opposite order\n");

	transId = begin_trans();
	printf("id of new transaction=%d\n",transId);
	printf("test. lock page %d.%d\n", pidarray[0].Pvolid, 
	pidarray[0].Ppage);
	status = lock_file(transId, fidarray[0],l_IS, MANUAL, FALSE);
	printf("lock_file returned=%d\n",status);
	status = lock_page(transId,fidarray[0],  pidarray[0],l_S, 
		MANUAL, FALSE, FALSE); /* lock page 0 */
	printf("lock_page of page 0 returned=%d\n",status);
	printf("test. lock page %d.%d\n", pidarray[1].Pvolid, 
	pidarray[1].Ppage);
	status = lock_page(transId,fidarray[0], pidarray[1], l_S, MANUAL, 
		FALSE, FALSE); /* lock page 1 */
	printf("lock_page of page 1 returned=%d\n",status);
	status = m_release_page(transId, pidarray[1]); /* unlock 1 */
	printf("m_release_page returned=%d\n",status);
	printf("m_release_page returned=%d\n",status); /* unlock 0 */
	/* now commit the transaction */
	status = commit_trans(transId);
	if (status != OK) 
	  printf("error status return from commit_trans = %d\n", status);
	else printf("commit ok\n");


	printf("test4. lock 2 pages. commit without any releases\n");

	transId = begin_trans();
	printf("id of new transaction=%d\n",transId);
	status = lock_file(transId, fidarray[1],l_IS, MANUAL, FALSE);
	printf("lock_file returned=%d\n",status);
	status = lock_page(transId, fidarray[1], pidarray[0],l_S, MANUAL, 
		FALSE, FALSE); /* lock page 0 */
	printf("lock_page of page 0 returned=%d\n",status);
	status = lock_page(transId, fidarray[1], pidarray[1],l_S, MANUAL, 
		FALSE, FALSE); /* lock page 1 */
	printf("lock_page of page 1 returned=%d\n",status);
	/* now commit the transaction */
	status = commit_trans(transId);
	if (status != OK) 
	  printf("error status return from commit_trans = %d\n", status);
	else printf("commit ok\n");


	printf("test5. start by locking 100 pages\n");
	transId = begin_trans();
	status = lock_file(transId, fidarray[1],l_IS, MANUAL, FALSE);
	printf("lock_file returned=%d\n",status);
	printf("id of new transaction=%d\n",transId);
	for (i=0;i<100;i++)
	{
	    status = lock_page(transId,fidarray[1],  pidarray[i],l_S, MANUAL, 
		FALSE, FALSE); /* lock page i */
	    if (status != GRANTED) 
	  	printf("error status return from lock_page=%d\n", status);
	}

	printf("next unlock the even pages explicitly\n");
	for (i=0;i<100;i=i+2)
	{
	    status = m_release_page(transId, pidarray[i]); /* unlock page i */
	    if (status != OK) 
	  	printf("error status return from release_page=%d\n", status);
	}
	printf("next unlock the odd pages explicitly\n");
	for (i=1;i<100;i=i+2)
	{
	    status = m_release_page(transId, pidarray[i]); /* unlock page i */
	    if (status != OK) 
	  	printf("error status return from release_page=%d\n", status);
	}
	/* now commit the transaction */
	status = commit_trans(transId);
	if (status != OK) 
	  printf("error status return from commit_trans = %d\n", status);
	else printf("commit ok\n");


	printf("test6. start by locking 1000 pages\n");
	transId = begin_trans();
	printf("id of new transaction=%d\n",transId);
	status = lock_file(transId, fidarray[1],l_IS, MANUAL, FALSE);
	printf("lock_file returned=%d\n",status);
	for (i=0;i<1000;i++)
	{
	    status = lock_page(transId,fidarray[1],  pidarray[i],l_S, MANUAL, 
		FALSE, FALSE); /* lock page i */
	    if (status != GRANTED) 
	  	printf("error status return from lock_page=%d\n", status);
	}

	printf("next unlock the even pages explicitly\n");
	for (i=0;i<1000;i=i+2)
	{
	    status = m_release_page(transId, pidarray[i]); /* unlock page i */
	    if (status != OK) 
	  	printf("error status return from release_page=%d\n", status);
	}

	printf("next relock the even pages - hits\n");
	for (i=0;i<1000;i=i+2)
	{
	    status = lock_page(transId,fidarray[1],  pidarray[i],l_S, 
		MANUAL, FALSE, FALSE); /* lock page i */
	    if (status != GRANTED) 
	  	printf("error status return from lock_page=%d\n", status);
	}

	printf("next unlock the odd pages explicitly\n");
	for (i=1;i<1000;i=i+2)
	{
	    status = m_release_page(transId, pidarray[i]); /* unlock page i */
	    if (status != OK) 
	  	printf("error status return from release_page=%d\n", status);
	}
	/* now commit the transaction */
	printf("finally commit the transaction - implicitly releasing\n");
	printf("the even locks\n");
	status = commit_trans(transId);
	if (status != OK) 
	  printf("error status return from commit_trans = %d\n", status);
	else printf("commit ok\n");


	printf("test6. lock a single file\n");

	transId = begin_trans();
	printf("id of new transaction=%d\n",transId);
	status = lock_file(transId, fidarray[0],l_IS, MANUAL, FALSE);
	printf("lock_file returned=%d\n",status);
	status = m_release_file(transId, fidarray[0]);
	printf("m_release_file returned=%d\n",status);
	/* now commit the transaction */
	status = commit_trans(transId);
	if (status != OK) 
	  printf("error status return from commit_trans = %d\n", status);
	else printf("commit ok\n");

	printf("test8. lock 2 files. Then unlock them in opposite order\n");

	transId = begin_trans();
	printf("id of new transaction=%d\n",transId);
	status = lock_file(transId, fidarray[0],l_IS, MANUAL, FALSE); /* lock file 0 */
	printf("lock_file of file 0 returned=%d\n",status);
	status = lock_file(transId, fidarray[1],l_IX, MANUAL, FALSE); /* lock file 1 */
	printf("lock_file of file 1 returned=%d\n",status);
	status = m_release_file(transId, fidarray[1]); /* unlock 1 */
	printf("m_release_file returned=%d\n",status);
	printf("m_release_file returned=%d\n",status); /* unlock 0 */
	/* now commit the transaction */
	status = commit_trans(transId);
	if (status != OK) 
	  printf("error status return from commit_trans = %d\n", status);
	else printf("commit ok\n");


	printf("test9. lock 2 file. commit without any releases\n");

	transId = begin_trans();
	printf("id of new transaction=%d\n",transId);
	status = lock_file(transId, fidarray[0],l_SIX, MANUAL, FALSE); /* lock file 0 */
	printf("lock_file of file 0 returned=%d\n",status);
	status = lock_file(transId, fidarray[1],l_S, MANUAL, FALSE); /* lock file 1 */
	printf("lock_file of file 1 returned=%d\n",status);
	/* now commit the transaction */
	status = commit_trans(transId);
	if (status != OK) 
	  printf("error status return from commit_trans = %d\n", status);
	else printf("commit ok\n");


	printf("test10. start by locking 100 files\n");
	transId = begin_trans();
	printf("id of new transaction=%d\n",transId);
	for (i=0;i<100;i++)
	{
	    status = lock_file(transId, fidarray[i],l_S, MANUAL, FALSE); /* lock file i */
	    if (status != GRANTED) 
	  	printf("error status return from lock_file=%d\n", status);
	}
	printf("next unlock the even files explicitly\n");
	for (i=0;i<100;i=i+2)
	{
	    status = m_release_file(transId, fidarray[i]); /* unlock file i */
	    if (status != OK) 
	  	printf("error status return from release_file=%d\n", status);
	}
	printf("next unlock the odd files explicitly\n");
	for (i=1;i<100;i=i+2)
	{
	    status = m_release_file(transId, fidarray[i]); /* unlock files i */
	    if (status != OK) 
	  	printf("error status return from release_file=%d\n", status);
	}
	/* now commit the transaction */
	status = commit_trans(transId);
	if (status != OK) 
	  printf("error status return from commit_trans = %d\n", status);
	else printf("commit ok\n");


	printf("test11. start by locking 1000 files\n");
	transId = begin_trans();
	printf("id of new transaction=%d\n",transId);
	for (i=0;i<1000;i++)
	{
	    status = lock_file(transId, fidarray[i],l_S, MANUAL, FALSE); /* lock files i */
	    if (status != GRANTED) 
	  	printf("error status return from lock_file=%d\n", status);
	}
	printf("next unlock the even files explicitly\n");
	for (i=0;i<1000;i=i+2)
	{
	    status = m_release_file(transId, fidarray[i]); /* unlock file i */
	    if (status != OK) 
	  	printf("error status return from release_file=%d\n", status);
	}

	printf("next relock the even files  \n");
	for (i=0;i<1000;i=i+2)
	{
	    status = lock_file(transId, fidarray[i],l_S, MANUAL, FALSE); /* lock files i */
	    if (status != GRANTED) 
	  	printf("error status return from lock_file=%d\n", status);
	}

	printf("next unlock the odd files explicitly\n");
	for (i=1;i<1000;i=i+2)
	{
	    status = m_release_file(transId, fidarray[i]); /* unlock file i */
	    if (status != OK) 
	  	printf("error status return from release_file=%d\n", status);
	}
	/* now commit the transaction */
	printf("finally commit the transaction - implicitly releasing\n");
	printf("the even locks\n");
	status = commit_trans(transId);
	if (status != OK) 
	  printf("error status return from commit_trans = %d\n", status);
	else printf("commit ok\n");

	wiss_final();
}

