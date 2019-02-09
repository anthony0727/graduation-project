
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


/* test1.c */

#include <curses.h>
/* <stdio.h> <sgtty.h> are included in <curses.h> */

struct sgttyb old,new;

init_sgttyb()
{
	ioctl(0,TIOCGETP,&old);
	new.sg_ispeed = old.sg_ispeed;
	new.sg_ospeed = old.sg_ospeed;
	new.sg_erase = old.sg_erase;
	new.sg_kill = old.sg_kill;
	new.sg_flags = RAW;
}


new_sgtty()
{ ioctl(0,TIOCSETP,&new);}

old_sgtty()
{ioctl(0,TIOCSETP,&old);}

cont()
{
	new_sgtty();
 	printf("\n\rPress <CR> to continue ");
	while( getchar() != '\r'); 
	old_sgtty();
}
