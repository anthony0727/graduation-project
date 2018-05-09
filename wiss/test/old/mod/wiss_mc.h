
#include <wiss.outside>



(* definitions from WiSS *)
const
	(* boolean values *)
	FALSE = 0;
	TRUE = 1;

	(* file access modes *)
	READ = 0;
	WRITE = 1;

	(* null address pointer *)
	NULL = 0;

	(* special "error" code *)
	eNOERROR = 0;

	(* relational operators *)
	EQ = 0;
	NE = 1;
	LT = 2;
	LE = 3;
	GT = 4;
	GE = 5;

	(* data types recognized by WiSS *)
	TINTEGER = 0;
	TLONG    = 1;
	TSHORT   = 2;
	TFLOAT   = 3;
	TDOUBLE  = 4;
	TSTRING  = 5;

	(* define the length you need *) 
	MAXFIELD = 256;

type

	addr = integer;			(* address or pointer type *)

	(* something that is two-byte long *)
	two  = array 0 : 1 of shortint;	

	RID  = array 0 : 7 of char;
	PID  = array 0 : 5 of char;

	(* field descriptor *)
	FIELDDESC = 
		record
			offset : two;
			length : two;
			ftype  : integer;
		end; (* of record *)

	(* data descriptor *)
	DATADESC = 
		record
			dtype  : integer;
			length : two;
		        dvalue : array 0 : (MAXFIELD - 1) of char;
		end; (* of record *)
	
	(* boolean expression *)
	BOOLEXP = 
		record
			operator  : integer;
			fielddesc : FIELDDESC;
			next	  : addr;
		        bvalue    : array 0 : (MAXFIELD - 1) of char;
		end; (* of record *)
			

(* here is the interface routines *)
procedure wiss_mount(DeviceName : addr) : integer;		external;
procedure wiss_dismount(DeviceName : addr) : integer;		external;

(*
procedure wiss_readrecord(OpenFileNum : integer;
	const RecID : RID; RecAdr : addr; 
	Length : integer) : integer;				external;
procedure wiss_writerecord(OpenFileNum : integer;
	RecID : addr; RecAdr : addr; 
	Length : integer) : integer;				external;
*)
procedure wiss_insertrecord(OpenFileNum : integer;
	RecAdr : addr; Length : integer;
	NearRID, NewRID : addr) : integer;			external;
(*
procedure wiss_appendrecord(OpenFileNum : integer;
	RecAdr : addr; Length : integer;
	NewRID : addr) : integer;				external;
procedure wiss_deleterecord(OpenFileNum : integer;
	NewRID : addr) : integer;				external;

procedure wiss_createlong(OpenFileNum : integer;
	DirRID : addr) : integer;				external;
procedure wiss_destroylong(OpenFileNum : integer;
	DirRID : addr) : integer;				external;

procedure wiss_sort(VolID : integer; FileName : addr;
	Keyattr : addr; Suffix : integer) : integer;		external;
	DirRID : addr) : integer;				external;
*)

procedure wiss_createfile(VolID : integer;
	FileName : addr; NumPages : integer;
	ExtentFillFactor, PageFillFactor : integer) : integer;	external;
procedure wiss_destroyfile(VolID : integer;
	FileName : addr) : integer;				external;
procedure wiss_createindex(VolID : integer; FileName : addr; 
	IndexNo : integer; KeyAttr : addr; FillFactor : integer;
	Primary, Sorted : integer) : integer;			external;
procedure wiss_dropindex(VolID : integer;
	FileName : addr; IndexNo : integer) : integer;		external;

procedure wiss_openfile(VolID : integer; FileName : addr;
	Mode : integer) : integer;				external;
procedure wiss_openindex(VolID : integer; FileName : addr;
	IndexNo, Mode : integer) : integer;			external;
procedure wiss_closefile(OpenFileNum : integer) : integer;	external;

procedure wiss_openfilescan(OpenFileNum : integer;
	BoolExpr : addr) : integer;				external;
procedure wiss_openindexscan(OpenFileNum : integer;
	IndexFileNum : integer; IndexKeyAttr : addr;
	LB, UB, BoolExpr : addr) : integer;			external;
procedure wiss_closescan(ScanID : integer) : integer;		external;

procedure wiss_readscan(ScanID : integer; RecAdr : addr;
	Length : integer) : integer;				external;

procedure wiss_fetchfirst(ScanID : integer; 
	FirstRID : addr) : integer;				external;
procedure wiss_fetchnext(ScanID : integer;
	NextRID : addr) : integer;				external;


procedure wiss_init;						external;

procedure wiss_error(message : addr; errorcode : integer);	external;
procedure wiss_fatalerror(message : addr; errorcode : integer);	external;

(************************************************************************)

(* movebytes is a WiSS utility routine that moves a block of bytes *)
(* "to" is the address of the destination, "from" is the address of
   the source, and "byte_count" is the number of bytes to move     *)
procedure movebytes(to, from : addr; byte_count : integer);	external;


(* since vmc currently don't have anything that is two-byte long,
   this macro allows you to assign a 4-byte integer into a variable
   that is faked as two-bye *)


#define assign2(to , from)\
	to[0] := from mod 400B; to[1] := (from mod 2000B) / 400B


