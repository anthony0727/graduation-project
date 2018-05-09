

module main;

use
#include "wiss_mc.d"

#include "wiss_mc.h"

(* wiss file name *)
#define FILENAME	adr("student")
(* wiss device name *)
#define DEVICENAME	adr("wiss.disk")
(* error handler *)
#define	checkerror(p,c)	if (c < 0) then printf(" error in %s\n", p); halt;end

const
	SIDOFFSET	= 0;
	SAGEOFFSET	= 4;
	SYROFFSET	= 8;
	SNAMEOFFSET	= 12;
	SSEXOFFSET	= 60;

(* student record format (for the wiss file) *)
type
	studentrec = record
		id : integer;	
		age : integer;		
		yr  : integer;
		name : array 0 : 47 of char;
		sex : char;
	end (* studentrec *);

(* variables used by the main program *)
var
	errorcode : integer;		(* for returned errors *)
	vol : integer;			(* volume id *)
	dummy : integer;
	condition : array 0 : 1 of BOOLEXP;

procedure buildbool(intarg : integer; strarg : char);
begin
	condition[1].operator := EQ;
	assign2(condition[1].fielddesc.offset, SSEXOFFSET);
	assign2(condition[1].fielddesc.length, 1);
	condition[1].fielddesc.ftype := TSTRING;
	condition[1].bvalue[0] := strarg;
	
	condition[0].operator := GT;
	assign2(condition[0].fielddesc.offset, SIDOFFSET);
	assign2(condition[0].fielddesc.length, 4);
	condition[0].fielddesc.ftype := TINTEGER;

	movebytes(adr(condition[0].bvalue), adr(intarg), 4);

end buildbool;

(* populate a sample relation *)
procedure load_database(vol : integer; filename : addr);

(* 
   [Description of the data file] 
    the test data is in an ascii file with the following format :
    each record per line; 
    record length : 34,
    0-2 : ID, 3-22 : name; 23 : sex; 25-26 : age, 28 : year, 
    29- : ignored for the time being
*)

const
	INPUT	= "/usr4/katz/wiss/test/2/btree/studentdata/student";
	SRECLENGTH	= 34;	/* length of a record */

var
	i : integer;			(* loop index *)
	errorcode : integer;		(* for returned error codes *)
	unixfile : integer;		(* unix file descriptor *)
	openfile_number : integer;	(* open file number of the wiss file *)
	buf : array 0 : SRECLENGTH of char;	
					(* buffer for records in unix file *)
	newrid : RID;			(* RID of the newly created record *)
	student : studentrec;		(* buffer for records in wiss file  *)

begin

	(* open the wiss file with WRITE permission *)
	openfile_number := wiss_openfile(vol, filename, WRITE);
	checkerror("test2/wiss_openfile", openfile_number);

	(* open the unix file first *)
	unixfile := open(INPUT, 0);
	if (unixfile < 0) then
		printf("\ncan not open student data file");
		halt;
	end;

	(* read records from the unix file and put them into the wiss file *)
	while (read(unixfile, buf, SRECLENGTH) > 0) do
		(* convert the record format *)
		i := 0;
		student.id := 0;
		while (i < 3) do
			if (buf[i] <> ' ') then 
				student.id := student.id * 10 + 
					(integer(buf[i]) - integer('0'));
			end; (* if *)
			inc(i);
		end (* while *);
			
		while (i < 23) do
			student.name[i - 3] := buf[i];
			inc(i);
		end (* while *);

		student.sex := buf[23];
		student.age := (integer(buf[25]) - integer('0')) * 10 +
				(integer(buf[26]) - integer('0'));
		student.yr  := (integer(buf[28]) - integer('0'));

		(* insert it into the wiss file *)
		errorcode := wiss_insertrecord(openfile_number, 
			adr(student), size(student), NULL, newrid);
		checkerror("test2/wiss_insertrecord", errorcode);

	end; (* while *)

	(* close the wiss file *)
	errorcode := wiss_closefile(openfile_number);
	checkerror("test2/wiss_closefile", errorcode);

end load_database;



(* this procedure does a sequential scan over the relation call filename *)
procedure retrieve(vol : integer; filename : addr; boolexp : addr);
var
	errorcode : integer;		(* for returned error codes *)
	openfile_number : integer;	(* open file number of the wiss file *)
	scanid : integer;		(* id of the scan opened *)
	student : studentrec;		(* buffer for student records *)
	dummy : RID;
begin

	(* open the wiss file with READ permission *)
	openfile_number := wiss_openfile(vol, filename, READ);
	checkerror("test2/wiss_openfile", openfile_number);

	(* open a sequential scan on the file just opened *)
	scanid := wiss_openfilescan(openfile_number, boolexp);
	checkerror("test2/wiss_openfilescan", scanid);

	(* set the scan cursor to the first qualified record if any *)
	errorcode := wiss_fetchfirst(scanid, dummy);

	(* do a sequential scan, and print out all the records *)
	while (errorcode >= eNOERROR) do

		(* read in the qualified record *)
		errorcode := wiss_readscan(scanid, adr(student), size(student));
		checkerror("test2/wiss_readscan", errorcode);

		(* print student record *)
		printf(" (ID = %3d, Age = %2.2d, Year = %d, Sex = %c, ",
			student.id, student.age, student.yr, student.sex);
		printf("Name = ""%-20.20s"")\n", student.name);

		(* set cursor to the next record *)
		errorcode := wiss_fetchnext(scanid, dummy);

	end (* while *);

	(* close the scan *)
	errorcode := wiss_closescan(scanid);
	checkerror("test2/wiss_closescan", errorcode);

	errorcode := wiss_closefile(openfile_number);
	checkerror("test2/wiss_closefile", errorcode);

end retrieve;


(* start of the main module body *)
begin

	(* warm up procedure for wiss *)
	wiss_init;

	(* mount the volume call DEVICENAME *)
	vol := wiss_mount(DEVICENAME);	
	checkerror("test2/wiss_mount", vol);

	(* create a wiss file which has 6 pages initially, both the extent
	    fill factor and page fill factor are 90%                    *)
	errorcode := wiss_createfile(vol, FILENAME, 6, 90,90);
	checkerror("test2/wiss_createfile", errorcode);

	(* load the database from a unix file, and put it into a wiss file
	   call "FILENAME"						*)
	printf(" loading the database ... \n");
	load_database(vol, FILENAME);

	(* RETRIEVE FROM (FILENAME) *)
	printf(" retrieving all records ... \n");
	retrieve(vol, FILENAME, NULL);

	(* build a boolean expression: find records of the female students 
	   who's ID number are greater than 30                           *)

	(*dummy := 30; *)(* this is really ugly ! *)
buildbool(30, 'f');
	
	(* chain up the boolean terms *)
	condition[0].next := adr(condition[1]) -adr (condition[0]);
	condition[1].next := NULL;

	(* RETRIEVE FROM (FILENAME) WHERE <condition> *)
	printf(" retrieving records of female students (ID > 30) ... \n");
	retrieve(vol, FILENAME, adr(condition));

	(* remove the file since the test in done *)
	errorcode := wiss_destroyfile(vol, FILENAME);
	checkerror("test2/wiss_destroyfile", errorcode);

	(* dismount the volume call DEVICENAME *)
	errorcode := wiss_dismount(DEVICENAME);  
	checkerror("test2/wiss_dismount", errorcode);


end main.
