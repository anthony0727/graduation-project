/* program to test mounting and dismounting a device */

extern char *io_error(), *bf_error(), *st_error();

#define	IOERROR(p,c)	if((int)(c)<0) \
		{printf("%s %s\n", p, io_error((int)(c)));io_final(); exit(-1);}
#define	BFERROR(p,c)	if((int)(c)<0) \
		{printf("%s %s\n", p, bf_error((int)(c)));bf_final(); io_final(); exit(-1);}

#define	ERROR(p,c)	if((int)(c)<0) printf("%s %s\n", p, st_error((int)(c)))
#define	STERROR(p,c)	if((int)(c)<0) {ERROR(p, c); st_final(); bf_final();\
				io_final();}


main(argc,argv)
int	argc;
char	**argv;
{
	int i;

	wiss_checkflags(&argc,&argv);
	i = io_init();			/* initialize level 0 */
	IOERROR("test1/io_init", i);
	
	i = bf_init();		/* initialize level 1 */
	BFERROR("test1/bf_init", i);

	i = st_init();		/* initialize level 2 */
	STERROR("test1/st_init", i);

	printf("\n About to mount wiss2\n");
	i = st_mount("wiss2");  /* mount the device wiss2 */
	STERROR("test1/st_mount", i);

	printf("\n About to dismount wiss2\n");
	i = st_dismount("wiss2");  /* dismount the device wiss2 */
	STERROR("test1/st_dismount", i);
}
