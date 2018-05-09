/* 
 * test3.c
 *
 * To profile and optimize the function initialize_resources(). 
 *
 */

/* to test server_lm.o without the rpc connection */
reply_to_waiter(clientNo, transId, action) 
int clientNo, transId, action;  /* action is either GRANTED or ABORTED */
{
  printf("\n        (call to function reply_to_waiter ==> nop) \n");
}


main()
{
	printf(" START: Driver for lock manager ...\n");
	initialize_resources();
	printf("\n ...TERMINATED run\n");
}
