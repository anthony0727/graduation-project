
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


union file_page {
	PID		page_id;
	FID		file_id;
};

/* 
 * There is one wait_node for each transaction waiting on a resource
 * (a file or a page) wait_nodes are threaded off the "wait" list of 
 * "node" structure.  If a transaction must wait for a resource, it
 *  puts itself to sleep by waiting on its own private semaphore 
 *  which is a componet of the "users" structure in shared memory 
 */

struct  wait_node {
	LOCKTYPE serving;
	int		trans_id;    /* The transaction waiting at the head */
	struct		wait_node	*flink, *blink;
	SEMNODE		*monitor;    	
	int upgrade; /* This variable helps to avoid a search to 
			determine whether an upgrade request is waiting */
	DURATION duration; 
};

/*
 *  Node corresponds to each resource in the envirnment which 
 *  is to be locked (in our case it is either a page or a file.  
 *  Each resource has a linked list attached to it which represents 
 *  the transactions which are waiting on it. 
 *  Each node corresponding to a resource has a latch for concurrency in it 
 *  (the name of it is concurrent). At times only the node under attention is 
 *  updated (eg when linked list of resources waiting is updated), and it 
 *  would be wastefull if the whole list was locked rather than just 
 *  that specific node.
 */

struct node {
	int		trans_id;
	short           no_locks[MAXLOCKTYPES];
	short           no_locktypes;
	LATCH		concurrent;
	union		file_page  owner_id;
	LOCKTYPE serving;
	enum		belongsto	owner;
    /* The linked list of trans waiting for the resources  */
    /* *wait is a linked list for waiting transactions, whereas the *o_list */
    /* is the list of the owners of the resource. */
	struct		wait_node	*wait, *o_list;  
	struct		node		*flink, *blink;
    /* the double linked list for the pages which belong a file. */
	struct 		node		*f_page, *b_page;
};
