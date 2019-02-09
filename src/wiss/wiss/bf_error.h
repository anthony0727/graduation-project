
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



#define	e1PAGENOTFOUND	-110	/* page not found in buffer pool */
#define	e1NOFREEBUFFERS	-210	/* no more free buffers */
#define	e1WRONGBUFFER	-310	/* accessing the wrong buffer */
#define	e1NULLFIDPARM	-510	/* null FID parameter */
#define	e1NULLPIDPARM	-610	/* null PID parameter */
#define	e1BADMODEPARM	-810	/* bad file mode parameter */
#define	eNOMOREBUCKETS	-910	/* no more hash buckets */
#define	eNOENTRY	-911	/* no entry for deletion */
#define	e1PAGEWASFOUND	-912	/* page was already in buffer pool */
#define	e1PAGEWASFIXED	-913	/* page was still fixed in buffer pool */
