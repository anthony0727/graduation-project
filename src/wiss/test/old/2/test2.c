/* program to test mounting and dismounting a device 
   (1) repeatly mounting and dismounting a device
   (2) dismounting an unmounted device
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
	IOERROR("test2/io_init", i);
	
	i = bf_init();		/* initialize level 1 */
	BFERROR("test2/bf_init", i);

	i = st_init();			/* initialize level 2 */
	STERROR("test2/st_init", i);

/* first test */

	printf("\n (1) About to mount wiss2 11 times\n");
	for (j = 0; j < 11; j++)
	{
		i = st_mount("wiss2");  /* mount the device wiss2 */
		if (i < 0)
			printf("fails after mounting wiss2 for %d times\n", j);
		STERROR("test2/st_mount", i);
		i = st_dismount("wiss2");  /* dismount the device wiss2 */
		STERROR("test2/st_dismount", i);
	}

	printf("successfully mount and dismount %d times\n", j);

}
