/* 
 * test4.c 
 *
 * To test the functions which realloc chunks of free nodes.
 *
 */

#include "wiss.h"		/* for FID ans PID */
#include "locktype.h"		/* for graph.h */
#include "lockquiz.h"		/* for l_IS, l_X ... */

extern activate();
extern committ_trans();
extern lock_file();
extern m_release_file();
extern lock_page();
extern m_release_page();

#define NB_LOCKS	33000

/* to test server_lm.o without the rpc connection */
reply_to_waiter(clientNo, transId, action) 
int clientNo, transId, action;  /* action is either GRANTED or ABORTED */
{
}


main()
{
  int		status;			/* for returned flags */
  int		trans;
  FID		file;
  PID		page;
  LOCKTYPE 	fmode, pmode;
  DURATION	dur;
  short		cond;
  

  /* constant parameters initialization */
  trans = 0;
  file.Fvolid = 1889;
  file.Ffilenum = 0;
  page.Pvolid = 1889;
  fmode = IS;
  pmode = S;
  dur = COMMIT;
  cond = FALSE;
  
  /* lockmanager start up */
  printf("initializes the resources \n");
  initialize_resources();
  activate(trans);
  
  /* lock 1 file */
  printf("lock file 0 \n");
  status = lock_file(trans, file, fmode, dur, cond);
  if (status < 0) {
    printf("lock_file: error = %d\n", status);
    exit(1);
  }

  /* lock NB_LOCKS pages */
  printf("lock %d pages:  ", NB_LOCKS);
  for (page.Ppage = 0; page.Ppage < NB_LOCKS; page.Ppage++) {
    if (page.Ppage % 100 == 0) printf(" %d ", page.Ppage);
    status = lock_page(trans, file, page, pmode, dur, cond);
    if (status < 0) {
      printf("lock_page: error = %d\n", status);
      exit(1);
    }
  }

  /* lockmanager end up */
  committ_trans(trans);
}
