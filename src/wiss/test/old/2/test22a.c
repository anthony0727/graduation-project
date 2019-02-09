/* program to test long data item routines */
/* 	
*/

#include <wiss.h>
#include <st.h>

#define	DEVNAME		"/dev/rhp2g"

extern char *io_error(), *bf_error(), *st_error();

#define	IOERROR(p,c)	if((int)(c)<0) \
		{printf("%s %s\n", p, io_error((int)(c)));io_final(); exit(-1);}
#define	BFERROR(p,c)	if((int)(c)<0) \
		{printf("%s %s\n", p, bf_error((int)(c)));bf_final(); io_final(); exit(-1);}

#define	ERROR(p,c)	if((int)(c)<0) printf("%s %s\n", p, st_error((int)(c)))
#define	STERROR(p,c)	if((int)(c)<0) {ERROR(p, c); st_final(); bf_final();\
				io_final();}

#define FILENAME	"tfile25"
#define	PFF		75
	

main(argc,argv)
int	argc;
char	**argv;
{
	int 	i, j, k, vol, ofn;
	RID 	lrid;	
	char 	rec[1000];
	int	end_of_file;

	wiss_checkflags(&argc,&argv);
	i = io_init();			/* initialize level 0 */
	IOERROR("test22a/io_init", i);
	
	i = bf_initialize();		/* initialize level 1 */
	BFERROR("test22a/bf_initialize", i);

	i = st_init();			/* initialize level 2 */
	STERROR("test22a/st_init", i);

	vol = st_mount(DEVNAME);  	/* mount the device wiss2 */
	STERROR("test22a/st_mount", vol);

	i = st_createfile(vol, FILENAME, 9, PFF,PFF);
	STERROR("test22a/st_createfile", i);

	i = st_dismount(DEVNAME);  /* dismount the device wiss2 */
	STERROR("test22a/st_dismount", i);

	printf(" about to test insert_frame \n");
	printf(" the slice size is %d, page fill factor %d %%\n", 
					SLICESIZE, PFF); 

	vol = st_mount(DEVNAME);  	/* mount the device wiss2 */
	STERROR("test22a/st_mount", vol);

	ofn = st_openfile(vol, FILENAME, WRITE);
	STERROR("test22a/st_openfile(for insertion)", ofn);
	
	i = st_createlong(ofn, &lrid);
	CHECKERROR(i);

	for (end_of_file = FALSE, k = 0; end_of_file == FALSE ; k = k + i)
	{
		for (i = 0; i < 1000 ; i++)
		{
			rec[i] = getchar();
			if (rec[i] == EOF || rec[i] == '\n')
				break;
		}

		if (rec[i] == EOF)
		{
			rec[i] = '\0';
			end_of_file = TRUE;
		}
		else rec[++i] = '\0';

		j = st_insertframe(ofn, &lrid, k, rec, i);
		STERROR("test22a/st_insertframe", j);

		r_dumplong(ofn, &lrid);
	}

	i = st_closefile(ofn);
	STERROR("test22a/st_closefile(for insertion)", i);

	i = st_destroyfile(vol, FILENAME);
	STERROR("test22a/st_destroyfile", i);

	i = st_dismount(DEVNAME);  /* dismount the device wiss2 */
	STERROR("test22a/st_dismount", i);

	printf("That's all folks\n");

}
