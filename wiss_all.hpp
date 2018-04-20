#include <wiss.h>
#include <wiss_r.h>
#include <lockquiz.h>
#include <locktype.h>
#include <page.h>
#include <st_fd.h>

extern "C" {
	char * am_error(char * routine, int errorcode);
	char * am_errormsg(int errorcode);
	char * am_fatalerror(char * routine, int error);

	int am_create_volume(char * devname, char * title, int VolID, int numexts, int extsize);
	int am_destroy_volume(char * devname);
	int st_mount(char * devname);
	int st_dismount(char * devname);

	int begin_trans();
	int commit_trans(int trans_id);
	int abort_trans(int transId);

	int st_createfile(int VolID, char * filename, int NumPages, int ExtentFillFactor, int PageFillFactor);
	int st_destroyfile(int VolID, char * filename, int trans_id, short lockup, short cond);
	int st_openfile(TWO VolID, char * filename, int mode);
	int am_closefile(int filenum);
	int st_rename(int VolID, char * newname, char * oldname, int trans_id, short lockup, short cond);

	int st_filepages(int VolID, char * filename);
	int st_indexpages(int VolID, char * filename, int indexno);
	int st_hashpages(int VolID, char * filename, int indexno);
	int st_recordcard(int VolID, char * filename);
	int st_keycard(int VolID, char * filename, int indexno);
	int st_hashcard(int VolID, char * filename, int indexno);

	int st_createindex(short VolID, char * filename, TWO indexno, KEYINFO * KeyAttr, short FillFactor, ONE Unique, short SortFile, int trans_id, short lockup, short cond);
	int st_dropbtree(int VolID, char * filename, int indexno, int trans_id, short lockup, short cond);
	int am_openindex(int VolID, char * filename, int indexno, int accessmode);
	int st_insertindex(TWO filenum, KEY * key, RID * ridptr, int trans_id, short lockup, short cond);
	int st_deleteindex(int filenum, KEY * key, RID * ridptr, int trans_id, short lockup, short cond);
	int st_getindex(int filenum, KEY * SearchKey, XCURSOR * Cursor, RID * FirstRID, int trans_id, short lockup, enum bt_oper oper, short cond);

	int st_createhash(int VolID, char * filename, int hashno, KEYINFO * KeyAttr, int FillFactor, int Unique, int trans_id, short lockup, short cond);
	int st_destroyhash(int VolID, char * filename, int hashno, int trans_id, short lockup, short cond);
	int st_openhash(int VolID, char * filename, int hashno, int accessmode);
	int st_inserthash(int filenum, KEY * key, RID * h_rid, int trans_id, short lockup, short cond);
	int st_deletehash(int filenum, KEY * key, RID * h_rid, int trans_id, short lockup, short cond);
	int st_gethash(int filenum, KEY * key, XCURSOR * Cursor, RID * h_rid, int trans_id, short lockup, short cond);
	int st_nexthash(int filenum, XCURSOR * Cursor, RID * h_rid, int trans_id, short lockup, short cond);

	int am_openfilescan(int openfilenum, BOOLEXP * booleanexpr, short trans_id, short lockup, LOCKTYPE lockmode, short cond);
	int am_setscan(int scanid, RID * NewRID);

	int am_openlongscan(int openfilenum, RID * rid, int trans_id, short lockup, LOCKTYPE lockmode, short cond);
	int am_setcursor(int scanid, int offset, short relocation);

	int am_openindexscan(int openfilenum, int indexfilenum, KEYINFO * indexkey, KEY * lb, KEY * ub, BOOLEXP * booleanexpr, int trans_id, short lockup, LOCKTYPE lockmode, short cond);
	int am_geticursor(int scanid, XCURSOR * Xcursor);
	int am_setindexscan(int scanid, XCURSOR * NewXCURSOR);

	int am_openhashscan(int openfilenum, int hashfilenum, KEYINFO * keyAttr, KEY * hashkey, BOOLEXP * booleanexpr, int trans_id, short lockup, LOCKTYPE lockmode, short cond);

	int am_readscan(int scanid, char * recaddr, int len);
	int am_insertscan(int scanid, char * recaddr, int len, RID * newrid);
	int am_deletescan(int scanid);
	int am_updatescan(int scanid, char * recadr, int len);
	int am_closescan(int scanid);

	int am_fetchfirst(int scanid, RID * firstrid, enum logical_op type);
	int am_fetchlast(int scanid, RID * lastrid, enum logical_op type);
	int am_fetchnext(int scanid, RID * nextrid, enum logical_op type);
	int am_fetchprev(int scanid, RID * prevrid, enum logical_op type);

	int st_firstfile(int filenum, RID * firstrid, int trans_id, int lockup, LOCKTYPE mode, int cond);
	int st_nextfile(int filenum, RID * ridptr, RID * nextrid, int trans_id, short lockup, LOCKTYPE mode, short cond);
	int st_prevfile(int filenum, RID * ridptr, RID * prevrid, int trans_id, short lockup, LOCKTYPE mode, short cond);
	int st_lastfile(int filenum, RID * lastrid, int trans_id, short lockup, LOCKTYPE mode, short cond);

	int st_createlong(int filenum, RID * ridptr, int trans_id, short lockup, short cond);
	int st_destroylong(int filenum, RID * ridptr, int trans_id, short lockup, short cond);
	int am_readlong(int scanid, char * recaddr, int length);
	int am_insertlong(int scanid, short where, char * recaddr, int length);
	int am_deletelong(int scanid, int length);
	int am_updatelong(int scanid, char * recaddr, int length);

	int st_insertrecord(int filenum, char * recaddr, int reclen, RID * nearrid, RID * newrid, int trans_id, short lockup, short cond);
	int st_deleterecord(int filenum, RID * ridptr, int trans_id, short lockup, short cond);
	int st_getrecord(int filenum, RID * ridptr, DATAPAGE ** pageptr, char ** recptr, int trans_id, short lockup, int mode, short cond);
	int st_readrecord(int filenum, RID * ridptr, char * recaddr, int reclen, int trans_id, short lockup, LOCKTYPE mode, short cond);
	int st_writerecord(int filenum, RID * ridptr, char * recaddr, int reclen, int trans_id, short lockup, short cond);
	int st_expandrecord(int filenum, RID * ridptr, int expandamt, int trans_id, short lockup, short cond);
	int st_appendrecord(int filenum, char * recaddr, int reclen, RID * newrid, int trans_id, short lockup, short cond);
	int st_appendfile(int filenum, char * recaddr, int reclen, RID * newrid, int trans_id, short lockup, short cond);
	int st_compare(int filenum, RID * ridptr, enum rel_op relation, FIELDDESC * field, char * constant, int trans_id, short lockup, short cond);
	int st_ridcompare(int ofn1, RID * ridptr1, FIELDDESC * field1, int ofn2, RID * ridptr2, FIELDDESC * field2, int * returnvalue, int trans_id, short lockup, short cond);

	/**/
	int st_pinpage(int filenum, PID * pidptr, DATAPAGE ** pageptr, int trans_id, short lockup, int mode, short cond);
	int bf_unpinpage(int filenum, PID * pidptr, DATAPAGE * pagebuf);
	int st_reservepage(int filenum, PID * pidptr, DATAPAGE ** pageptr, int trans_id, short lockup, int mode, short cond);
	int bf_releasepage(int filenum, PID * pidptr, DATAPAGE * pagebuf);
	int st_freesize();
	int st_getdevaddr(TWO volid);
	int st_getrecordptr(DATAPAGE ** pageptr, int slotno, char ** recptr);

	int bf_reserve_alloc_pages(int * tableindex_ptr, char ** pageptr_ptr, int num_wanted_pages);
	int st_reserve_lock_page(int filenum, PID * pidptr, DATAPAGE ** pageptr, int trans_id, short lockup, int mode, short cond);

	int st_appendnewfile(int filenum, char * recaddr, int reclen, RID * newrid, int trans_id, short lockup, short cond);
	/**/

	int st_sort(TWO VolID, char * filename, KEYINFO * keyinfoptr, TWO suffix, int trans_id, short lockup, short cond);
	int st_sortinto(TWO VolID, char * filename, char * into, KEYINFO * keyinfoptr, TWO suffix, int trans_id, short lockup, short cond);
}

extern "C" {
	int wiss_init();
	int wiss_lock_file(int transId, int openfilenum, int lockMode, short duration, short cond);
	int wiss_final();
}
