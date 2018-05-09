/* program to test remounting a mounted device
*/

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
	int i, j;

	wiss_checkflags(&argc,&argv);
	i = io_init();			/* initialize level 0 */
	IOERROR("test3/io_init", i);
	
	i = bf_initialize();		/* initialize level 1 */
	BFERROR("test3/bf_initialize", i);

	i = st_init();			/* initialize level 2 */
	STERROR("test3/st_init", i);

	printf(" mouting the device wiss2 for the first time\n");
	i = st_mount("wiss2");  /* mount the device wiss2 */
	STERROR("test3/st_mount", i);

	printf(" remouting the device wiss2 without dismounting\n");
	i = st_mount("wiss2");  /* mount the device wiss2 */
	printf(" should get fatal error messages :\n");
	STERROR("test3/st_mount", i);
	printf(" st_mount ERROR!! - did not catch the remounting error");

}
