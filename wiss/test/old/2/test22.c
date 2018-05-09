/* program to test long data item routines */

#include <wiss.h>
#include <st.h>

#define	BUFTRACE	1

extern char *io_error(), *bf_error(), *st_error();

#define	IOERROR(p,c)	if((int)(c)<0) \
	{printf("%s %s\n",p,io_error((int)(c)));io_final(); exit(-1);}
#define	BFERROR(p,c)	if((int)(c)<0) \
	{printf("%s %s\n",p,bf_error((int)(c)));bf_final();io_final();exit(-1);}
#define	ERROR(p,c)	if((int)(c)<0) printf("%s %s\n", p, st_error((int)(c)))
#define	STERROR(p,c)	if((int)(c)<0) {ERROR(p, c); st_final(); bf_final();\
			io_final();printall(ofn, &lrid);exit(-1);}

#define FILENAME	"tfile22"
#define	PFF		75
#define	CZ		917
#define	MAXCLU		9
#define	MAXVALUE	26
#define	MAXOFF		100000
#define	MAXACTION	10
#define	MAXLEN		(MAXOFF+MAXCLU)
#define	NUMTESTS	200

int	tsize;
char	seeds[MAXLEN];
char	buf[CZ*MAXCLU];
int	num_insert, num_delete, num_read, num_write, num_comp;

main(argc,argv)
int	argc;
char	**argv;
{
	register int 	i, j, k;
	int	e;
	int	vol, ofn;
	RID 	lrid;	
	register char	seed;
	int	action, off, size;

/* initialization */

	printf(" Initializing ... \n");
	wiss_checkflags(&argc,&argv);
	e = io_init();			/* initialize level 0 */
	IOERROR("test22/io_init", e);
	e = bf_init();		/* initialize level 1 */
	BFERROR("test22/bf_init", e);
	e = st_init();			/* initialize level 2 */
	STERROR("test22/st_init", e);
	vol = st_mount("wiss2");  	/* mount the device wiss2 */
	STERROR("test22/st_mount", vol);
	e = st_createfile(vol, FILENAME, 9, PFF,PFF);
	STERROR("test22/st_createfile", e);
	ofn = st_openfile(vol, FILENAME, WRITE);
	STERROR("test22/st_openfile(for insertion)", ofn);
	e = st_createlong(ofn, &lrid);
	STERROR("test22/st_createlong", e);
#ifdef	BUFTRACE
	BF_dumpfixed();
#endif

/* start testing */
	tsize = 0;
	num_insert = num_delete = num_read = num_write = num_comp = 0;
	for (i = 0; i < NUMTESTS; i++) {
		/* randomly decide what to do next */
		action = random() % MAXACTION;
		off = random() % (tsize+1);
		size = random() % MAXCLU + 1;
		seed = 'a' + random() % MAXVALUE;
/*
printf("loop %d: action %d, off=%d, size=%d, seed=%c\n",i,action,off,size,seed);
*/

		switch (action) {
		case 0:
		case 1:
		case 2:
		case 3:
			if (tsize+size > MAXLEN) size = MAXLEN-tsize;
			if (size <= 0) { i--; break; /* over again */}
			bcopy(&seeds[off], &seeds[off+size], tsize-off);
			for (j = 0; j < size; j++) seeds[j+off] = seed;
			tsize += size;
			for (j = 0; j < size * CZ; j++) buf[j] = seed;
			e = st_insertframe(ofn,&lrid,off*CZ,buf,size*CZ);
			STERROR("test22/st_insertframe", e);
			num_insert++;
			break;
		case 4:
		case 5:
			if (off+size > tsize) size = tsize - off;
			if (size <= 0) { i--; break; /* over again */}
			e = st_readframe(ofn,&lrid,off*CZ,buf,size*CZ);
			STERROR("test22/st_readframe", e);
			for (j = 0; j < size; j++, off++) {
				seed = seeds[off];
				for (k = 0; k < CZ; k++)
				    if (buf[j*CZ+k] != seed) {
					printf(" ERROR detected at %d:%d \n",
						j+off, k);
					printf("should be %c, but %c read\n",
						seed, buf[j*CZ+k]);
					printall(ofn, &lrid);
					exit(-1);
			    	    }
			}
			num_read++;
			break;
		case 6:
		case 7:
			if (off+size > tsize) size = tsize - off;
			if (size <= 0) { i--; break; /* over again */}
			for (j = 0; j < size ; j++) seeds[j+off] = seed;
			for (j = 0; j < size * CZ; j++) buf[j] = seed;
			e = st_writeframe(ofn,&lrid,off*CZ,buf,size*CZ);
			STERROR("test22/st_writeframe", e);
			num_write++;
			break;
		case 8:
			if (off+size > tsize) size = tsize - off;
			if (size <= 0) { i--; break; /* over again */}
			bcopy(&seeds[off+size], &seeds[off], tsize-off-size);
			tsize -= size;
			e = st_deleteframe(ofn, &lrid,  off*CZ, size*CZ);
			STERROR("test22/st_deleteframe", e);
			num_delete++;
			break;
		case 9: 
			if (random()%10) { i--; break; /* over again */}
			e = st_compresslong(ofn, &lrid);
			STERROR("test22/st_compresslong", e);
			num_comp++;
			break;
		}
		/* total check once every 10 times */
#ifdef	BUFTRACE
	e = BF_dumpfixed();
	if (e != 0) printf("loop %d: action %d, off=%d, size=%d, seed=%c\n",
		i,action,off,size,seed);
#endif
		if (i % 10 == 9) {
		    for (j = 0; j < tsize; j++) {
			e = st_readframe(ofn,&lrid,j*CZ,buf,CZ);
			STERROR("test22/st_readframe", e);
			for (k = 0; k < CZ; k++)
			    	if (buf[k] != seeds[j]) {
					printf(" ERROR detected at %d:%d \n",
						j, k);
					printf("should be %c, but %c read\n",
						seeds[j], buf[k]);
					printall(ofn, &lrid);
					exit(-1);
			    }
		    }
		}
	}

/* Clean up the mess */
	printf(" Cleaning up ...\n");
	e = st_destroylong(ofn, &lrid);
	STERROR("test22/st_destroylong", e);
	e = st_closefile(ofn);
	STERROR("test22/st_closefile(for destroy_long", e);
	e = st_destroyfile(vol, FILENAME);
	STERROR("test22/st_destroyfile", e);
	e = st_dismount("wiss2");  /* dismount the device wiss2 */
	STERROR("test22/st_dismount", e);
	printf("That's all folks\n");
	printf(" %d inserts, %d deletes, %d reads, %d writes %d compresses\n",
		num_insert, num_delete, num_read, num_write, num_comp);
	printf(" the recorded long data item has %d units\n", tsize);
}

printall(ofn, ridptr)
int	ofn;
RID	*ridptr;
{
	int	i;
	printf(" %d inserts, %d deletes, %d reads, %d writes %d compresses\n",
		num_insert, num_delete, num_read, num_write, num_comp);
	printf(" RID of the directory is %d:%d:%d\n", 
		ridptr->Rvolid, ridptr->Rpage, ridptr->Rslot);
	r_dumplong(ofn, ridptr);
	printf(" the recorded long data item has %d units\n", tsize);
	for (i = 0; i < tsize; i++) 
		if (i % 70 == 69) printf("%c\n", seeds[i]);
		else printf("%c", seeds[i]);
	printf("\n");
}
