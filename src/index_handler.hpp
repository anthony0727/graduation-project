#ifndef __INDEX_HANDLER_HPP__
#define __INDEX_HANDLER_HPP__

#include <sstream>
#include <vector>
#include <atomic>

struct pageinfo	{
	int page_id;					/* page id in the volume */
	int first_nodeid;				/* first nodenumber of the page */
	int last_nodeid;				/* last nodenumber of the page */
};

struct pagemeta	{
	std::atomic_flag lock;
	char * pre_result;
	int num_length;
	int num_count;
};

class index_handler {
private:
	//indexes
	struct pageinfo * graphindex;	/* index which stores page information about preprocessed graph structure in volume */
	struct pagemeta ** graphmeta_ptr;
	struct pagemeta * graphmeta;
	char ** bufferindex;			/* index which stores address information about pages existing in the buffer */
	int * connectedsize;			/* index which stores the number of pages connected */

	//index info
	std::string filename;
	int size_index;					/* size of index which is equal to the number of pages in preprocessed graph structure */
	int num_nodes;					/* number of nodes in the graph data */

public:
	index_handler()	{
	}

	~index_handler()	{
	}

	std::string filename_graphinfo()	{
		std::stringstream ss;
		ss << filename;
		ss << ".graphinfo";
		return ss.str();
	}

	std::string filename_graphindex()	{
		std::stringstream ss;
		ss << filename;
		ss << ".graphindex";
		return ss.str();
	}

	void load_graphindex(std::string _filename)	{
		filename = _filename;

		FILE * fp = NULL;
		
		/* load graph information */
		fp = fopen(filename_graphinfo().c_str(), "rb");

		fread(&size_index, sizeof(int), 1, fp);
		fread(&num_nodes, sizeof(int), 1, fp);

		fclose(fp);

		/* load ridtable */
		graphindex = new struct pageinfo[size_index];

		fp = fopen(filename_graphindex().c_str(), "rb");

		fread(graphindex, sizeof(struct pageinfo), size_index, fp);

		fclose(fp);

		/* make pagemeta */
		graphmeta_ptr = new struct pagemeta*[size_index];
		graphmeta = new struct pagemeta[size_index];

		/* make bufferindex */
		bufferindex = new char*[size_index];

		/* make lock */
		for (int index = 0; index < size_index; index++)	{
			graphmeta[index].lock.clear();
			graphmeta[index].pre_result = NULL;
			graphmeta[index].num_length = 0;
			graphmeta[index].num_count = 0;
		}

		/* make ldi */
		connectedsize = new int[size_index];

		memset(connectedsize, 0, size_index * sizeof(int));
		for (int index = 1; index < size_index;)	{
			int start_index = index - 1;
			int last_index = index;

			for (; last_index < size_index; last_index++)	{
				if (graphindex[last_index].first_nodeid != graphindex[last_index - 1].first_nodeid)	{
					break;
				}
			}

			for (int i = start_index; i < last_index; i++)	{
				connectedsize[i] = last_index - start_index;
				graphmeta_ptr[i] = &graphmeta[start_index];
			}

			index = last_index + 1;
		}

		if (connectedsize[size_index - 1] == 0)		{
			connectedsize[size_index - 1] = 1;
			graphmeta_ptr[size_index - 1] = &graphmeta[size_index - 1];
		}
	}

	void unload_graphindex()	{
		delete graphmeta_ptr;
		delete graphmeta;
		delete graphindex;
		delete bufferindex;
		delete connectedsize;
	}

	void clear_pagemeta()	{
		for (int index = 1; index < size_index; index++)	{
			graphmeta[index].num_length = 0;
			graphmeta[index].num_count = 0;
		}
	}

	int get_size_index()	{
		return size_index;
	}

	int get_num_nodes()	{
		return num_nodes;
	}

	int get_first_nodeid(int tableindex)	{
		return graphindex[tableindex].first_nodeid;
	}

	int get_last_nodeid(int tableindex)	{
		return graphindex[tableindex].last_nodeid;
	}

	int get_pageid(int tableindex)	{
		return graphindex[tableindex].page_id;
	}

	struct pageinfo get_pageinfo(int tableindex)	{
		return graphindex[tableindex];
	}

	int get_connectedsize(int tableindex)	{
		return connectedsize[tableindex];
	}

	pagemeta * get_pagemeta_ptr(int tableindex)	{
		return graphmeta_ptr[tableindex];
	}

	char * get_bufferindex(int tableindex)	{
		return bufferindex[tableindex];
	}

	void set_bufferindex(int tableindex, char * ptr)	{
		bufferindex[tableindex] = ptr;
	}

	char ** get_bufferindex_ptr()	{
		return bufferindex;
	}
};

#endif
