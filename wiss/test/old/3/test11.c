/* program to test long data item routines */
/* 	
	This test program reads in sentences (end with '.') from
	the standard input and store it into a long data item.
	Then about half of the sentences are deleted.
	This program MUST be compiled with DEBUG on.
*/

#include <stdio.h>
#undef	 NULL
#include <wiss.h>
#include <lockquiz.h>
#include <locktype.h>

#define	WISSERROR(p,c)	if((int)(c)<0) am_fatalerror(p,(int)(c))
#define	ERROR(p,c)	if((int)(c)<0) am_error(p,(int)(c))
#define TESTFILE	"tfile25"
#define	PAGEFF		75
#define	SLICESIZE	60
	

main(argc,argv)
int	argc;
char	**argv;
{
	int 	i, j, k, vol, ofn;
	RID 	lrid;	
	char 	rec[1000];
	int	offset[1000];
	int	length[1000];
	int	s_count;	/* sentence count */
	int	sid;
	int	end_of_file;
	int	trans_id;

	wiss_checkflags(&argc,&argv);

        (void) wiss_init();                     /* initialize level 3 */

        trans_id = begin_trans();               /* inserted by LSM */
        printf("new transaction id = %d\n",trans_id);

	vol = st_mount("wiss3");  	/* mount the device wiss3 */
	WISSERROR("test11/st_mount", vol);

	i = st_createfile(vol, TESTFILE, 9, PAGEFF,PAGEFF);
	WISSERROR("test11/st_createfile", i);

	i = st_dismount("wiss3");  /* dismount the device wiss3 */
	WISSERROR("test11/st_dismount", i);


	vol = st_mount("wiss3");  	/* mount the device wiss3 */
	WISSERROR("test11/st_mount", vol);

	ofn = st_openfile(vol, TESTFILE, WRITE);
	WISSERROR("test11/st_openfile(for insertion)", ofn);
	
	i = st_createlong(ofn, &lrid);
	WISSERROR("test11/st_createlong", i);

	printf(" About to test insert_long \n");
	printf(" The slice size is %d, the page fill factor is %d %%\n", 
					SLICESIZE, PAGEFF); 

              /* lock the file in IX mode */
        i = wiss_lock_file(trans_id, ofn, l_IX, COMMIT, FALSE);
        WISSERROR("test1/lockfile",i);

	sid = am_openlongscan(ofn, &lrid, trans_id, TRUE, l_IX, FALSE);
	WISSERROR("test11/am_openlongscan", sid);
	printf(" Open long scan %d on file %d\n", sid, ofn);

	for (end_of_file = FALSE, k = s_count = 0; 
		end_of_file == FALSE ; k = k + i, s_count++)
	{
		j = am_setcursor(sid, k, 0);
		WISSERROR("test11/am_setcursor", j);

		if (s_count >= 1000)
			break;	/* too many sentences to handle */

		offset[s_count] = k;

		for (i = 0; i < 1000 ; i++)
		{
			rec[i] = getchar();
			if (rec[i] == EOF || rec[i] == '.')
				break;
			if (rec[i] == '\n')	/* replace CRLF */
				rec[i] = ' ';
		}

		if (rec[i] == EOF)
		{
			rec[i] = '\0';
			end_of_file = TRUE;
		}
		else rec[++i] = '\0';

		length[s_count] = i;

		j = am_insertlong(sid, (short) FALSE, rec, i);
		WISSERROR("test11/am_insertlong", j);

	}

	printf(" Insert %d sentences into the long item\n", s_count);
	r_dumplong(ofn, &lrid, trans_id, TRUE, FALSE);


	printf(" read the %d%s sentence : \n", s_count - 1,
		s_count==2?"st" : s_count==3?"nd" : s_count == 4? "rd":"th");

	j = am_setcursor(sid, offset[s_count - 2], 0);
	WISSERROR("test11/am_setcursor", j);

	j = am_readlong(sid, rec, length[s_count - 2]);
	WISSERROR("test11/am_readlong", j);

	rec[length[s_count - 2]] = '\0';

	printf(" Here it is : %s\n", rec);

	printf(" \n\nDelete half of the sentences\n");

	for (i = s_count - 2; i >= 0; i -= 2)
	{
		j = am_setcursor(sid, offset[i], 0);
		WISSERROR("test11/am_setcursor", j);

		j = am_deletelong(sid, length[i]);
		WISSERROR("test11/am_deletelong", j);
		
	}

	r_dumplong(ofn, &lrid, trans_id, TRUE, FALSE);

	printf(" Finally, compress the long data item.\n");

	j = st_compresslong(ofn, &lrid, 0, trans_id, TRUE, FALSE);
	WISSERROR("test11/st_compresslong", j);

	r_dumplong(ofn, &lrid, trans_id, TRUE, FALSE);

	i = st_closefile(ofn);
	WISSERROR("test11/st_closefile", i);

	i = st_destroyfile(vol, TESTFILE, trans_id, TRUE, FALSE);
	WISSERROR("test11/st_destroyfile", i);

         /* now commit the transaction */
         i = commit_trans(trans_id);
         if (i != 1)
            printf("error status return from commit_trans = %d\n", i);
         else printf("commit ok\n");

	i = st_dismount("wiss3");  /* dismount the device wiss3 */
	WISSERROR("test11/st_dismount", i);
        (void) wiss_final();

	printf("That's all folks\n");

}
