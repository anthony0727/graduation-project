
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



extern int procNum;  


#define INIT_LOCK_NODE_M(p,owner,flink,blink)\
    { p->resptr = NULL;\
      p->flink = flink;\
      p->blink = blink;\
      p->collision = NULL;\
    }

#define   INIT_RES_NODE_M(m_source, m_flink, m_blink, m_trans_id)\
{\
  LOCKTYPE lkt;\
  \
  for (lkt=0; lkt<MAXLOCKTYPES; lkt++)\
    m_source->no_locks[lkt] = 0;\
  m_source -> no_locktypes = 0;\
  m_source -> serving = l_NL;\
  m_source -> owner = NOBODY;\
  m_source -> flink = m_flink;\
  m_source -> blink = m_blink;\
  m_source -> wait = NULL;\
  m_source -> f_page = NULL;\
  m_source -> b_page = NULL;\
  m_source -> trans_id = m_trans_id;\
}

#define INIT_WAIT_NODE_M(m_source, m_transid, m_flink, m_blink, m_request)\
{\
  m_source -> serving = m_request;\
  m_source -> trans_id = m_transid;\
  m_source -> flink = m_flink;\
  m_source -> blink = m_blink;\
  }

#define MALLOC_LOCK_M(p)\
    { SetLatch(&smPtr->freeLockLatch, procNum, NULL);\
      if (smPtr->freelocks[0].flink == &smPtr->freelocks[0]) alloc_more_locks ();\
      p=smPtr->freelocks [0].flink;\
      smPtr->freelocks[0].flink=(p)->flink;\
      (p)->flink = (p)->blink = (p)->collision = NULL;\
      ReleaseLatch(&smPtr->freeLockLatch, procNum);\
    }
	
#define FREE_LOCK_NODE_M(p)\
    { SetLatch(&smPtr->freeLockLatch, procNum, NULL);\
      p->resptr=NULL;\
      if (p->blink != NULL) p->blink->flink = p->flink;\
      if (p->flink != NULL) p->flink->blink = p->blink;\
      p->flink = smPtr->freelocks[0].flink;\
      smPtr->freelocks[0].flink=p;\
      ReleaseLatch(&smPtr->freeLockLatch, procNum);\
    }


#define RELEASE_NODE_M(i,bucket)\
    { SetLatch(&smPtr->freeNodeLatch, procNum, NULL);\
      if (bucket -> blink != NULL) bucket -> blink -> flink = bucket -> flink;\
      if (bucket -> flink != NULL) bucket -> flink -> blink = bucket -> blink;\
      if (smPtr->locktable [i].lockptr == bucket) smPtr->locktable [i].lockptr = bucket -> flink;\
      ReleaseLatch(&bucket->concurrent,procNum);\
      bucket -> owner = NOBODY;\
      bucket -> flink = smPtr->freenodes [0].flink;\
      bucket -> wait = NULL;\
      bucket -> trans_id = -1;\
      smPtr->freenodes [0].flink = bucket;\
      ReleaseLatch(&smPtr->freeNodeLatch, procNum);\
    }

#define ADDBUCKET_M(p,Mindex)\
    { SetLatch(&smPtr->freeNodeLatch, procNum, NULL);\
      if(smPtr->freenodes [FREECELL].flink == &smPtr->freenodes[FREECELL]) alloc_more_node();\
      p = smPtr->freenodes [FREECELL].flink;\
      smPtr->freenodes [FREECELL].flink = p->flink;\
      p->wait = p->o_list = NULL;\
      p->f_page = p->b_page = NULL;\
      p->flink = p->blink = NULL;\
      if (smPtr->locktable[Mindex].lockptr == NULL) smPtr->locktable [Mindex].lockptr = p;\
      else { p->flink = smPtr->locktable[Mindex].lockptr;\
	     smPtr->locktable [Mindex].lockptr->blink = p;\
	     smPtr->locktable [Mindex].lockptr = p;\
      }\
      ReleaseLatch(&smPtr->freeNodeLatch, procNum);\
    }

#define LOCK_STRENGTH_M(mlone,mltwo,mlres)\
    {\
	    *mlres = LM_conv[mlone][mltwo];\
    }


#define MALLOC_OWNERNODE_M(p)\
    { SetLatch(&smPtr->lockLatch, procNum, NULL);\
      if (smPtr->freewaitnodes [FREEWAIT].flink == &smPtr->freewaitnodes[FREEWAIT]) alloc_more_waitnode ();\
      p = smPtr->freewaitnodes [FREEWAIT].flink;\
      smPtr->freewaitnodes [FREEWAIT].flink = p -> flink;\
      p -> flink = p -> blink = NULL;\
      ReleaseLatch(&smPtr->lockLatch, procNum);\
    }

#define MALLOC_WAITNODE_M(p)\
    { SetLatch(&smPtr->lockLatch, procNum, NULL);\
      if (smPtr->freewaitnodes [FREEWAIT].flink == &smPtr->freewaitnodes[FREEWAIT]) alloc_more_waitnode ();\
      p = smPtr->freewaitnodes [FREEWAIT].flink;\
      smPtr->freewaitnodes [FREEWAIT].flink = p -> flink;\
      p -> flink = p -> blink = NULL;\
      ReleaseLatch(&smPtr->lockLatch, procNum);\
    }

#define FREE_WAIT_NODE_M(p)\
    { SetLatch(&smPtr->lockLatch, procNum, NULL);\
      p -> flink = smPtr->freewaitnodes [FREEWAIT].flink;\
      smPtr->freewaitnodes [0].flink = p;\
      ReleaseLatch(&smPtr->lockLatch, procNum);\
    }


#define CREATE_PAGE_LOCKNODE_M(p,tid,filep,mypid,req)\
    { int  index = hash_page(mypid);\
      ADDBUCKET_M(p,index);\
      p -> owner_id.page_id = mypid;\
      p -> owner = PAGER;\
      p -> serving = req;\
      p -> trans_id = tid;\
	  if(!(p->no_locks[req]++))\
		  p->no_locktypes++;\
\
      if (filep != NULL) {\
        p->f_page = filep->f_page;\
        filep->f_page = p;\
        p->b_page = filep;\
        if (p->f_page != NULL) p->f_page->b_page = p;\
      }\
    }

#define CREATE_FILE_LOCKNODE_M(p,tid,myfid,req)\
    { int index = hash_file(myfid);\
      ADDBUCKET_M(p,index);\
      p -> owner_id . file_id = myfid;\
      p -> owner = FILER;\
      p -> serving = req;\
      p -> trans_id = tid;\
	  if(!(p->no_locks[req]++))\
			p->no_locktypes++;\
    }

#define CREATE_PAGE_LOCK_M(mnode,mpid,mreq,mresource)\
    { struct    lockq    *cell;\
      int	i;\
      MALLOC_LOCK_M(cell);\
      cell -> request = mreq;\
      cell -> resptr = mresource;\
      if (mnode -> locks_held == NULL) mnode -> locks_held = cell;\
      else\
      { cell -> flink = mnode -> locks_held;\
	mnode -> locks_held -> blink = cell;\
	mnode -> locks_held = cell;\
      }\
      i = HASHLOCALPID(mpid);\
      cell->collision = mnode->pidHashTbl->pidChain[i];\
      mnode->pidHashTbl->pidChain[i] = cell;\
    }

#define CREATE_FILE_LOCK_M(mnode,mfid,mreq,mresource)\
    { struct    lockq    *cell;\
      int	i;\
      MALLOC_LOCK_M(cell);\
      cell -> request = mreq;\
      cell -> resptr = mresource;\
      if (mnode -> locks_held == NULL) mnode -> locks_held = cell;\
      else\
      { cell -> flink = mnode -> locks_held;\
	cell -> flink -> blink = cell;\
	mnode -> locks_held = cell;\
      }\
      i = HASHLOCALFID(mfid);\
      cell->collision = mnode->fidHashTbl->fidChain[i];\
      mnode->fidHashTbl->fidChain[i] = cell;\
    }

#define ADDOWNER_M(mnode, mreq, mtid)\
    { struct wait_node *mcell;\
      LOCKTYPE mstr;\
      int       locmtrans;\
      if (mnode -> trans_id == -1)\
      { mnode -> trans_id = mtid; mnode -> serving = mreq;}\
      else { MALLOC_OWNERNODE_M(mcell);\
	     mcell -> trans_id = mtid;\
	     mcell -> serving = mreq;\
	     if (mnode -> o_list == NULL) mnode -> o_list = mcell;\
	     else { mcell -> flink = mnode -> o_list;\
		    mnode -> o_list -> blink = mcell;\
		    mnode -> o_list = mcell;\
	     }\
	     LOCK_STRENGTH_M(mnode -> serving, mreq,&mstr);\
	     if (mstr != mnode -> serving)\
	     {\
	       mstr = mnode -> serving;\
	       locmtrans = mnode -> trans_id;\
	       mnode -> serving = mcell -> serving;\
	       mnode -> trans_id = mcell -> trans_id;\
	       mcell -> serving = mstr;\
	       mcell -> trans_id = locmtrans;\
	     }\
	}\
 	mnode -> no_locks[mreq]++;\
	if (mnode -> no_locks[mreq] == 1)\
	  mnode -> no_locktypes++;\
   }


#define ADDOWNER_U_M(mbucket, mreq, mtid)\
{\
	struct       wait_node    *temp, *mainone, *cell;\
	LOCKTYPE strongest, oldmreq;\
	short     done;\
	int       temptrans;\
\
	if (mbucket -> mtid == -1) {\
		printf("addowner_u: Cannot have empty owner FATAL\n");\
		exit(1);	\
	}\
	if (mbucket -> mtid == mtid){\
		oldmreq = mbucket->serving;\
	    mbucket -> mtid = mtid;\
	    mbucket -> serving = mreq;\
		if ((LM_supr[mreq][oldmreq] != mreq) &&\
				(mbucket->no_locktypes > 1)){\
			mainone = NULL;\
			strongest = mbucket -> serving;\
			for(temp = mbucket->o_list;temp != NULL; temp = temp -> flink){\
	    		if (LM_supr[temp -> serving][strongest] != strongest) {\
					strongest = temp -> serving;\
					mainone = temp;\
	    		}\
			}\
			if (mainone != NULL) {\
	    		temptrans = mainone->mtid;\
	    		mainone -> serving = mbucket -> serving;\
	    		mainone -> mtid = mbucket -> mtid;\
	    		mbucket -> mtid = temptrans;\
	    		mbucket -> serving = strongest;\
			}\
		}\
	}\
	else {\
	cell = mbucket -> o_list;\
	while((cell != NULL)) {\
		if (cell -> mtid == mtid)\
			break;\
		cell = cell->flink;\
	}\
	if(cell == NULL){\
		printf("addowner_u: No owner node in the resource list\n");\
		return(NOT_OK);\
	}\
	else {\
	oldmreq = mbucket->serving;\
	cell -> serving = mreq;\
	if (LM_supr[mreq][oldmreq] == mreq) {\
		temptrans = mbucket -> mtid;\
		mbucket -> serving = cell -> serving;\
		mbucket -> mtid = cell -> mtid;\
		cell -> serving = oldmreq;\
		cell -> mtid = temptrans;\
	}\
	}\
	}\
}


#define RELEASE_OWNER_NODE_M(mbucket, tid, mmode)\
{\
    struct    wait_node    *temp;\
    LOCKTYPE strongest;\
    struct      wait_node              *mainone;\
    int         loc_trans;\
\
    if (mbucket -> trans_id == tid)\
    {\
	*mmode = mbucket -> serving;\
	if (mbucket -> o_list != NULL)\
	{\
	    mbucket -> trans_id = mbucket -> o_list -> trans_id;\
	    mbucket -> serving = mbucket -> o_list -> serving;\
	}\
	else\
	    mbucket -> trans_id = -1;\
	temp = mbucket -> o_list;\
    }\
    else \
    {\
	for (temp = mbucket -> o_list; (temp != NULL && (temp -> trans_id != tid)); temp = temp -> flink);\
	if (temp == NULL)\
	{\
	    printf ("\n ERROR, EXTREMELY DANGEROUS, unknown owner %d to release.\n",tid);\
	    printf ("The owners of the resource are : ");\
	    printf ("The head of the owners is %d\n",mbucket->trans_id);\
	    for (temp = mbucket -> o_list; (temp != NULL); temp=temp->flink) \
		printf ("%d ",temp->trans_id);\
	    exit(1);\
	}\
	*mmode = temp -> serving;\
    }\
    if (temp != NULL)\
    {\
        if (mbucket -> o_list == temp)\
	    mbucket -> o_list = temp -> flink;\
        if (temp -> flink != NULL)\
	    temp -> flink -> blink = temp -> blink;\
        if (temp -> blink != NULL)\
	    temp -> blink -> flink = temp -> flink;\
        FREE_WAIT_NODE_M(temp);\
    }\
    if (mbucket -> o_list != NULL) {\
\
    if (mbucket -> no_locktypes > 1)\
    {\
	mainone = NULL;\
	strongest = mbucket -> serving;\
	for (temp = mbucket -> o_list; temp != NULL; temp = temp -> flink)\
	{\
	    if (LM_supr[temp -> serving][strongest] != strongest)\
	    {\
		strongest = temp -> serving;\
		mainone = temp;\
	    }\
	}\
	if (mainone != NULL)\
	{\
	    loc_trans = mainone->trans_id;\
	    mainone -> serving = mbucket -> serving;\
	    mainone -> trans_id = mbucket -> trans_id;\
	    mbucket -> trans_id = loc_trans;\
	    mbucket -> serving = strongest;\
	}\
    }\
	}\
}
#define RELEASE_PAGE_LOCK_M(tcell, mpid,mres)\
{\
    LOCKTYPE  tlmode;\
    int       index;\
\
    if (mres == NULL) {\
		printf (\
		"ERROR (macro  RELEASE_PAGE_LOCK_M), Xact %d has no such page \n"\
				,tcell->trans_id);\
		return(ABORTED);\
    }\
    RELEASE_OWNER_NODE_M(mres,tcell->trans_id,&tlmode);\
\
    if (--(mres->no_locks[tlmode]) == 0){\
		 mres->no_locktypes--;\
    }\
    if (mres -> wait != NULL ) {\
		 if (mres->no_locktypes)\
	         	walkfile (mres, mres->serving);\
		 else walkfile(mres, l_NL);\
    }\
\
    if ( (mres -> wait == NULL) && (mres -> no_locktypes == 0)) {\
\
                if (mres->b_page != NULL) /* for NULL FID */ \
                  mres->b_page->f_page = mres->f_page;\
		if (mres->f_page != NULL) mres->f_page->b_page = mres->b_page;\
\
		index = hash_page (mpid);\
		RELEASE_NODE_M(index, mres);\
    } else ReleaseLatch(&mres->concurrent,procNum);\
}


#define RELEASE_FILE_LOCK_M(tcell, mfid, mresource)\
{\
    LOCKTYPE  mt_lockmode;\
    int       index;\
    \
    if (mresource == NULL) {\
		printf (\
		"ERROR, no such file(%d,%d)acquired to release,mresource->trans_id=%d.\n",\
				mfid.Fvolid, mfid.Ffilenum, tcell->trans_id);\
		return(-1);\
    }\
\
    RELEASE_OWNER_NODE_M(mresource,tcell->trans_id,&mt_lockmode);\
\
    if (--(mresource->no_locks[mt_lockmode]) == 0){\
		/*\
		 * Decrement Number of holders for resource XXXXXXX\
		 */\
		mresource->no_locktypes--;\
    }\
    if (mresource -> wait != NULL ) {\
			if(mresource->no_locktypes)\
	         	walkfile (mresource, mresource->serving);\
			else walkfile (mresource, l_NL);\
    }\
\
    if ((mresource->wait == NULL) && (mresource->no_locktypes == 0)) {\
		index = hash_file (mfid);\
		RELEASE_NODE_M(index, mresource);\
    } else ReleaseLatch(&mresource->concurrent,procNum);\
}

