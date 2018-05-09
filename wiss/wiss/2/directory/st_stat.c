
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
/* Module st_stat : statistics about the files

   IMPORTS :
	sys_getuser();
	ST_accessfiledesc();

   EXPORTS :
	int st_filepages (volid, filename)
	int st_indexpages (volid, filename, indexno)
	int st_recordcard (volid, filename)
	int st_keycard (volid, filename, indexno)

*/

#include <wiss.h>
#include <st.h>

int st_filepages(volid, filename)
int	volid;
char	*filename;
{

	FILEDESC	fd;
	int		e;

#ifdef TRACE
	if (checkset(&Trace2,tSTAT))
		printf("st_filepages(volid=%d,filename=%s)\n" ,volid, filename);
#endif

	/* get file descriptor from file directory */
	SetLatch(&smPtr->level2Latch, procNum, NULL);
	e = ST_accessfiledesc(volid,filename, &fd, READ); 
	CHECKERROR_RLATCH(e,&smPtr->level2Latch, procNum);
	ReleaseLatch(&smPtr->level2Latch, procNum);
	return(fd.numpages);
}

int st_recordcard(volid, filename)
int	volid;
char	*filename;
{
	FILEDESC	fd;
	int		e;

#ifdef TRACE
	if (checkset(&Trace2,tSTAT))
		printf("st_recordcard(volid=%d,filename=%s)\n" ,volid, filename);
#endif

	SetLatch(&smPtr->level2Latch, procNum, NULL);
	/* get file descriptor from file directory */
	e = ST_accessfiledesc(volid, filename, &fd, READ); 
	CHECKERROR_RLATCH(e,&smPtr->level2Latch, procNum);
	ReleaseLatch(&smPtr->level2Latch, procNum);
	return(fd.card);
}

int st_indexpages(volid, filename, indexno)
int	volid;
char	*filename;
int	indexno;
{
	FILEDESC	fd;
	char		name[MAXFILENAMELEN];
	int		e;

#ifdef TRACE
	if (checkset(&Trace2,tSTAT))
		printf("st_indexpages(volid=%d,filename=%s, indexno=%d)\n" 
			,volid, filename, indexno);
#endif
	SetLatch(&smPtr->level2Latch, procNum, NULL);

	suffixname(name, filename, INDEXSUF, indexno);
	/* get file descriptor from file directory */
	e = ST_accessfiledesc(volid,name, &fd, READ); 
	CHECKERROR_RLATCH(e,&smPtr->level2Latch, procNum);
	ReleaseLatch(&smPtr->level2Latch, procNum);
	return(fd.numpages);
}

int st_keycard(volid, filename, indexno)
int	volid;
char	*filename;
int	indexno;
{
	FILEDESC	fd;
	char		name[MAXFILENAMELEN];
	int		e;

#ifdef TRACE
	if (checkset(&Trace2,tSTAT))
		printf("st_keycard(volid=%d,filename=%s,indexno=%d)\n" 
			,volid, filename, indexno);
#endif
	SetLatch(&smPtr->level2Latch, procNum, NULL);
	suffixname(name, filename, INDEXSUF, indexno);

	/* get file descriptor from file directory */
	e = ST_accessfiledesc(volid,name, &fd, READ); 
	CHECKERROR_RLATCH(e,&smPtr->level2Latch, procNum);
	ReleaseLatch(&smPtr->level2Latch, procNum);
	return(fd.card);
}


int st_hashpages(volid, filename, indexno)
int	volid;
char	*filename;
int	indexno;
{
	FILEDESC	fd;
	char		name[MAXFILENAMELEN];
	int		e;

#ifdef TRACE
	if (checkset(&Trace2,tSTAT))
		printf("st_hashpages(volid=%d,filename=%s, indexno=%d)\n" 
			,volid, filename, indexno);
#endif
	SetLatch(&smPtr->level2Latch, procNum, NULL);
	suffixname(name, filename, HASHSUF, indexno);

	/* get file descriptor from file directory */
	e = ST_accessfiledesc(volid,name, &fd, READ); 
	CHECKERROR_RLATCH(e,&smPtr->level2Latch, procNum);
	ReleaseLatch(&smPtr->level2Latch, procNum);
	return(fd.numpages);
}

int st_hashcard(volid, filename, indexno)
int	volid;
char	*filename;
int	indexno;
{
	FILEDESC	fd;
	char		name[MAXFILENAMELEN];
	int		e;

#ifdef TRACE
	if (checkset(&Trace2,tSTAT))
		printf("st_hashcard(volid=%d,filename=%s,indexno=%d)\n" 
			,volid, filename, indexno);
#endif

	SetLatch(&smPtr->level2Latch, procNum, NULL);
	suffixname(name, filename, HASHSUF, indexno);

	/* get file descriptor from file directory */
	e = ST_accessfiledesc(volid,name, &fd, READ); 
	CHECKERROR_RLATCH(e,&smPtr->level2Latch, procNum);
	ReleaseLatch(&smPtr->level2Latch, procNum);
	return(fd.card);
}
