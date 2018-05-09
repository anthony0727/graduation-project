/* errors from level 2, records sections */

E(e2NAMEINUSE,-121,`file name already in use')
E(e2BADSLOTNUMBER,-221,`an invalid slot number')
E(e2LISTOFEMPTYSLOTS,-321,`empty slots at end of page')	/* an inconsistency */
E(e2NOROOMONPAGE,-421,`no room on page for expansion')

E(e2NULLRIDPTR,-521,`null RID pointer')
E(e2NULLPIDPTR,-621,`null PID pointer')
E(e2NULLFIDPTR,-721,`null FID pointer')
E(e2NULLPAGEPTR,-921,`null page buffer pointer ')
E(e2NULLRECADDR,-821,`null record buffer address')
E(e2DELNOEXTREC,-1021,`delete a non-existing record')
E(e2PAGENOTINFILE,-1121,`the page referenced is not in the file')

E(e2ENDOFFILE,-1621,`end of file in st_nextfile/st_prevfile')
E(e2RECWONTFIT,-1721,`no room for record on this page')
E(e2CANTINSERTREC,-1821,`st_insertrecord fails for unknown reason')
E(e2VOLUMESTILLACTIVE,-1921 ,`attempt to dismount a volume with open files')

E(e2NOMORESLICES,-2121,`long data item grows exceed limit')
E(e2BADDATATYPE,-2221,`bad data type in st_compare')
E(e2ILLEGALOP,-2321,`illegal operator found in st_compare')
