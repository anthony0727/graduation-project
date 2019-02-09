
/* program to test sort */

#include <wiss.h>

#define SHORT		20
#define LONG		1000
#define	IOERROR(p,c)	if((int)(c)<0) \
		{printf("%s %s\n", p, io_error(c));io_final(); exit(-1);}
#define	BFERROR(p,c)	if((int)(c)<0) \
	{printf("%s %s\n",p,bf_error(c));bf_final(); io_final(); exit(-1);}
#define	ERROR(p,c)	if((int)(c)<0) printf("%s %s\n", p, st_error((int)(c)))
#define	STERROR(p,c)	if((int)(c)<0) {ERROR(p, c); st_final(); bf_final();\
				io_final();}


struct {
	int sid;
	char sname[22];
	char ssex[2];
	int sage;
	int syr;
	float sgpa;} student;

struct{
	char iname[16]; 
	char idname[22]; } inst;

struct{
	int cnum;
	char cname[32];
	char cdname[22];} course;

struct{
	char dname[22];} dept;

char buff[150],*buffpt;
 float fval;


main()
{
	int i, j, k , vol, ofn;
	RID rid[10];	
	int rl;		/* record length for reading in data files */
	int tid;
	char ct;
	char it;

	ct = 'c';
	it = 'i';


/* initialization */

/*
	wiss_checkflags(&argc,&argv);
*/

	i = io_init();			/* initialize level 0 */
	IOERROR("test2/io_init", i);
	
	i = bf_init();		/* initialize level 1 */
	BFERROR("test2/bf_init", i);

	i = st_init();			/* initialize level 2 */
	STERROR("test2/st_init", i);

	vol = st_mount("wiss2");  	/* mount the device wiss2 */
	STERROR("test2/st_mount", vol);

/* testing */

	i = st_createfile(vol, "dept", 2, 90,90);
	STERROR("test2/st_createfile", i);

	ofn = st_openfile(vol, "dept", WRITE);
	STERROR("test2/st_openfile(for insertion)", ofn);

	k=open("/usr4/wiss/wiss/test/2/btree/studentdata/dept",0);
	if (k<0) {
		printf("\ncan not open dept data file");
		return;
	}
	printf("\n\nstoring the dept records");
	rl=23;
	while (read(k,buff,rl)) {
		buffpt=buff;
		gettext(dept.dname,22);
		printf("\n%s",dept.dname);
		j = st_insertrecord(ofn, dept, SHORT, NULL, &rid[i]);
		STERROR("test2/st_insertrecord", j);
	}
	close(k);
	i = st_closefile(ofn);
	STERROR("test2/st_closefile(for insertion)", i);

	/* store the instructor records in the database		*/

	printf("CREATING INST FILE\n\n");

	i = st_createfile(vol, "inst", 2, 90,90);
	STERROR("test2/st_createfile", i);

	ofn = st_openfile(vol, "dept", WRITE);
	STERROR("test2/st_openfile(for insertion)", ofn);

	printf("\nrcreate inst = %1d\n",rcreate("inst",(sizeof inst)));
/*
	printf("sort inst (offset=0) = %1d\n",rsort("inst",ct,16,0));
*/
	tid = ropen("inst");
	if (tid== -1) printf("\n inst OPEN ERROR\n");

	k=open("/usr4/wiss/wiss/test/2/btree/studentdata/inst",0);
	if (k<0) {
		printf("\ncan not open instructor data file");
		return;
	}
	printf("\n\nstoring the instructor records");
	rl=39;
	while (read(k,buff,rl)) {
		buffpt=buff;
		gettext(inst.iname,16);
		gettext(inst.idname,22);
		printf("\n%s,%s",inst.iname,inst.idname);
		i = insert("inst",&inst);
		if (i<0) printf("\nINSERT ERROR\n");
	}
	close(k);
	tid = rclose("inst");
	if (tid== -1) printf("\n inst CLOSE ERROR\n");

	/* store the course records in the database */

	printf("\n\n");
	printf("\nrcreate course = %1d\n",rcreate("course",(sizeof course)));
	tid = ropen("course");
	if (tid== -1) printf("\n course OPEN ERROR\n");

	printf("rsort course (offset = 0) = %1d\n", rsort("course",it,2,0));
	k=open("/usr4/wiss/wiss/test/2/btree/studentdata/course",0);
	if (k<0) {
		printf("\ncan not open course data file");
		return;
	}
	printf("\n\nstoring the course records");
	rl=58;
	while (read(k,buff,rl)) {
		buffpt=buff;
		course.cnum=getint(3);
		gettext(course.cname,32);
		gettext(course.cdname,22);
		printf("\n%d,%s,%s",course.cnum,course.cname,course.cdname);
		i = insert("course",&course);
		if (i<0) printf("\nINSERT ERROR\n");
	}
	close(k);
	tid = rclose("course");
	if (tid== -1) printf("\n course CLOSE ERROR\n");

	/* store the student tuples in the database */

	printf("\n\n");
	printf("\nrcreate student = %1d\n",rcreate("student",(sizeof student)));
	tid = ropen("student");
	if (tid== -1) printf("\n student OPEN ERROR\n");

	k=open("/usr4/wiss/wiss/test/2/btree/studentdata/student",0);
	if (k<0) {
		printf("\ncan not open student data file");
		return;
	}
	rl=34;
	while (read(k,buff,rl)) {
		buffpt=buff;
		student.sid=getint(3);
		gettext(student.sname,20);
		gettext(student.ssex,1);
		buffpt++;
		student.sage=getint(2);
		buffpt++;
		student.syr=getint(1);
		buffpt++;
		getreal();
		student.sgpa=fval;
		printf("\n%d,%s,%s,%d,%d,%f",student.sid,student.sname,
		student.ssex,student.sage,student.syr,student.sgpa);
		i = insert("student",&student);
		if (i<0) printf("\nINSERT ERROR\n");
	}
	close(k);
	tid = rclose("student");
	if (tid== -1) printf("\n student CLOSE ERROR\n");
	printf("rsort student (offset = 0) = %1d\n", rsort("student",it,2,0));
	printf("\n\n");

/*
	i = st_destroyfile(vol, FILENAME);
	STERROR("test2/st_destroyfile", i);
*/

	i = st_dismount("wiss2");  /* dismount the device wiss2 */
	STERROR("test2/st_dismount", i);


}


gettext(ptr,len)
	char *ptr; int len; {
		int i;
		for (i=0;i<len;i++) *ptr++ = *buffpt++;
		if (len == 01)
			*ptr = '\0';
		else {
			ptr--;
			*ptr-- = '\0';
			while (*ptr == ' ')
				*ptr-- = '\0';
		}
}

getreal() {
	char c;
	float ten;
	int n,p;
	ten = 10;
	c = *buffpt++;
	n = c - '0';
	buffpt++;
	c = *buffpt++;
	p = c - '0';
	fval = n + p/ten;
}



getint(len) int len; {
	int m,n,o;
	char c;
	n = 0;
	for (m=0;m<len;m++) {
		c = *buffpt++;
		if ((c >= '0') && (c <= '9'))
			n = n * 10 + (c-'0');
	}
	return(n);
}

strcopy(s,d)
char *s,*d;
{
	strcpy(d,s);
}
