
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

설명 :
에러 코드를 출력한다

함수 :
error :
errormsg :
fatalerror :

*/
#define wiss_error(routine, errorcode) 		am_error(routine, errorcode)
#define wiss_errormsg(errorcode)   			am_errormsg(errorcode)
#define wiss_fatalerror(routine, error)   	am_fatalerror(routine, error)
/* ERROR END */



/* VOLUMN

설명 :
WiSS 볼륨을 다룬다

함수 :
createvolumn : 볼륨을 생성
destroyvolumn : 볼륨을 삭제
mountvolumn : 볼륨을 마운트
dismountvolumn : 볼륨을 디스마운트

*/
#define	wiss_create_volume(devname, title, volid, numexts, extsize) 	am_create_volume(devname, title, volid, numexts, extsize)
#define	wiss_destroy_volume(devname) 									am_destroy_volume(devname)
#define	wiss_mount(devname) 											st_mount(devname)
#define	wiss_dismount(devname) 											st_dismount(devname)
/* VOLUMN END */



/* TRANSACTION

설명 :
레코드를 다룬다

함수 :

*/
#define	wiss_begin_trans()  begin_trans()
#define	wiss_commit_trans(trans_id) commit_trans(trans_id)
#define	wiss_abort_trans(transId) abort_trans(transId)
/* TRANSACTION END */



/* FILE

설명 :
파일을 다룬다

함수 :
createfile : 파일을 생성
destroyfile : 파일을 삭제
openfile : 파일을 열기
closefile : 파일을 닫기
rename : 파일의 이름을 변경

*/
#define	wiss_createfile(VolID, FileName, NumPages, ExtentFillFactor, PageFillFactor)    st_createfile(VolID, FileName, NumPages, ExtentFillFactor, PageFillFactor)
#define	wiss_destroyfile(VolID, FileName, trans_id, lockup, cond) 						st_destroyfile(VolID, FileName, trans_id, lockup, cond)
#define	wiss_openfile(volid, filename, mode) 											st_openfile(volid, filename, mode)
#define	wiss_closefile(filenum)   														am_closefile(filenum)
#define wiss_rename(volid, newname, oldname, trans_id, lockup, cond) 							st_rename(volid, new, old, trans_id, lockup, cond)
/* FILE END */



/* FILE STATISTICS

설명 :
알고 싶은 파일 디스크립터를 찾아서 파일의 정보를 알아낸다

함수 :
filepages : 데이터 파일의 페이지 수를 반환
indexpages : 인덱스 파일의 페이지 수를 반환
hashpages : 해시 파일의 페이지 수를 반환
filepages : 데이터 파일의 레코드(키) 수를 반환
indexpages : 인덱스 파일의 레코드(키) 수를 반환
hashpages : 해시 파일의 레코드(키) 수를 반환

*/
#define wiss_filepages(volid, filename) 			st_filepages(volid, filename)
#define wiss_indexpages(volid, filename, indexno) 	st_indexpages(volid, filename, indexno)
#define wiss_hashpages(volid, filename, indexno) 	st_hashpages(volid, filename, indexno)
#define wiss_recordcard(volid, filename) 			st_recordcard(volid, filename)
#define wiss_keycard(volid, filename, indexno)		st_keycard(volid, filename, indexno)
#define wiss_hashcard(volid, filename, indexno) 	st_hashcard(volid, filename, indexno)
/* FILE STATISTICS END */



/* INDEX

설명 :
데이터 파일에 대한 인덱스 파일을 다룬다

함수 :
createindex : 데이터 파일에 대한 B트리 인덱스 파일을 생성
dropindex : 인덱스 파일을 삭제
openindex : 인덱스 파일을 열기
insertindex : 인덱스 파일의 B트리에 RID를 삽입
deleteindex : 인덱스 파일의 B트리에서 RID를 제거
getindex : 인덱스 파일에서 인덱스 키의 RID를 읽음

*/
#define	wiss_createindex(VolID, FileName, IndexNo, KeyAttr, FillFactor, Unique, SortFile, trans_id, lockup, cond) 	st_createindex(VolID, FileName, IndexNo, KeyAttr, FillFactor, Unique,   SortFile, trans_id, lockup, cond)
#define	wiss_dropindex(volid, filename, indexno, trans_id, lockup, cond) 											st_dropbtree(volid, filename, indexno, trans_id, lockup, cond)
#define	wiss_openindex(volid, filename, indexno, accessmode) 														am_openindex(volid, filename, indexno, accessmode)
#define	wiss_insertindex(filenum, key, ridptr, trans_id, lockup, cond) 												st_insertindex(filenum, key, ridptr, trans_id, lockup, cond)
#define	wiss_deleteindex(filenum,key,ridptr, trans_id, lockup, cond) 												st_deleteindex(filenum,key,ridptr, trans_id, lockup, cond)
#define	wiss_getindex(filenum, SearchKey, Cursor, FirstRID, trans_id, lockup, oper, cond) 							st_getindex(filenum, SearchKey, Cursor, FirstRID, trans_id, lockup, oper, cond)
/* INDEX END */



/* HASH

설명 :
데이터 파일에 대한 해시 파일을 다룬다

함수 :
createhash : 데이터 파일에 대한 해시 파일을 생성
destroyhash : 해시 파일을 삭제
openhash : 해시 파일을 열기
inserthash : 해시 파일의 해시 키에 RID를 삽입
deletehash : 해시 파일의 해시 키에서 RID를 제거
gethash : 해시 파일에서 해시 키의 첫번째 RID를 읽음
nexthash : 해시 파일에서 해시 키의 다음 RID를 읽음

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

설명 :
조건에 부합하는 RID만을 고를 수 있도록 한다

함수 :
openfilescan : 데이터 파일의 스캔을 열기
setscan : 데이터 파일 스캔의 위치를 조정

openlongscan : 데이터 파일의 LDI 스캔을 열기
setcursor : 데이터 파일 LDI 스캔의 위치를 조정

openindexscan : 인덱스 파일의 스캔을 열기
geticursor : 인덱스 파일 스캔의 위치를 얻음
setiscan : 인덱스 파일 스캔의 위치를 조정

openhashscan : 해시 파일의 스캔을 열기

readscan : 현재 스캔 위치의 레코드를 읽기
insertscan : 현재 스캔 위치 근처에 레코드를 삽입
deletescan : 현재 스캔 위치의 레코드를 제거
updatescan : 현재 스캔 위치의 레코드를 수정
closescan : 스캔을 닫기

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

설명 :
데이터 파일의 레코드들 중 쿼리에 맞는 레코드의 위치를 찾는다

함수 :
fetchfirst : 스캔 쿼리에 처음으로 부합하는 레코드를 탐색
fetchlast : 스캔 쿼리에 마지막으로 부합하는 레코드를 탐색
fetchnext : 현재 위치에서 다음 방향으로 스캔 쿼리에 부합하는 레코드를 탐색
fetchprev : 현재 위치에서 이전 방향으로 스캔 쿼리에 부합하는 레코드를 탐색

*/
#define	wiss_fetchfirst(scanid, firstrid, type)		am_fetchfirst(scanid, firstrid, type)
#define	wiss_fetchlast(scanid, lastrid, type)		am_fetchlast(scanid, lastrid, type)
#define	wiss_fetchnext(scanid, nextrid, type)		am_fetchnext(scanid, nextrid, type)
#define	wiss_fetchprev(scanid, prevrid, type)		am_fetchprev(scanid, prevrid, type)
/* FETCH END */



/* FILE RECORD

설명 :
파일에서 RID를 찾는다

함수 :
firstfile : 파일의 처음부터 유효한 레코드를 탐색
nextfile : 파일의 현재 위치 이후로 유효한 레코드를 탐색
prevfile : 파일의 현재 위치 이전으로 유효한 레코드를 탐색
lastfile : 파일에서 마지막으로 유효한 레코드를 탐색

*/
#define wiss_firstfile(filenum, firstrid, trans_id, lockup, mode, cond) 		st_firstfile(filenum, firstrid, trans_id, lockup, mode, cond) 
#define wiss_nextfile(filenum, ridptr, nextrid, trans_id, lockup, mode, cond) 	st_nextfile(filenum, ridptr, nextrid, trans_id, lockup, mode, cond) 
#define wiss_prevfile(filenum, ridptr, prevrid, trans_id, lockup, mode, cond) 	st_prevfile(filenum, ridptr, prevrid, trans_id, lockup, mode, cond)
#define wiss_lastfile(filenum, lastrid, trans_id, lockup, mode, cond)			st_lastfile(filenum, lastrid, trans_id, lockup, mode, cond)
/* FILE RECORD END */



/* LONG DATA ITEM

설명 :
Long Data Item을 다룬다

함수 :
createlong : LDI를 생성
destroylong : LDI를 삭제
readlong : LDI의 데이터를 읽음
insertlong : LDI에 데이터를 추가
deletelong : LDI의 데이터를 삭제
updatelong : LDI의 데이터를 수정

*/
#define	wiss_createlong(filenum, ridptr, trans_id, lockup, cond) 	st_createlong(filenum, ridptr, trans_id, lockup, cond)
#define	wiss_destroylong(filenum, ridptr, trans_id, lockup, cond) 	st_destroylong(filenum, ridptr, trans_id, lockup, cond)
#define	wiss_readlong(scanid, recaddr, length) 						am_readlong(scanid, recaddr, length)
#define	wiss_insertlong(scanid, where, recaddr, length) 			am_insertlong(scanid, where, recaddr, length)
#define	wiss_deletelong(scanid,length)								am_deletelong(scanid,length)
#define	wiss_updatelong(scanid, recaddr, length) 					am_updatelong(scanid, recaddr, length)
/* LONG DATA ITEM END */



/* RECORD

설명 :
레코드를 다룬다

함수 :
insertrecord : 레코드를 삽입
deleterecord : 레코드를 삭제
getrecord : 레코드의 길이를 읽기
readrecord : 레코드를 읽기
writerecord : 레코드를 쓰기
expandrecord : 레코드의 크기를 조정
appendrecord : 레코드를 끝에 쓰기
appendfile : 레코드를 끝에 쓰기
compare : 레코드의 필드 값을 비교
ridcompare : 두 레코드의 필드 값을 비교

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

설명 :
레코드들을 정렬한다

함수 :
stsort : 파일 내의 레코드들을 정렬한다
stsortinto : 파일 내의 레코드들을 지정한 이름의 파일에 정렬한다

*/
#define	wiss_sort(VolID, filename, keyinfoptr, suffix, trans_id, lockup, cond) 				st_sort(VolID, filename, keyinfoptr, suffix, trans_id, lockup, cond)
#define	wiss_sortinto(VolID, filename, into, keyinfoptr, suffix, trans_id, lockup, cond) 	st_sortinto(VolID, filename, into, keyinfoptr, suffix, trans_id, lockup, cond)
/* SORT END */