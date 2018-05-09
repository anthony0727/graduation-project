
/* error codes returned by level 0 */

E(e0DEVSEEKERROR,-101,`device seek error')
E(e0DEVREADERROR,-201,`device read error')
E(e0DEVWRITEERROR,-301,`device write error')
E(e0MOUNTFAILED,-401,`device mount failed')
E(e0DISMOUNTFAILED,-501,`device dismount failed')

E(e0VOLNOTMOUNTED,-102,`referenced volume not mounted')
E(e0TOOMANYVOLS,-202,`too many volumes mounted')
E(e0TOOMANYFILES,-302,`too many files created')
E(e0NOSPACEONDISK,-402,`insufficient space on disk')
E(e0NOMOREMEMORY,-502,`insufficient main memory')
E(e0VOLMOUNTED,-602,`referenced volume is mounted ')

E(e0FIDPIDNOTMATCH,-103,`volume IDs in the FID and PID are inconsistent')
E(e0FILENOTINUSE,-203,`reference to a non-existent file')
E(e0INVALIDPID,-303,`invalid page number')
E(e0INVALIDFID,-403,`invalid file number')
E(e0BADHEADER,-503,`bad volume header')

E(e0NULLPIDPTR, -104, `null PID pointer')
E(e0NULLFIDPTR, -204, `null FID pointer')
E(e0NULLBUFPTR, -304, `null memory buffer pointer')
