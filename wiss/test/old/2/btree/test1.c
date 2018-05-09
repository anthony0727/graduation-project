
#define     IOERROR(p,c)     if((int)(c)<0) \
    {printf("%s %s\n", p, io_error(c));io_final(); exit(-1);}
#define     BFERROR(p,c)     if((int)(c)<0) \
    {printf("%s %s\n",p,bf_error(c));bf_final(); io_final(); exit(-1);}
#define     ERROR(p,c) \
    if((int)(c)<0) printf("%s %s\n", p, st_error((int)(c)))
#define     STERROR(p,c) \
    if((int)(c)<0) {ERROR(p, c); st_final(); bf_final(); io_final();}

#define          NUMREC     10000
#define          RECSIZE     200
#define          TESTFILE     "test1"

#include     <wiss.h>
extern     Trace1;

main(argc, argv)
int     argc;     
char     **argv;
{
     int     i, j, k;
     int     e;
     int     vol;               /* volume id */
     int     bufsize;
     short     record[NUMREC];
     RID     rid;
     KEYINFO     keyattr;
     char     buf[RECSIZE];
     int     bool[NUMREC];

moncontrol(0);

     e = io_init();               /* initialize level 0 */
     IOERROR("test1/io_init", e);
     
     e = bf_init();          /* initialize level 1 */
     BFERROR("test1/bf_init", e);

     e = st_init();               /* initialize level 2 */
     STERROR("test1/st_init", e);

     /* mount the volume call "wiss2" */
     vol = st_mount("wiss2");     
     STERROR("test6/st_mount", vol);

     if (argc >= 2) 
          bufsize = atoi(argv[1]);
     else bufsize = 25;
     set_sortbuf(bufsize);     
     printf(" sort buffer %d\n", bufsize);

     printf(" creating an empty data file %s\n", TESTFILE);
     e = st_createfile(vol, TESTFILE, 6, 90,90);
     STERROR("test1/st_createfile", e);

     j = st_openfile(vol, TESTFILE, WRITE);
     STERROR("test1/st_openfile", j);

     for (i = 0; i < NUMREC; i++)
          bool[i] = 0;
          
     for (i = 0; i < NUMREC; i++)
     {
          record[i] = rand() % NUMREC;
          if (bool[record[i]] == 1)
          {
               for (k = record[i]; bool[k] == 1; 
                    k = (k == NUMREC - 1) ? 0 : k + 1);
               record[i] = k;
          }
          bool[record[i]] = 1;     /* avoid duplicates */


          printf("record[%3d] = %3d, ", i, record[i]);
          if (i % 4 == 3) printf("\n");


          *(short *) buf = record[i];
          e = st_insertrecord(j, buf, RECSIZE, NULL, &rid);
          STERROR("test1/st_insertrecord", e);
          if (e < eNOERROR) break;
     }

     e = st_closefile(j);
     STERROR("test1/st_closefile", e);

     j = st_openfile(vol, TESTFILE, WRITE);
     STERROR("test1/st_openfile", j);


     e = st_firstfile(j, &rid);
     STERROR("test1/st_firstfile", e);

     for ( i = 0; e >= eNOERROR; i++ )
     {
          e = st_readrecord(j, &rid, &record[0], sizeof(short));
          STERROR("test1/st_readrecord", e);
          

          printf("record[%3d] = %3d, ", i, record[0]);
          if (i % 4 == 3) printf("\n");


          e = st_nextfile(j, &rid, &rid);
          if (e < eNOERROR) break;
     }

     e = st_closefile(j);
     STERROR("test1/st_closefile", e);

     keyattr.offset = 0;
     keyattr.type = TINTEGER;
     keyattr.length = sizeof(int); 

     e = st_sort(vol, TESTFILE, &keyattr, 1);
     STERROR("test1/st_sort", e);

     printf(" after sorting : \n");

     j = st_openfile(vol, TESTFILE, WRITE);
     STERROR("test1/st_openfile", j);

     e = st_firstfile(j, &rid);
     STERROR("test1/st_firstfile", e);

     for ( i = 0; e >= eNOERROR; i++ )
     {
          e = st_readrecord(j, &rid, &record[i], sizeof(short));
          STERROR("test1/st_readrecord", e);
          

          printf("record[%3d] = %3d, ", i, record[i]);
          if (i % 4 == 3) printf("\n");


          if (record[i] != i)
               printf(" BUG in sort!!\n");

          e = st_nextfile(j, &rid, &rid);
          if (e < eNOERROR) break;
     }

     e = st_closefile(j);
     STERROR("test1/st_closefile", e);
     
     /* remove the file since the test in done */
     e = st_destroyfile(vol, TESTFILE);
     STERROR("test1/st_destroyfile", e);

     /* dismount the volume call "wiss2" */
     e = st_dismount("wiss2");  
     STERROR("test1/st_dismount", e);

     printf(" All is well ! That's it!\n");

}
