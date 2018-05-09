
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



#define	DEBUG	1

static int	cur_user;

/* get the id of the current user */
sys_getuser()
{
#ifdef	DEBUG
	return(cur_user);
#else
	return(0);
/*
	return(getuid());
*/
#endif
}

/* this routine is callable only under debug mode */
sys_setuser(user)
int	user;
{
#ifdef	DEBUG
	cur_user = user;
#endif
}

static int	vir_clock = 0;

sys_time()
/* Wiss's virtual clock */
{
	return(vir_clock++);
}
