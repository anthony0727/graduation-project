/* program to test opening and closing a file */
/* permisssion and concurrency control */
/* FOR DEBUGGING VERSION OF UTIL/SYS.C ONLY */
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


#include <wiss.h>

main(argc,argv)
int	argc;
char	**argv;
{
	int 	i, vol;
	int	ofn, ofn1, ofn2, ofn3, ofn4;
	int	num, num1, num2, num3, num4;

	wiss_checkflags(&argc,&argv);
	i = io_init();			/* initialize level 0 */
	IOERROR("test25/io_init", i);
	
	i = bf_init();		/* initialize level 1 */
	BFERROR("test25/bf_init", i);

	i = st_init();		/* initialize level 2 */
	STERROR("test25/st_init", i);

	vol = st_mount("wiss2");  /* mount the device wiss2 */
	STERROR("test25/st_mount", vol);

/* user 1 */
sys_setuser(1000); 
	i = st_createfile(vol, "tfile1", 9, 90,90);
	STERROR("test25/st_createfile", i);

	i = st_chmod(vol, "tfile1", 0606);
	STERROR("test24/st_chmod", i);

/* user 2 */
sys_setuser(2000); 
	i = st_createfile(vol, "tfile2", 9, 90,90);
	STERROR("test25/st_createfile", i);

	i = st_chmod(vol, "tfile2", 0606);
	STERROR("test24/st_chmod", i);

/* testing openfile and closefile */


sys_setuser(1000); 
	ofn = st_openfile(vol, "tfile1", READ);
	STERROR("test25/st_openfile", ofn);
sys_setuser(2000); 
	num2 = st_openfile(vol, "tfile2", READ);
	STERROR("test25/st_openfile", num2);
	num4 = st_openfile(vol, "tfile2", READ);
	STERROR("test25/st_openfile", num4);
sys_setuser(1000); 
	ofn1 = st_openfile(vol, "tfile1", READ);
	STERROR("test25/st_openfile", ofn1);
	ofn2 = st_openfile(vol, "tfile1", READ);
	STERROR("test25/st_openfile", ofn2);
sys_setuser(2000); 
	num = st_openfile(vol, "tfile2", READ);
	STERROR("test25/st_openfile", num);
sys_setuser(1000); 
	ofn3 = st_openfile(vol, "tfile1", READ);
	STERROR("test25/st_openfile", ofn3);
sys_setuser(2000); 
	num1 = st_openfile(vol, "tfile2", READ);
	STERROR("test25/st_openfile", num1);
sys_setuser(1000); 
	ofn4 = st_openfile(vol, "tfile1", READ);
	STERROR("test25/st_openfile", ofn4);
sys_setuser(2000); 
	num3 = st_openfile(vol, "tfile2", READ);
	STERROR("test25/st_openfile", num3);

	printf(" file numbers for the first user are %d, %d, %d, %d, %d\n", 
		ofn, ofn1, ofn2, ofn3, ofn4);
	printf(" file numbers for the second user are %d, %d, %d, %d, %d\n", 
		num, num1, num2, num3, num4);


sys_setuser(1000); 
	i = st_closefile(ofn4);
	STERROR("test25/st_closefile1", i);
sys_setuser(2000); 
	i = st_closefile(num1);
	STERROR("test25/st_closefile4", i);
	i = st_closefile(num4);
	STERROR("test25/st_closefile1", i);
sys_setuser(1000); 
	i = st_closefile(ofn3);
	STERROR("test25/st_closefile2", i);
sys_setuser(2000); 
	i = st_closefile(num2);
	STERROR("test25/st_closefile3", i);
sys_setuser(1000); 
	i = st_closefile(ofn2);
	STERROR("test25/st_closefile3", i);
	i = st_closefile(ofn1);
	STERROR("test25/st_closefile4", i);
sys_setuser(2000); 
	i = st_closefile(num3);
	STERROR("test25/st_closefile2", i);
sys_setuser(1000); 
	i = st_closefile(ofn);
	STERROR("test25/st_closefile5", i);
sys_setuser(2000); 
	i = st_closefile(num);
	STERROR("test25/st_closefile5", i);


printf(" testing mixed READs & WRITEs by the same owner\n");
sys_setuser(1000); 
	ofn = st_openfile(vol, "tfile1", READ);
	STERROR("test25/st_openfile", ofn);
sys_setuser(2000); 
	num2 = st_openfile(vol, "tfile2", WRITE);
	STERROR("test25/st_openfile", num2);
	num4 = st_openfile(vol, "tfile2", READ);
	STERROR("test25/st_openfile", num4);
sys_setuser(1000); 
	ofn1 = st_openfile(vol, "tfile1", READ);
	STERROR("test25/st_openfile", ofn1);
	ofn2 = st_openfile(vol, "tfile1", WRITE);
	STERROR("test25/st_openfile", ofn2);
sys_setuser(2000); 
	num = st_openfile(vol, "tfile2", READ);
	STERROR("test25/st_openfile", num);
sys_setuser(1000); 
	ofn3 = st_openfile(vol, "tfile1", WRITE);
	STERROR("test25/st_openfile", ofn3);
sys_setuser(2000); 
	num1 = st_openfile(vol, "tfile2", WRITE);
	STERROR("test25/st_openfile", num1);
sys_setuser(1000); 
	ofn4 = st_openfile(vol, "tfile1", READ);
	STERROR("test25/st_openfile", ofn4);
sys_setuser(2000); 
	num3 = st_openfile(vol, "tfile2", READ);
	STERROR("test25/st_openfile", num3);

	printf(" file numbers for the first user are %d, %d, %d, %d, %d\n", 
		ofn, ofn1, ofn2, ofn3, ofn4);
	printf(" file numbers for the second user are %d, %d, %d, %d, %d\n", 
		num, num1, num2, num3, num4);


sys_setuser(1000); 
	i = st_closefile(ofn4);
	STERROR("test25/st_closefile1", i);
sys_setuser(2000); 
	i = st_closefile(num1);
	STERROR("test25/st_closefile4", i);
	i = st_closefile(num4);
	STERROR("test25/st_closefile1", i);
sys_setuser(1000); 
	i = st_closefile(ofn3);
	STERROR("test25/st_closefile2", i);
sys_setuser(2000); 
	i = st_closefile(num2);
	STERROR("test25/st_closefile3", i);
sys_setuser(1000); 
	i = st_closefile(ofn2);
	STERROR("test25/st_closefile3", i);
	i = st_closefile(ofn1);
	STERROR("test25/st_closefile4", i);
sys_setuser(2000); 
	i = st_closefile(num3);
	STERROR("test25/st_closefile2", i);
sys_setuser(1000); 
	i = st_closefile(ofn);
	STERROR("test25/st_closefile5", i);
sys_setuser(2000); 
	i = st_closefile(num);
	STERROR("test25/st_closefile5", i);

	printf("testing conflict detection\n");

sys_setuser(1000); 
	ofn = st_openfile(vol, "tfile1", READ);
	STERROR("test25/st_openfile", ofn);
	ofn1 = st_openfile(vol, "tfile1", READ);
	STERROR("test25/st_openfile", ofn1);
sys_setuser(2000); 
	num = st_openfile(vol, "tfile1", READ);	/* should be ok */
	STERROR("test25/st_openfile", num);
	i = st_closefile(num);
	STERROR("test25/st_closefile5", i);
	num = st_openfile(vol, "tfile1", WRITE);	/* should be an error*/
	TERROR("test25/st_openfile", num);
sys_setuser(1000); 
	i = st_closefile(ofn1);
	STERROR("test25/st_closefile4", i);
	i = st_closefile(ofn);
	STERROR("test25/st_closefile5", i);

sys_setuser(2000); 
	num = st_openfile(vol, "tfile2", WRITE);
	STERROR("test25/st_openfile", num);
	num1 = st_openfile(vol, "tfile2", READ);
	STERROR("test25/st_openfile", num1);
sys_setuser(1000); 
	ofn = st_openfile(vol, "tfile2", READ);
	TERROR("test25/st_openfile", ofn);
	ofn1 = st_openfile(vol, "tfile2", WRITE);
	TERROR("test25/st_openfile", ofn1);
sys_setuser(2000); 
	i = st_closefile(num);
	STERROR("test25/st_closefile4", i);
	i = st_closefile(num1);
	STERROR("test25/st_closefile4", i);


	i = st_destroyfile(vol, "tfile1");
	STERROR("test25/st_destroyfile", i);

	i = st_destroyfile(vol, "tfile2");
	STERROR("test25/st_destroyfile", i);

	i = st_dismount("wiss2");  /* dismount the device wiss2 */
	STERROR("test25/st_dismount", i);
}
