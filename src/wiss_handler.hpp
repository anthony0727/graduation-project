#ifndef __WISS_HANDLER_HPP__
#define __WISS_HANDLER_HPP__

#include <wiss_all.hpp>
#include <functional>
#include <algorithm>
#include <sstream>

#define	CHECKERR(p,c) if((int)(c)<0) fatalerror(p,(int)(c))
#define BUFFERSIZE 100

class wiss_handler	{
private:
	std::string volume_name;
	std::string relation_name;

	TWO volume_id;
	int devaddr;
	int relation_descriptor;

	int transId;

	int error;

public:
	wiss_handler()	{
	}

	~wiss_handler()	{
	}

	/* filename part */

	std::string filename_graphinfo()	{
		std::stringstream ss;
		ss << relation_name;
		ss << ".graphinfo";
		return ss.str();
	}

	std::string filename_graphindex()	{
		std::stringstream ss;
		ss << relation_name;
		ss << ".graphindex";
		return ss.str();
	}

	/* sequential functions */

	void initialize_wiss()	{
		wiss_init();
	}

	void finalize_wiss()	{
		wiss_final();
	}

	void mount_volume(std::string _volume_name)	{
		volume_name = _volume_name;

		volume_id = wiss_mount((char *)volume_name.c_str());
		CHECKERR("load/wiss_mount", volume_id);

		transId = begin_trans();
		printf("new transaction id = %d\n", transId);

		devaddr = wiss_getdevaddr(volume_id);
	}

	void dismount_volume()	{
		/* now commit the transaction */
		error = commit_trans(transId);
		if (error != 1)
			printf("error status return from commit_trans = %d\n", error);
		else printf("commit ok\n");

		error = wiss_dismount((char *)volume_name.c_str());
		CHECKERR("load/wiss_dismount", error);
	}

	void open_file(std::string _relation_name)	{
		relation_name = _relation_name;

		relation_descriptor = wiss_openfile(volume_id, (char *)relation_name.c_str(), READ);
		CHECKERR("load/wiss_openfile", relation_descriptor);

		error = wiss_lock_file(transId, relation_descriptor, l_S, COMMIT, FALSE);
		CHECKERR("build/wiss_lock_file", error);
	}

	void close_file()	{
		error = wiss_closefile(relation_descriptor);
		CHECKERR("load/wiss_closefile", error);
	}

	/* parallel functions */

	/* pin a page in page buffer */
	void pin_page(int page_id, char ** ptr)	{
		PID pid = { page_id, volume_id };

		CHECKERR("load/wiss_pinpage", wiss_pinpage(relation_descriptor, &pid, (DATAPAGE **)ptr, transId, FALSE, l_S, FALSE));
	}

	/* unpin a page in page buffer */
	void unpin_page(int page_id, char * ptr)	{
		PID pid = { page_id, volume_id };

		CHECKERR("load/wiss_unpinpage", wiss_unpinpage(relation_descriptor, &pid, (DATAPAGE *)ptr));
	}

	/* reserve a page space and pin the page space */
	int reserve_alloc_pages(int * tableindex_ptr, char ** pageptr_ptr, int num_wanted_pages)	{
		int num_acquired = wiss_reserve_alloc_pages(tableindex_ptr, pageptr_ptr, num_wanted_pages);

		CHECKERR("load/wiss_reserve_allocpages", num_acquired);

		return num_acquired;
	}

	/* reserve a page space and pin the page space */
	void reserve_lock_page(int page_id, char ** ptr)	{
		PID pid = { page_id, volume_id };

		CHECKERR("load/wiss_reserve_lock_page", wiss_reserve_lock_page(relation_descriptor, &pid, (DATAPAGE **)ptr, transId, FALSE, l_S, FALSE));
	}

	/* reserve a page space and pin the page space */
	void reserve_page(int page_id, char ** ptr)	{
		PID pid = { page_id, volume_id };

		CHECKERR("load/wiss_reservepage", wiss_reservepage(relation_descriptor, &pid, (DATAPAGE **)ptr, transId, FALSE, l_S, FALSE));
	}

	/* unpin a page loaded from disk */
	void release_page(int page_id, char * ptr)	{
		PID pid = { page_id, volume_id };

		CHECKERR("load/wiss_releasepage", wiss_releasepage(relation_descriptor, &pid, (DATAPAGE *)ptr));
	}

	/* get a record in the page */
	int get_record(char * page_ptr, int slot_no, char ** recptr)	{
		int length = wiss_getrecordptr((DATAPAGE **)&page_ptr, slot_no, recptr);
		CHECKERR("read/wiss_getrecordptr", length);

		return length;
	}

	/* other variables part */

	int get_freesize()	{
		return wiss_freesize();
	}

	int get_volume_id()	{
		return volume_id;
	}

	int get_devaddr()	{
		return devaddr;
	}

	/* error dealing part */

	void fatalerror(std::string p, int e)
	{
		printf("fatal error. first abort the transaction\n");
		wiss_abort_trans(transId);
		printf("errorcode %d\n", e);
		/* dismount the device */
		(void)wiss_dismount((char *)volume_name.c_str());

		wiss_final();  /* clean up processes and shared memory segments */

		wiss_fatalerror((char *)p.c_str(), (int)e);
	}

	/* preprocessing part */

	void preprocessing(std::string volumename, std::string filename, std::string dataname)	{
		char *relname = (char *)filename.c_str();

		int maxdata_byte = PAGESIZE - DPFIXED - 12;
		int maxdata_intsize = maxdata_byte / sizeof(EDGETYPE) - 2;
		int maxdata_intbyte = maxdata_intsize * sizeof(EDGETYPE);

		int	e, f1;
		RID	rid;
		int pagenum = 0, maxvid = -1;

		int current;

		mount_volume(volumename);

		/* make sure the relation  does not exist already */
		(void)wiss_destroyfile(volume_id, relname, transId, FALSE, FALSE);

		/*
		* Create and load the data file.
		*/

		e = wiss_createfile(volume_id, relname, 30, 100, 100);
		printf("return from createf=%d\n", e);
		CHECKERR("load/wiss_createfile", e);

		/* generate tuples */

		current = 1;	/* tuple being generated */

		f1 = wiss_openfile(volume_id, relname, WRITE);
		CHECKERR("load/wiss_openfile", f1);

		e = wiss_lock_file(transId, f1, l_IX, COMMIT, FALSE);
		CHECKERR("build/wiss_lock_file", e);

		relation_name = filename;

		FILE * fp = fopen(dataname.c_str(), "rt");
		FILE * fptable = fopen(filename_graphindex().c_str(), "wb");

		EDGETYPE * data = new EDGETYPE[maxdata_intsize];
		int prev_srcvid = 0;
		int curr_srcvid = 0;
		int remain_length;
		int record_size;

		bool prev_ldi;
		bool curr_ldi = false;

		int * index = new int[maxdata_intsize];
		int index_top = 0;

		while (true)
		{
			prev_srcvid = curr_srcvid;
			fread(&curr_srcvid, sizeof(int), 1, fp);
			fread(&remain_length, sizeof(int), 1, fp);

			if (feof(fp)) break;

			prev_ldi = curr_ldi;
			curr_ldi = remain_length > maxdata_intsize;

			if (curr_srcvid > maxvid) maxvid = curr_srcvid;

			do {
				record_size = remain_length > maxdata_intsize ? maxdata_intsize : remain_length;

				for (int i = 0; i < record_size; i++)	{
					fread(&data[i], sizeof(EDGETYPE), 1, fp);
				}

				if (record_size > 0)	{
					std::sort(data, data + record_size, less_edge);

					if (maxvid < get_node_id(data[record_size - 1])) maxvid = get_node_id(data[record_size - 1]);
				}

				if (prev_ldi == true)	{
					e = wiss_appendnewfile(f1, (char *)data, record_size * sizeof(EDGETYPE), &rid, transId, TRUE, FALSE);
					CHECKERR("load/wiss_appendfile", e);

					prev_ldi = false;
				}
				else {
					e = wiss_appendfile(f1, (char *)data, record_size * sizeof(EDGETYPE), &rid, transId, TRUE, FALSE);
					CHECKERR("load/wiss_appendfile", e);
				}

				if (rid.Rslot == 0)	{
					if (pagenum != 0)	{
						index[index_top++] = prev_srcvid;
						prev_srcvid = curr_srcvid;

						if (index_top + 3 > maxdata_intsize)	{
							fwrite(index, sizeof(int), index_top, fptable);
							index_top = 0;
						}
					}

					pagenum++;
					index[index_top++] = rid.Rpage;
					index[index_top++] = curr_srcvid;
				}
				
				remain_length = remain_length - record_size;
			} while (remain_length > 0);

			if (e < eNOERROR)
			{
				printf("%d error return from appendfile\n", e);
				break;
			}

			if ((current % 10000) == 0)
				printf("Total # of tuples written = %d\n", current);

			current++;  /* increment  number of tuples generated */
		}

		index[index_top++] = prev_srcvid;
		fwrite(index, sizeof(int), index_top, fptable);

		fclose(fptable);
		fclose(fp);

		delete data;
		delete index;

		e = wiss_closefile(f1);
		CHECKERR("load/wiss_closefile", e);
		printf("closefile completed\n");

		maxvid++;

		FILE * fpinfo = fopen(filename_graphinfo().c_str(), "wb");

		fwrite(&pagenum, sizeof(int), 1, fpinfo);
		fwrite(&maxvid, sizeof(int), 1, fpinfo);

		fclose(fpinfo);

		dismount_volume();
	}
};

#endif