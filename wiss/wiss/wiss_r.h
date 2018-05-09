
/********************************************************/
/*                                                      */
/*               WiSS Storage System                    */
/*          Version SystemV-4.0, September 1990	        */
/*                                                      */
/*              COPYRIGHT (C) 1990                      */
/*                David J. DeWitt 		        */
/*               Madison, WI U.S.A.                     */
/*                                                      */
/*	         ALL RIGHTS RESERVED                    */
/*                                                      */
/********************************************************/


/*
The following functions have a special interface and have
no associated macro in this file:

wiss_checkflags(argcptr, argvptr)

*/

/* ERROR

���� :
���� �ڵ带 ����Ѵ�

�Լ� :
error :
errormsg :
fatalerror :

*/
#define wiss_error(routine, errorcode) 		am_error(routine, errorcode)
#define wiss_errormsg(errorcode)   			am_errormsg(errorcode)
#define wiss_fatalerror(routine, error)   	am_fatalerror(routine, error)
/* ERROR END */



/* VOLUMN

���� :
WiSS ������ �ٷ��

�Լ� :
createvolumn : ������ ����
destroyvolumn : ������ ����
mountvolumn : ������ ����Ʈ
dismountvolumn : ������ �𽺸���Ʈ

*/
#define	wiss_create_volume(devname, title, volid, numexts, extsize) 	am_create_volume(devname, title, volid, numexts, extsize)
#define	wiss_destroy_volume(devname) 									am_destroy_volume(devname)
#define	wiss_mount(devname) 											st_mount(devname)
#define	wiss_dismount(devname) 											st_dismount(devname)
/* VOLUMN END */



/* TRANSACTION

���� :
���ڵ带 �ٷ��

�Լ� :

*/
#define	wiss_begin_trans()  begin_trans()
#define	wiss_commit_trans(trans_id) commit_trans(trans_id)
#define	wiss_abort_trans(transId) abort_trans(transId)
/* TRANSACTION END */



/* FILE

���� :
������ �ٷ��

�Լ� :
createfile : ������ ����
destroyfile : ������ ����
openfile : ������ ����
closefile : ������ �ݱ�
rename : ������ �̸��� ����

*/
#define	wiss_createfile(VolID, FileName, NumPages, ExtentFillFactor, PageFillFactor)    st_createfile(VolID, FileName, NumPages, ExtentFillFactor, PageFillFactor)
#define	wiss_destroyfile(VolID, FileName, trans_id, lockup, cond) 						st_destroyfile(VolID, FileName, trans_id, lockup, cond)
#define	wiss_openfile(volid, filename, mode) 											st_openfile(volid, filename, mode)
#define	wiss_closefile(filenum)   														am_closefile(filenum)
#define wiss_rename(volid, newname, oldname, trans_id, lockup, cond) 							st_rename(volid, new, old, trans_id, lockup, cond)
/* FILE END */



/* FILE STATISTICS

���� :
�˰� ���� ���� ��ũ���͸� ã�Ƽ� ������ ������ �˾Ƴ���

�Լ� :
filepages : ������ ������ ������ ���� ��ȯ
indexpages : �ε��� ������ ������ ���� ��ȯ
hashpages : �ؽ� ������ ������ ���� ��ȯ
filepages : ������ ������ ���ڵ�(Ű) ���� ��ȯ
indexpages : �ε��� ������ ���ڵ�(Ű) ���� ��ȯ
hashpages : �ؽ� ������ ���ڵ�(Ű) ���� ��ȯ

*/
#define wiss_filepages(volid, filename) 			st_filepages(volid, filename)
#define wiss_indexpages(volid, filename, indexno) 	st_indexpages(volid, filename, indexno)
#define wiss_hashpages(volid, filename, indexno) 	st_hashpages(volid, filename, indexno)
#define wiss_recordcard(volid, filename) 			st_recordcard(volid, filename)
#define wiss_keycard(volid, filename, indexno)		st_keycard(volid, filename, indexno)
#define wiss_hashcard(volid, filename, indexno) 	st_hashcard(volid, filename, indexno)
/* FILE STATISTICS END */



/* INDEX

���� :
������ ���Ͽ� ���� �ε��� ������ �ٷ��

�Լ� :
createindex : ������ ���Ͽ� ���� BƮ�� �ε��� ������ ����
dropindex : �ε��� ������ ����
openindex : �ε��� ������ ����
insertindex : �ε��� ������ BƮ���� RID�� ����
deleteindex : �ε��� ������ BƮ������ RID�� ����
getindex : �ε��� ���Ͽ��� �ε��� Ű�� RID�� ����

*/
#define	wiss_createindex(VolID, FileName, IndexNo, KeyAttr, FillFactor, Unique, SortFile, trans_id, lockup, cond) 	st_createindex(VolID, FileName, IndexNo, KeyAttr, FillFactor, Unique,   SortFile, trans_id, lockup, cond)
#define	wiss_dropindex(volid, filename, indexno, trans_id, lockup, cond) 											st_dropbtree(volid, filename, indexno, trans_id, lockup, cond)
#define	wiss_openindex(volid, filename, indexno, accessmode) 														am_openindex(volid, filename, indexno, accessmode)
#define	wiss_insertindex(filenum, key, ridptr, trans_id, lockup, cond) 												st_insertindex(filenum, key, ridptr, trans_id, lockup, cond)
#define	wiss_deleteindex(filenum,key,ridptr, trans_id, lockup, cond) 												st_deleteindex(filenum,key,ridptr, trans_id, lockup, cond)
#define	wiss_getindex(filenum, SearchKey, Cursor, FirstRID, trans_id, lockup, oper, cond) 							st_getindex(filenum, SearchKey, Cursor, FirstRID, trans_id, lockup, oper, cond)
/* INDEX END */



/* HASH

���� :
������ ���Ͽ� ���� �ؽ� ������ �ٷ��

�Լ� :
createhash : ������ ���Ͽ� ���� �ؽ� ������ ����
destroyhash : �ؽ� ������ ����
openhash : �ؽ� ������ ����
inserthash : �ؽ� ������ �ؽ� Ű�� RID�� ����
deletehash : �ؽ� ������ �ؽ� Ű���� RID�� ����
gethash : �ؽ� ���Ͽ��� �ؽ� Ű�� ù��° RID�� ����
nexthash : �ؽ� ���Ͽ��� �ؽ� Ű�� ���� RID�� ����

*/
#define wiss_createhash(VolID, filename, hashno, KeyAttr, FillFactor, Unique, trans_id, lockup, cond) \  	st_createhash(VolID, filename, hashno, KeyAttr, FillFactor, Unique, trans_id, lockup, cond)
#define wiss_destroyhash(volid, filename, hashno, trans_id, lockup, cond) \  								st_destroyhash(volid, filename, hashno, trans_id, lockup, cond)
#define wiss_openhash(volid, filename, hashno, accessmode) \  												st_openhash(volid, filename, hashno, accessmode)
#define wiss_inserthash(filenum, key, h_rid, trans_id, lockup, cond) \  									st_inserthash(filenum, key, h_rid, trans_id, lockup, cond)
#define wiss_deletehash(filenum, key, h_rid, trans_id, lockup, cond) \  									st_deletehash(filenum, key, h_rid, trans_id, lockup, cond)
#define wiss_gethash(filenum, key, Cursor, h_rid, trans_id, lockup, cond) \  								st_gethash(filenum, key, Cursor, h_rid, trans_id, lockup, cond)
#define wiss_nexthash(filenum, Cursor, h_rid, trans_id, lockup, cond) \  									st_nexthash(filenum, Cursor, h_rid, trans_id, lockup, cond)
/* HASH END */


/* SCAN

���� :
���ǿ� �����ϴ� RID���� �� �� �ֵ��� �Ѵ�

�Լ� :
openfilescan : ������ ������ ��ĵ�� ����
setscan : ������ ���� ��ĵ�� ��ġ�� ����

openlongscan : ������ ������ LDI ��ĵ�� ����
setcursor : ������ ���� LDI ��ĵ�� ��ġ�� ����

openindexscan : �ε��� ������ ��ĵ�� ����
geticursor : �ε��� ���� ��ĵ�� ��ġ�� ����
setiscan : �ε��� ���� ��ĵ�� ��ġ�� ����

openhashscan : �ؽ� ������ ��ĵ�� ����

readscan : ���� ��ĵ ��ġ�� ���ڵ带 �б�
insertscan : ���� ��ĵ ��ġ ��ó�� ���ڵ带 ����
deletescan : ���� ��ĵ ��ġ�� ���ڵ带 ����
updatescan : ���� ��ĵ ��ġ�� ���ڵ带 ����
closescan : ��ĵ�� �ݱ�

*/
#define	wiss_openfilescan(openfilenum, booleanexpr, trans_id, lockup, lockmode, cond)										am_openfilescan(openfilenum, booleanexpr, trans_id, lockup, lockmode, cond)
#define	wiss_setscan(scanid, NewRID) 																						am_setscan(scanid, NewRID)

#define	wiss_openlongscan(openfilenum, rid, trans_id, lockup, lockmode, cond) 												am_openlongscan(openfilenum, rid, trans_id, lockup, lockmode, cond)
#define	wiss_setcursor(scanid, offset, relocation) 																			am_setcursor(scanid, offset, relocation)

#define	wiss_openindexscan(openfilenum, indexfilenum, indexkey, lb, ub, booleanexpr, trans_id, lockup, lockmode, cond)   	am_openindexscan(openfilenum, indexfilenum, indexkey, lb, ub, booleanexpr, trans_id, lockup, lockmode, cond)
#define	wiss_geticursor(scanid, Xcursor) 																					am_geticursor(scanid, Xcursor)
#define	wiss_setiscan(scanid, NewXCURSOR) 																					am_setindexscan(scanid, NewXCURSOR)

#define	wiss_openhashscan(openfilenum, hashfilenum, keyAttr, hashkey, booleanexpr, trans_id, lockup, lockmode, cond) 		am_openhashscan(openfilenum, hashfilenum, keyAttr, hashkey,booleanexpr, trans_id, lockup, lockmode, cond) 

#define	wiss_readscan(scanid, recaddr, len)																					am_readscan(scanid, recaddr, len)
#define	wiss_insertscan(scanid, recaddr, len, newrid) 																		am_insertscan(scanid, recaddr, len, newrid)
#define	wiss_deletescan(scanid)																								am_deletescan(scanid)
#define	wiss_updatescan(scanid, recadr, len) 																				am_updatescan(scanid, recadr, len)
#define	wiss_closescan(scanid)																								am_closescan(scanid)
/* SCAN END */



/* FETCH

���� :
������ ������ ���ڵ�� �� ������ �´� ���ڵ��� ��ġ�� ã�´�

�Լ� :
fetchfirst : ��ĵ ������ ó������ �����ϴ� ���ڵ带 Ž��
fetchlast : ��ĵ ������ ���������� �����ϴ� ���ڵ带 Ž��
fetchnext : ���� ��ġ���� ���� �������� ��ĵ ������ �����ϴ� ���ڵ带 Ž��
fetchprev : ���� ��ġ���� ���� �������� ��ĵ ������ �����ϴ� ���ڵ带 Ž��

*/
#define	wiss_fetchfirst(scanid, firstrid, type)		am_fetchfirst(scanid, firstrid, type)
#define	wiss_fetchlast(scanid, lastrid, type)		am_fetchlast(scanid, lastrid, type)
#define	wiss_fetchnext(scanid, nextrid, type)		am_fetchnext(scanid, nextrid, type)
#define	wiss_fetchprev(scanid, prevrid, type)		am_fetchprev(scanid, prevrid, type)
/* FETCH END */



/* FILE RECORD

���� :
���Ͽ��� RID�� ã�´�

�Լ� :
firstfile : ������ ó������ ��ȿ�� ���ڵ带 Ž��
nextfile : ������ ���� ��ġ ���ķ� ��ȿ�� ���ڵ带 Ž��
prevfile : ������ ���� ��ġ �������� ��ȿ�� ���ڵ带 Ž��
lastfile : ���Ͽ��� ���������� ��ȿ�� ���ڵ带 Ž��

*/
#define wiss_firstfile(filenum, firstrid, trans_id, lockup, mode, cond) 		st_firstfile(filenum, firstrid, trans_id, lockup, mode, cond) 
#define wiss_nextfile(filenum, ridptr, nextrid, trans_id, lockup, mode, cond) 	st_nextfile(filenum, ridptr, nextrid, trans_id, lockup, mode, cond) 
#define wiss_prevfile(filenum, ridptr, prevrid, trans_id, lockup, mode, cond) 	st_prevfile(filenum, ridptr, prevrid, trans_id, lockup, mode, cond)
#define wiss_lastfile(filenum, lastrid, trans_id, lockup, mode, cond)			st_lastfile(filenum, lastrid, trans_id, lockup, mode, cond)
/* FILE RECORD END */



/* LONG DATA ITEM

���� :
Long Data Item�� �ٷ��

�Լ� :
createlong : LDI�� ����
destroylong : LDI�� ����
readlong : LDI�� �����͸� ����
insertlong : LDI�� �����͸� �߰�
deletelong : LDI�� �����͸� ����
updatelong : LDI�� �����͸� ����

*/
#define	wiss_createlong(filenum, ridptr, trans_id, lockup, cond) 	st_createlong(filenum, ridptr, trans_id, lockup, cond)
#define	wiss_destroylong(filenum, ridptr, trans_id, lockup, cond) 	st_destroylong(filenum, ridptr, trans_id, lockup, cond)
#define	wiss_readlong(scanid, recaddr, length) 						am_readlong(scanid, recaddr, length)
#define	wiss_insertlong(scanid, where, recaddr, length) 			am_insertlong(scanid, where, recaddr, length)
#define	wiss_deletelong(scanid,length)								am_deletelong(scanid,length)
#define	wiss_updatelong(scanid, recaddr, length) 					am_updatelong(scanid, recaddr, length)
/* LONG DATA ITEM END */



/* RECORD

���� :
���ڵ带 �ٷ��

�Լ� :
insertrecord : ���ڵ带 ����
deleterecord : ���ڵ带 ����
getrecord : ���ڵ��� ���̸� �б�
readrecord : ���ڵ带 �б�
writerecord : ���ڵ带 ����
expandrecord : ���ڵ��� ũ�⸦ ����
appendrecord : ���ڵ带 ���� ����
appendfile : ���ڵ带 ���� ����
compare : ���ڵ��� �ʵ� ���� ��
ridcompare : �� ���ڵ��� �ʵ� ���� ��

*/
#define	wiss_insertrecord(filenum, recaddr, reclen, nearrid, newrid, trans_id, lockup, cond) 				st_insertrecord(filenum, recaddr, reclen, nearrid, newrid, trans_id, lockup, cond)
#define	wiss_deleterecord(filenum, ridptr, trans_id, lockup, cond) 											st_deleterecord(filenum, ridptr, trans_id, lockup, cond)
#define	wiss_getrecord(filenum, ridptr, pageptr, recptr, trans_id, lockup, mode, cond) 						st_getrecord(filenum, ridptr, pageptr, recptr, trans_id, lockup, mode, cond) 
#define	wiss_readrecord(filenum, ridptr, recaddr, reclen, trans_id, lockup, mode, cond)						st_readrecord(filenum, ridptr, recaddr, reclen, trans_id, lockup, mode, cond)
#define	wiss_writerecord(filenum, ridptr, recaddr, reclen, trans_id, lockup, cond) 							st_writerecord(filenum, ridptr, recaddr, reclen, trans_id, lockup, cond)
#define	wiss_expandrecord(filenum, ridptr, expandamt, trans_id, lockup, cond) 								st_expandrecord(filenum, ridptr, expandamt, trans_id, lockup, cond)
#define	wiss_appendrecord(filenum, recaddr, reclen, newrid, trans_id, lockup, cond)							st_appendrecord(filenum, recaddr, reclen, newrid, trans_id, lockup, cond)
#define	wiss_appendfile(filenum, recaddr, reclen, newrid, trans_id, lockup, cond)							st_appendfile(filenum, recaddr, reclen, newrid, trans_id, lockup, cond)
#define	wiss_compare(filenum, ridptr, relation, field, constant, trans_id, lockup, cond)					st_compare(filenum, ridptr, relation, field, constant, trans_id, lockup, cond)
#define	wiss_ridcompare(ofn1, ridptr1, field1, ofn2, ridptr2, field2, returnvalue, trans_id, lockup, cond) 	st_ridcompare(ofn1, ridptr1, field1, ofn2, ridptr2, field2, returnvalue, trans_id, lockup, cond)
/* RECORD END */
/**/
#define	wiss_pinpage(filenum, pidptr, pageptr, trans_id, lockup, mode, cond) 								st_pinpage(filenum, pidptr, pageptr, trans_id, lockup, mode, cond) 
#define wiss_unpinpage(filenum, pidptr, pagebuf)															bf_unpinpage(filenum, pidptr, pagebuf)
#define	wiss_reservepage(filenum, pidptr, pageptr, trans_id, lockup, mode, cond) 							st_reservepage(filenum, pidptr, pageptr, trans_id, lockup, mode, cond) 
#define wiss_releasepage(filenum, pidptr, pagebuf)															bf_releasepage(filenum, pidptr, pagebuf)
#define wiss_freesize()																						st_freesize()
#define wiss_getdevaddr(volid)																				st_getdevaddr(volid)
#define wiss_getrecordptr(pageptr, slotno, recptr)															st_getrecordptr(pageptr, slotno, recptr)

#define wiss_reserve_alloc_pages(tableindex_ptr, pageptr_ptr, num_wanted_pages)								bf_reserve_alloc_pages(tableindex_ptr, pageptr_ptr, num_wanted_pages)
#define wiss_reserve_lock_page(filenum, pidptr, pageptr, trans_id, lockup, mode, cond)						st_reserve_lock_page(filenum, pidptr, pageptr, trans_id, lockup, mode, cond)

#define	wiss_appendnewfile(filenum, recaddr, reclen, newrid, trans_id, lockup, cond)						st_appendnewfile(filenum, recaddr, reclen, newrid, trans_id, lockup, cond)
/**/



/* SORT

���� :
���ڵ���� �����Ѵ�

�Լ� :
stsort : ���� ���� ���ڵ���� �����Ѵ�
stsortinto : ���� ���� ���ڵ���� ������ �̸��� ���Ͽ� �����Ѵ�

*/
#define	wiss_sort(VolID, filename, keyinfoptr, suffix, trans_id, lockup, cond) 				st_sort(VolID, filename, keyinfoptr, suffix, trans_id, lockup, cond)
#define	wiss_sortinto(VolID, filename, into, keyinfoptr, suffix, trans_id, lockup, cond) 	st_sortinto(VolID, filename, into, keyinfoptr, suffix, trans_id, lockup, cond)
/* SORT END */