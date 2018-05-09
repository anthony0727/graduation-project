
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
/* Module d_dumptable : print the open file table 

   IMPORTS:
	io_createfile(volid, numpages, extentfillfactor, *newfid)
	io_destroyfile(*fid)
	ST_newfiledesc(volid, filename, *fid, pagefillfactor)

   EXPORTS:
	st_createfile(volid, filename, numpages, extentfillfactor, 
			pagefillfactor)

*/

#include <wiss.h>
#include <st.h>

d_dumptable()
/* Dump the open file table 

   Returns:
	None

   Side Effects:
	None

   Errors generated here:
	None

*/

{
	register i;

	printf("+---------------------------------------------------------+\n");
	printf("|      Open File Table  (Level 2, Storage Structure)      |\n");
	printf("+---+----+----+----+----+-----+-----+---------------------+\n");
	printf("| # |User| Vol| FID|Mode|Pages| Type|     File Name       |\n");
	printf("+---+----+----+----+----+-----+-----+---------------------+\n");


	for (i = 0; i < MAXOPENFILES; i++)
	{
		if (smPtr->files[i].ptr < 0) continue;	
		printf("|%3d|%4d|%4d|%4d|%4.4s|%5d|%5.5s| %-20.20s|\n",
			i, F_USER(i), F_VOLUMEID(i), F_FILEID(i).Ffilenum,
			(smPtr->files[i].mode==READ)?"  R ":"  W ", 
			F_NUMPAGES(i), 
			(F_FILETYPE(i)==DATAFILE)?" file":"index",F_FILENAME(i));
	}
	printf("+---+----+----+----+----+-----+-----+---------------------+\n");
}


