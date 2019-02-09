/* program to test opening and closing a file */

extern char *io_error(), *bf_error(), *st_error();

#define	IOERROR(p,c)	if((int)(c)<0) \
		{printf("%s %s\n", p, io_error((int)(c)));io_final(); exit(-1);}
#define	BFERROR(p,c)	if((int)(c)<0) \
		{printf("%s %s\n", p, bf_error((int)(c)));bf_final(); io_final(); exit(-1);}

#define	ERROR(p,c)	if((int)(c)<0) printf("%s %s\n", p, st_error((int)(c)))
#define	STERROR(p,c)	if((int)(c)<0) {ERROR(p, c); st_final(); bf_final();\
				io_final();}
#define TERROR(p,c) 	if((int)(c)<0) ERROR(p,(int)(c));\
			else printf("%s did not catch error !\n", p)

#define	QERROR(pp,c, a1, a2)\
			printf(" open the same file twice (%s-%s)\n", a1, a2);\
			if((int)(c)<0) printf("%s %s\n", pp, st_error(c);\
			else printf("No error ?\n")

#include <wiss.h>

main(argc,argv)
int	argc;
char	**argv;
{
	int i, vol, ofn;

	wiss_checkflags(&argc,&argv);
	i = io_init();			/* initialize level 0 */
	IOERROR("test9/io_init", i);
	
	i = bf_init();		/* initialize level 1 */
	BFERROR("test9/bf_init", i);

	i = st_init();			/* initialize level 2 */
	STERROR("test9/st_init", i);

	vol = st_mount("wiss2");  	/* mount the device wiss2 */
	STERROR("test9/st_mount", vol);

	i = st_createfile(vol, "tfile1", 9, 90,90);
	STERROR("test9/st_createfile", i);

/* open a file on an unmounted device */
	printf(" open a file on an unmounted device\n");
	i = st_openfile(vol + 1, "tfile1", READ);
	TERROR("test9/st_openfile", i);

/* open a file with undefined access mode */
	printf("open a file with wrong access mode -1\n");
	i = st_openfile(vol , "tfile1", -1);
	TERROR("test9/st_openfile", i);

/* open the same file twice : READ-READ, READ-WRITE, WRITE-READ, WRITE-WRITE */

	printf("about to open %s (READ-READ)\n", "tfile1");
	ofn = st_openfile(vol , "tfile1", READ);
	STERROR("test9/st_openfile", ofn);
	i = st_openfile(vol, "tfile1", READ);
	STERROR("test9/st_openfile", i);
	i = st_closefile(i);
	STERROR("test9/st_closefile", i);
	i = st_closefile(ofn);
	STERROR("test9/st_closefile", i);

	printf("about to open %s (READ-WRITE)\n", "tfile1");
	ofn = st_openfile(vol , "tfile1", READ);
	STERROR("test9/st_openfile", ofn);
	i = st_openfile(vol , "tfile1", WRITE);
	QERROR("test9/st_openfile", i, "READ", "WRITE");
	i = st_closefile(ofn);
	STERROR("test9/st_closefile", i);

	printf("about to open %s (WRITE-READ)\n", "tfile1");
	ofn = st_openfile(vol , "tfile1", WRITE);
	STERROR("test9/st_openfile", ofn);
	i = st_openfile(vol , "tfile1", READ);
	QERROR("test9/st_openfile", i, "WRITE", "READ");
	i = st_closefile(ofn);
	STERROR("test9/st_closefile", i);

	printf("about to open %s (WRITE-WRITE)\n", "tfile1");
	ofn = st_openfile(vol , "tfile1", WRITE);
	STERROR("test9/st_openfile", ofn);
	i = st_openfile(vol , "tfile1", WRITE);
	QERROR("test9/st_openfile", i, "WRITE", "WRITE");
	i = st_closefile(ofn);
	STERROR("test9/st_closefile", i);

	i = st_destroyfile(vol, "tfile1");
	STERROR("test9/st_destroyfile", i);

	i = st_dismount("wiss2");  /* dismount the device wiss2 */
	STERROR("test9/st_dismount", i);

}
