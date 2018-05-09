
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


From bolo%garfield@cs.wisc.edu Fri Mar  2 23:22:30 1990
Return-Path: <bolo%garfield@cs.wisc.edu>
Received: from inria.inria.fr by bdblues.altair.fr (4.0/server.22-09-87)
	id AA13097; Fri, 2 Mar 90 23:22:29 +0100;dollarg = bolo%garfield@cs.wisc.edu
Received: from kraft-slices.cs.wisc.edu by inria.inria.fr (5.61+/89.0.8) via Fnet-EUnet id AA02064; Fri, 2 Mar 90 23:25:00 +0100 (MET)
Received: from garfield.cs.wisc.edu by kraft-slices.cs.wisc.edu; Fri, 2 Mar 90 16:25:13 -0600
Date: Fri, 2 Mar 90 16:25:10 CST
From: bolo%garfield@cs.wisc.edu (Joe Burger)
Message-Id: <9003022225.AA11954@garfield.cs.wisc.edu>
Received: by garfield.cs.wisc.edu; Fri, 2 Mar 90 16:25:10 CST
To: dewitt%garfield@cs.wisc.edu
Subject: bug in 68000 bitmap code
Status: RO


I was doing some assembly work, and noticed two typos in the 68000
bitmap code I did for you.  The problem would only show up if the
bitmap size argument to wsetmap or clearmap was greater than 512k.

bolo/joe

Here is a context diff

*** /tmp/,RCSt1023544	Fri Mar  2 16:20:48 1990
--- 68kbits.S	Fri Mar  2 16:19:45 1990
***************
*** 85,91 ****
  	moveb	d1,a0@+
  2:
  	dbra	d0,1b
! 	clrl	d0
  	subql	#1,d0
  	jcc	1b
  
--- 85,91 ----
  	moveb	d1,a0@+
  2:
  	dbra	d0,1b
! 	clrw	d0
  	subql	#1,d0
  	jcc	1b
  
***************
*** 104,110 ****
  	clrb	a0@+
  2:
  	dbra	d0,1b
! 	clrl	d0
  	subql	#1,d0
  	jcc	1b
  
--- 104,110 ----
  	clrb	a0@+
  2:
  	dbra	d0,1b
! 	clrw	d0
  	subql	#1,d0
  	jcc	1b
  

