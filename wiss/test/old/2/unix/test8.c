/* program to test opening and closing a file */
/* permisssion and concurrency control */

extern char *io_error(), *bf_error(), *st_error();

#define	IOERROR(p,c)	if((int)(c)<0) \
		{printf("%s %s\n", p, io_error((int)(c)));io_final(); exit(-1);}
#define	BFERROR(p,c)	if((int)(c)<0) \
		{printf("%s %s\n", p, bf_error((int)(c)));bf_final(); io_final(); exit(-1);}

#define	ERROR(p,c)	if((int)(c)<0) printf("%s %s\n", p, st_error((int)(c)))
#define	STERROR(p,c)	if((int)(c)<0) {ERROR(p, c); st_final(); bf_final();\
				io_final();}



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
	IOERROR("test8/io_init", i);
	
	i = bf_init();		/* initialize level 1 */
	BFERROR("test8/bf_init", i);

	i = st_init();		/* initialize level 2 */
	STERROR("test8/st_init", i);

	vol = st_mount("wiss2");  /* mount the device wiss2 */
	STERROR("test8/st_mount", vol);

	i = st_createfile(vol, "tfile1", 9, 90,90);
	STERROR("test8/st_createfile", i);

	i = st_createfile(vol, "tfile2", 9, 90,90);
	STERROR("test8/st_createfile", i);

/* testing openfile and closefile */

	ofn = st_openfile(vol, "tfile1", READ);
	STERROR("test8/st_openfile", ofn);

	i = st_closefile(ofn);
	STERROR("test8/st_closefile", i);
	
	ofn = st_openfile(vol, "tfile1", READ);
	STERROR("test8/st_openfile", ofn);

	i = st_closefile(ofn);
	STERROR("test8/st_closefile", i);
	
	ofn = st_openfile(vol, "tfile1", READ);
	STERROR("test8/st_openfile", ofn);
	num2 = st_openfile(vol, "tfile2", READ);
	STERROR("test8/st_openfile", num2);
	num4 = st_openfile(vol, "tfile2", READ);
	STERROR("test8/st_openfile", num4);
	ofn1 = st_openfile(vol, "tfile1", READ);
	STERROR("test8/st_openfile", ofn1);
	ofn2 = st_openfile(vol, "tfile1", READ);
	STERROR("test8/st_openfile", ofn2);
	num = st_openfile(vol, "tfile2", READ);
	STERROR("test8/st_openfile", num);
	ofn3 = st_openfile(vol, "tfile1", READ);
	STERROR("test8/st_openfile", ofn3);
	num1 = st_openfile(vol, "tfile2", READ);
	STERROR("test8/st_openfile", num1);
	ofn4 = st_openfile(vol, "tfile1", READ);
	STERROR("test8/st_openfile", ofn4);
	num3 = st_openfile(vol, "tfile2", READ);
	STERROR("test8/st_openfile", num3);

	printf(" file numbers for the first file are %d, %d, %d, %d, %d\n", 
		ofn, ofn1, ofn2, ofn3, ofn4);
	printf(" file numbers for the second file are %d, %d, %d, %d, %d\n", 
		num, num1, num2, num3, num4);

	i = st_closefile(ofn4);
	STERROR("test8/st_closefile1", i);
	i = st_closefile(num1);
	STERROR("test8/st_closefile4", i);
	i = st_closefile(num4);
	STERROR("test8/st_closefile1", i);
	i = st_closefile(ofn3);
	STERROR("test8/st_closefile2", i);
	i = st_closefile(num2);
	STERROR("test8/st_closefile3", i);
	i = st_closefile(ofn2);
	STERROR("test8/st_closefile3", i);
	i = st_closefile(ofn1);
	STERROR("test8/st_closefile4", i);
	i = st_closefile(num3);
	STERROR("test8/st_closefile2", i);
	i = st_closefile(ofn);
	STERROR("test8/st_closefile5", i);
	i = st_closefile(num);
	STERROR("test8/st_closefile5", i);

	i = st_destroyfile(vol, "tfile1");
	STERROR("test8/st_destroyfile", i);

	i = st_destroyfile(vol, "tfile2");
	STERROR("test8/st_destroyfile", i);

	i = st_dismount("wiss2");  /* dismount the device wiss2 */
	STERROR("test8/st_dismount", i);
}
