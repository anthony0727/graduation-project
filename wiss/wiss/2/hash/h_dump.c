
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



/* Module : h_dumpfile - print the contents of a hash file

   IMPORTS :
	bf_readbuf(trans_id, filenum, fid, pid, returnpageptr)
	bf_freebuf(filenum, pid, pageptr)
	bt_print_btpage(dp)

   EXPORTS:
	h_dumpfile(filenum)
	h_dumphashtab(filenum)
*/

#include	<wiss.h>
#include	<st.h>

h_dumphashtab(filenum)
int	filenum;	/* which hash file */

/* This routine print the hash table of a hash index

   Returns:
	None

   Side Effects:
	None

   Errors:
	None
*/
{
	int		e;	/* for returned errors */
	int		i;
	PID		hpid;	/* page id of the hash table */
	ROOTPAGE	*ht;	/* the hash table */

	/* check file # */
	CHECKOFN(filenum);

	/* get the hash table */
	hpid = F_ROOTPID(filenum);
	e = bf_readbuf(-1, filenum, FC_FILEID(filenum), &hpid, (PAGE **)&ht);
	CHECKERROR(e);

	printf("Hash index: Global Depth %d, PID of the hash table %d:%d\n", 
		GLOBALDEPTH(filenum), hpid.Pvolid, hpid.Ppage);
	for (i = 0;  i < 1 << GLOBALDEPTH(filenum); i++) 
		printf(" bucket[%2.2d]=%4.4d%c", i, ht->bucket[i], 
			(i+1)%4 ? ',' : '\n');
	(void) bf_freebuf(filenum, &hpid, (PAGE *)ht);

} /* h_dumphashtab */

h_dumpfile(filenum)
int	filenum;	/* which hash file to search */

/* This routine print the contents of a hash file

   Returns:
	None

   Side Effects:
	None

   Errors:
	None
*/
{
	int		e;	/* for returned errors */
	int		i;
	PID		pid;	/* page id templete */
	PID		hpid;	/* page id of the hash table */
	ROOTPAGE	*ht;	/* the hash table */
	BTREEPAGE	*dp;	/* leaf page pointer*/

	/* check file # */
	CHECKOFN(filenum);

	/* get the hash table */
	hpid = F_ROOTPID(filenum);
	e = bf_readbuf(-1, filenum, FC_FILEID(filenum), &hpid, (PAGE **)&ht);
	CHECKERROR(e);

	printf("Hash index: Global Depth %d, PID of the hash table %d:%d\n", 
		GLOBALDEPTH(filenum), hpid.Pvolid, hpid.Ppage);
	for (i = 0;  i < 1 << GLOBALDEPTH(filenum); i++) 
		printf(" bucket[%2.2d]=%4.4d%c", i, ht->bucket[i], 
			(i+1)%4 ? ',' : '\n');

	for (i = 0, pid= hpid;  i < 1 << GLOBALDEPTH(filenum); i++) {
		for (pid.Ppage = ht->bucket[i]; pid.Ppage != NULLPAGE;
				pid.Ppage = dp->btcontrol.next) {
			e = bf_readbuf(-1, filenum, FC_FILEID(filenum), &pid, (PAGE **)&dp);
			if (e < eNOERROR) {
				(void) bf_freebuf(filenum, &hpid, (PAGE *)ht);
				return(e); /* return the error code */
			}
			if (i >= (1 << LOCALDEPTH(dp))) {
				(void) bf_freebuf(filenum, &pid, (PAGE *)dp);
				break;	/* already printed */
			}
			(void) bt_print_btpage(dp);
			(void) bf_freebuf(filenum, &pid, (PAGE *)dp);
		}
	}
	printf("\n");

	(void) bf_freebuf(filenum, &hpid, (PAGE *)ht);
	return(eNOERROR);
	
} /* h_dumpfile */

