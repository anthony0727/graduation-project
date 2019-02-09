/* program to test file protection (read/write, owner/other) */
/* the library sys_getuser must be compiled with DEBUG on */

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


#define	TESTFILE	"tfile1"

#include <wiss.h>

main(argc,argv)
int	argc;
char	**argv;
{
	int i, vol, ofn;

	printf(" A remainder : sys_getuser must be compiled");
	printf(" with DEBUG flag on!\n");

	wiss_checkflags(&argc,&argv);
	i = io_init();			/* initialize level 0 */
	IOERROR("test24/io_init", i);
	
	i = bf_init();		/* initialize level 1 */
	BFERROR("test24/bf_init", i);

	i = st_init();			/* initialize level 2 */
	STERROR("test24/st_init", i);

	vol = st_mount("wiss2");  	/* mount the device wiss2 */
	STERROR("test24/st_mount", vol);

sys_setuser(1000); /* the owner of the file to be created */

	printf(" * create the file - %s\n", TESTFILE);
	i = st_createfile(vol, TESTFILE, 9, 90,90);
	STERROR("test24/st_createfile", i);

	printf(" * open and close the file for read (by owner)\n");
	i = st_openfile(vol, TESTFILE, READ);
	STERROR("test24/st_openfile (owner-read)", i);

	i = st_closefile(i);
	STERROR("test24/st_closefile", i);

	printf(" * open and close the file for write (by owner)\n");
	i = st_openfile(vol, TESTFILE, WRITE);
	STERROR("test24/st_openfile (owner-write)", i);

	i = st_closefile(i);
	STERROR("test24/st_closefile", i);

/* switch to another user */
sys_setuser(2000); 

	printf(" * open and close the file for read (by another user) - should be ok\n");
	i = st_openfile(vol, TESTFILE, READ);
	STERROR("test24/st_openfile (other-read)", i);

	i = st_closefile(i);
	STERROR("test24/st_closefile", i);

	printf(" * open and close the file for write (by another user)\n");
	i = st_openfile(vol, TESTFILE, WRITE);
	TERROR("test24/st_openfile (other-write)", i);

	printf(" * change file mode to other-writerable (not by owner) \n");
	i = st_chmod(vol, TESTFILE, 0606);
	TERROR("test24/st_chmod", i);

/* back to the owner */
sys_setuser(1000); 
	printf(" * change file mode to other-writerable by owner\n");
	i = st_chmod(vol, TESTFILE, 0606);
	STERROR("test24/st_chmod", i);

	printf(" * open and close the file for read (by owner)\n");
	i = st_openfile(vol, TESTFILE, READ);
	STERROR("test24/st_openfile (owner-read)", i);

	i = st_closefile(i);
	STERROR("test24/st_closefile", i);

	printf(" * open and close the file for write (by owner)\n");
	i = st_openfile(vol, TESTFILE, WRITE);
	STERROR("test24/st_openfile (owner-write)", i);

	i = st_closefile(i);
	STERROR("test24/st_closefile", i);


/* switch to another user */
sys_setuser(2000); 
	printf(" * open and close the file for read (by another user), should be ok\n");
	i = st_openfile(vol, TESTFILE, READ);
	STERROR("test24/st_openfile (other-read)", i);

	i = st_closefile(i);
	STERROR("test24/st_closefile", i);

	printf(" * open and close the file for write (by another user), should be ok\n");
	i = st_openfile(vol, TESTFILE, WRITE);
	STERROR("test24/st_openfile (other-write)", i);

	i = st_closefile(i);
	STERROR("test24/st_closefile", i);

/* back to the owner */
sys_setuser(1000); 

	printf(" * remove all permission by owner\n");
	i = st_chmod(vol, TESTFILE, 0);
	STERROR("test24/st_chmod", i);

	printf(" * open and close the file for read (by owner)\n");
	i = st_openfile(vol, TESTFILE, READ);
	TERROR("test24/st_openfile (owner-read)", i);

	printf(" * open and close the file for write (by owner)\n");
	i = st_openfile(vol, TESTFILE, WRITE);
	TERROR("test24/st_openfile (owner-write)", i);

sys_setuser(2000); 
	printf(" * open and close the file for read (not by owner)\n");
	i = st_openfile(vol, TESTFILE, READ);
	TERROR("test24/st_openfile (owner-read)", i);

	printf(" * open and close the file for write (not by owner)\n");
	i = st_openfile(vol, TESTFILE, WRITE);
	TERROR("test24/st_openfile (owner-write)", i);

/* back to the owner */
sys_setuser(1000); 
	printf(" * change to owner readable by owner\n");
	i = st_chmod(vol, TESTFILE, 0400);
	STERROR("test24/st_chmod", i);

	printf(" * open and close the file for read (by owner)\n");
	i = st_openfile(vol, TESTFILE, READ);
	STERROR("test24/st_openfile (owner-read)", i);

	i = st_closefile(i);
	STERROR("test24/st_closefile", i);

	printf(" * open and close the file for write (by owner)\n");
	i = st_openfile(vol, TESTFILE, WRITE);
	TERROR("test24/st_openfile (owner-write)", i);

	printf(" * destroy the file by owner\n");
	i = st_destroyfile(vol, TESTFILE);
	TERROR("test24/st_destroyfile", i);

	printf(" * change to owner writeable by owner\n");
	i = st_chmod(vol, TESTFILE, 0600);
	STERROR("test24/st_chmod", i);

	printf(" * destroy the file by owner\n");
	i = st_destroyfile(vol, TESTFILE);
	STERROR("test24/st_destroyfile", i);

	i = st_dismount("wiss2");  /* dismount the device wiss2 */
	STERROR("test24/st_dismount", i);

	printf(" that's it\n");


}
