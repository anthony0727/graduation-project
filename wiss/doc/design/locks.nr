.bp
.sh 1 "Transactions and Locking"
.pp
WiSS provides support for locking and transactions through the
following routines:
.np
trans_id = begin_trans();
.nf
.fi
.sp 1
Begins a new transaction and returns an integer that identifies 
the transaction.  
.np
commit_trans(trans_id)
.nf
int	trans_id;
.fi
.sp
Commits the effects of a transaction and releases all locks
held by the transaction.
.br
.np
abort_trans(trans_id)
.nf
int	trans_id;
.fi
.sp
Aborts the effects of a transaction and releases all locks
held by the transaction.  
.lp
.pp
Calls to the lock manager are performed automatically by
WiSS when the system is accessed through the level 3 interface.
A number of the lower level routines take 3 new parameters:
a transaction identifier, a \fBlockup\fR flag (a boolean), and a \fBcond\fR
flag (also a boolean).  The lockup flag turns on locking if it
is set to TRUE.  If the lockup flag is TRUE, then the conditional
flag is used by the lock manager to determine how to handle 
lock waits.  If cond is FALSE, on a conflict lock request the lock 
manager will block the requesting process until either the resource can be
granted or the lock manager detects a deadlock.  If cond is TRUE,
instead of blocking the requesting process on a conflicting
lock request, the lock manager calls the user defined exception
handling procedure will_block().  \fBMore is needed here.\fR
.pp
Certain level 3 calls expect a lockMode parameter.  The valid
lock modes are:  l_NL, l_IS, l_IX, l_S, l_SIX, l_X.  These
values are defined in the include file <lockquiz.h>.
.pp
It is \fBvery important\fR to notice that opening a file or
an index directly via the calls wiss_openfile(), wiss_openindex(),
and wiss_openhash() \fBdoes not set any locks\fR on the file
or index.  If you intend to by-pass the level 3 scan interfaces
and access records and indices directly you must \fBexplicitly\fR lock the 
file and/or index in the appropriate mode.  
Once a lock has been set on a file, the record and index routines
will automatically set the appropriate locks on the pages they
access.
The following routines can be used to set a lock on a file:
.np
lock_file
(transId, fid, lockMode, duration, cond)
.nf
int	transId;
FID	fid;
short	lockMode;
DURATION duration;
short	cond;
.fi
.sp
Locks the file identified by the fid parameter in mode lockMode.
lockMode must be one of the valid lock modes:  l_NL, l_IS, l_IX,
l_S, l_SIX, l_X.  duration must be either COMMIT or MANUAL.  
To convert an open file number (of either a file or an index)
to a fid, use the macro F_FILEID(fid) which is defined in
wiss_r.h
If duration is equal to COMMIT the lock is held until the 
transaction commits.  MANUAL specifies that the lock may be 
released before the end of
the transaction.  cond is either TRUE or FALSE and indicates
whether a conditional lock is being requested (cond==TRUE).
.sp
A lock request will return one of the following values:
ABORTED, GRANTED, NOT_GRANTED, WAIT, COND_ABORTED, COND_WAIT 
(defined in lockquiz.h)
.sp
To manually release the lock on a file or index use the routine:
.np
m_release_file
(transId, fid)
.nf
int	transId;
FID	fid;
.fi
Releases all locks held by transaction transId on file fid 
and returns either OK if all went well.
.br
The lock manager supports the following calls to set and release
locks on pages:
.np
lock_page
(transId, fid, pid, lockMode, duration, cond, getPageFlag)
.nf
int	transId;
FID	fid;
PID	pid;
short	lockMode;
DURATION duration;
short	cond;
short	getPageFlag;
.fi
.sp
Locks the page identified by the fid, pid pair in mode lockMode.
lockMode must be one of the valid lock modes:  l_NL, l_IS, l_IX,
l_S, l_SIX, l_X.  duration must be either COMMIT or MANUAL.  
If COMMIT the lock is held until the transaction commits.  MANUAL
specifies that the lock may be released before the end of
the transaction.  cond is either TRUE or FALSE and indicates
whether a conditional lock is being requested (cond==TRUE).
.sp
A lock request will return one of the following values:
ABORTED, GRANTED, NOT_GRANTED, WAIT, COND_ABORTED, COND_WAIT.
.sp
If getPageFlag is true, the rpc call to the server will attempt
to get both the lock and the data page being locked.  This
flag should be viewed as a hint as under certain conditions
(e.g., the server lock manager cannot grant the lock), the
page will not be returned to the workstation.
.np
m_release_page
(transId, pid)
.nf
int	transId;
FID	pid;
.fi
Releases the lock on page pid held by transaction transId
and returns either OK if all went well.
.br
