#ifndef __attribute_handler_HPP__
#define __attribute_handler_HPP__

#include <unistd.h>
#include <fcntl.h>
#include <atomic>
#include <vector>
#include <omp.h>
#include <climits>

#define BOUNDTYPE_SRC true
#define BOUNDTYPE_DST false

#define IN true
#define OUT false

#define OFFSET_BITMAP 20
#define NUM_VALUES_IN_PAGE (size_t(1) << OFFSET_BITMAP)

struct attribute_handler_info	{
	std::string filename;
	size_t size_valuetype;
	bool existfile;

	attribute_handler_info(std::string _filename, size_t _size_valuetype, bool _existfile)	{
		filename = _filename;
		size_valuetype = _size_valuetype;
		existfile = _existfile;
	}
};

#define WORDTYPE size_t
#define WORDTYPP_BYTE 8
#define WORDRANGE_LOG2 6
#define WORDFILTER 63
#define WORDBIT(nodenumber) (WORDTYPE(1) << ((nodenumber) & WORDFILTER))

class bitattribute_handler {
private:
	//bit array
	std::atomic<WORDTYPE> * bitarray;	/* bit array of current iteration */

	//array info
	size_t size_array;						/* size of bit array in size_word */

	//node info
	size_t num_nodes;						/* total number of nodes */

public:
	bitattribute_handler() : bitarray(NULL) {
	}

	~bitattribute_handler()	{
	}

	/* memory allocation and deallocation */
	void create(size_t _num_nodes)	{
		size_array = (_num_nodes >> WORDRANGE_LOG2) + 1;

		num_nodes = _num_nodes;

		bitarray = new std::atomic<WORDTYPE>[size_array];

		clear();
	}
	void destroy()	{
		delete bitarray;
	}

	/* clear all bits */
	void clear()	{
		for (size_t arrayindex = 0; arrayindex < size_array; arrayindex++)	{
			bitarray[arrayindex] = 0;
		}
	}

	/* is nodenumber set? */
	bool get_bit(size_t nodenumber)	{
		return (bitarray[nodenumber >> WORDRANGE_LOG2] & (WORDTYPE(1) << (nodenumber & WORDFILTER))) != 0;
	}

	/* set a bit of nodenumber */
	bool set_bit(size_t nodenumber)	{
		return (atomic_fetch_or(&bitarray[nodenumber >> WORDRANGE_LOG2], WORDBIT(nodenumber)) & WORDBIT(nodenumber)) == WORDTYPE(0);
	}

	size_t get_size_array()	{
		return size_array;
	}
};

class attribute_handler {
private:
	int attribute_fd;
	std::string filename;

	size_t size_valuetype;

	/* chunk information */
	size_t num_nodes;
	size_t num_values;				/* number of values in a chunk */
	size_t num_chunks;				/* number of chunks in a vector */
	size_t num_pages;
	size_t size_chunk;				/* memory size of a chunk */
	size_t size_last_chunk;

	size_t firstnode_id;			/* first node number in the chunk */
	size_t lastnode_id;				/* last node number in the chunk */
	size_t current_chunk_id;			/* offset to represent current chunk number in the vector */
	size_t next_chunk_id;

	/* page information */
	size_t size_page;
	size_t size_lastpage;

	char * default_value;
	char * attribute_ptr;			/* attribute vector to contain values resulted from graph algorithms */

	bool * pagelog_ptr;
	bool * pageflag_ptr;
	
	bool inmemory;					/* all attribute values can be in memory? */
	bool boundtype;					/* attribute for source or destination node */
	bool existfile;					/* flag whether to use already existing file */
	bool writefile;					/* flag whether to write attributes after processing graph algorithm? */
	bool edge_direction;
public:
	attribute_handler(attribute_handler_info vector_info, size_t _num_nodes, size_t _num_chunks, size_t _num_pages, size_t _num_values, bool _edge_direction)	{
		/* attribute filename */
		filename = vector_info.filename;

		/* size of value */
		size_valuetype = vector_info.size_valuetype;

		/* chunk information */
		num_nodes = _num_nodes;
		num_chunks = _num_chunks;
		num_values = _num_values;
		num_pages = _num_pages;
		size_chunk = num_values * size_valuetype;
		size_last_chunk = (num_nodes - (num_chunks - 1) * num_values) * size_valuetype;

		firstnode_id = 0;
		lastnode_id = num_values - 1;
		current_chunk_id = num_chunks == 1 ? 0 : num_chunks;

		/* page information */
		size_page = NUM_VALUES_IN_PAGE * size_valuetype;
		size_lastpage = (num_nodes - (num_pages - 1) * NUM_VALUES_IN_PAGE) * size_valuetype;

		default_value = NULL;
		attribute_ptr = NULL;

		pagelog_ptr = NULL;
		pageflag_ptr = NULL;
		
		/* other flags */
		inmemory = num_chunks == 1 ? true : false;
		existfile = vector_info.existfile;
		writefile = false;
		edge_direction = _edge_direction;
	}

	~attribute_handler() {
	}

	void set(bool * _pageflag_ptr)	{
		/* set vector */
		attribute_ptr = (char *)malloc(size_chunk);
		pagelog_ptr = (bool *)malloc(num_pages);
		memset(pagelog_ptr, false, num_pages);

		if (existfile) {
			attribute_fd = open(filename.c_str(), O_RDWR);

			if (inmemory)	{
#pragma omp parallel for schedule(dynamic, 1)
				for (size_t pageindex = 0; pageindex < num_pages; pageindex++) {
					pread(attribute_fd, attribute_ptr + pageindex * size_page, size_page, pageindex * size_page);
				}
				
				close(attribute_fd);
			}
		}
		else {
			if (inmemory)	{
				if (default_value != NULL)	{
#pragma omp parallel for
					for (char * value_ptr = attribute_ptr; value_ptr < attribute_ptr + size_chunk; value_ptr += size_valuetype) {
						memcpy(value_ptr, default_value, size_valuetype);
					}
				}
			}
			else {
				attribute_fd = open(filename.c_str(), O_RDWR | O_CREAT | O_TRUNC);
				ftruncate(attribute_fd, num_nodes * size_valuetype);

				if (default_value != NULL && !writefile)	{
					for (char * value_ptr = attribute_ptr; value_ptr < attribute_ptr + size_page; value_ptr += size_valuetype) {
						memcpy(value_ptr, default_value, size_valuetype);
					}

#pragma omp parallel for schedule(dynamic, 1)
					for (size_t page_id = 0; page_id < num_pages; page_id++) {
						pwrite(attribute_fd, attribute_ptr, (page_id + 1 != num_pages) ? size_page : size_lastpage, page_id * size_page);
					}
				}
			}
		}

		pageflag_ptr = _pageflag_ptr;
	}

	void unset()	{
		if (!inmemory)	{
			close(attribute_fd);
		}

		free(attribute_ptr);
		free(pagelog_ptr);
		if (default_value != NULL) free(default_value);
	}

	void request_chunk(size_t chunk_id)	{
		next_chunk_id = chunk_id;
	}

	void exchange_chunk()	{
		if (inmemory) return;
		if (current_chunk_id == next_chunk_id) return;

		if (is_chunkid_valid(current_chunk_id))	{
			if (writefile) write_chunk(current_chunk_id);
		}

		if (is_chunkid_valid(next_chunk_id))	{
			read_chunk(next_chunk_id);
		}
	}

	void read_chunk(size_t chunk_id)	{
		if (inmemory) return;

		firstnode_id = chunk_id * num_values;
		lastnode_id = (chunk_id + 1 == num_chunks) ? (num_nodes - 1) : (firstnode_id + num_values - 1);
		current_chunk_id = chunk_id;

		size_t fpage_id = firstnode_id >> OFFSET_BITMAP;
		size_t lpage_id = lastnode_id >> OFFSET_BITMAP;
		size_t file_offset = chunk_id * size_chunk;

#pragma omp parallel for schedule(dynamic, 1)
		for (size_t page_id = fpage_id; page_id <= lpage_id; page_id++)	{
			if (edge_direction == boundtype || pageflag_ptr[page_id] == true)	{
				if (default_value == NULL || pagelog_ptr[page_id] == true) {
					pread(attribute_fd, attribute_ptr + (page_id - fpage_id) * size_page, (page_id + 1 != num_pages) ? size_page : size_lastpage, file_offset + (page_id - fpage_id) * size_page);
				}
				else {
					char * value_ptr = attribute_ptr + (page_id - fpage_id) * size_page;
					char * value_ptr_end = value_ptr + ((page_id + 1 != num_pages) ? size_page : size_lastpage);
					while (value_ptr < value_ptr_end) {
						memcpy(value_ptr, default_value, size_valuetype);
						value_ptr += size_valuetype;
					}
				}

				pagelog_ptr[page_id] = true;
			}
		}
	}

	void write_chunk(size_t chunk_id)	{
		size_t fpage_id = firstnode_id >> OFFSET_BITMAP;
		size_t lpage_id = lastnode_id >> OFFSET_BITMAP;
		size_t file_offset = chunk_id * size_chunk;

#pragma omp parallel for schedule(dynamic, 1)
		for (size_t page_id = fpage_id; page_id <= lpage_id; page_id++)	{
			if (edge_direction == boundtype || pageflag_ptr[page_id] == true)	{
				pwrite(attribute_fd, attribute_ptr + (page_id - fpage_id) * size_page, (page_id + 1 != num_pages) ? size_page : size_lastpage, file_offset + (page_id - fpage_id) * size_page);
			}
		}
	}

	void flush_chunk()	{
		if (is_chunkid_valid(current_chunk_id))	{
			if (writefile) write_chunk(current_chunk_id);
		}

		firstnode_id = 0;
		lastnode_id = num_values - 1;
		current_chunk_id = num_chunks;
	}

	bool is_chunkid_valid(size_t chunk_id)	{
		return (0 <= chunk_id && chunk_id < num_chunks);
	}

	bool is_load_needed(size_t lower_pageid, size_t upper_pageid)	{
		if (!is_chunkid_valid(next_chunk_id)) return false;
		if (edge_direction == boundtype) return true;

		size_t fpage_id = (next_chunk_id * num_values) >> OFFSET_BITMAP;
		size_t lpage_id = ((next_chunk_id + 1 == num_chunks) ? (num_nodes - 1) : (firstnode_id + num_values - 1)) >> OFFSET_BITMAP;

		if (fpage_id > upper_pageid || lpage_id < lower_pageid) return false;

		return true;
	}


	/* local values related functions */


	size_t get_firstnode_number()	{
		return firstnode_id;
	}
	
	size_t get_lastnode_number()	{
		return lastnode_id;
	}

	bool is_inmemory()	{
		return inmemory;
	}

	void set_boundtype(bool _boundtype)	{
		boundtype = _boundtype;
	}

	bool get_boundtype()	{
		return boundtype;
	}

	void set_writefile(bool _writefile)	{
		writefile = _writefile;
	}

	size_t get_chunk_id()	{
		return current_chunk_id;
	}


	/* algorithm related functions */


	template<typename valuetype>
	void get_first20(valuetype * toplist)	{
		valuetype * value_ptr = (valuetype *)attribute_ptr;
		int max20ths = lastnode_id - firstnode_id + 1;
		if (max20ths > 20) max20ths = 20;
		for (int i = 0; i < max20ths; i++)	{
			*(toplist + i) = value_ptr[i];
		}
	}

	template<typename valuetype>
	void sort_chunk()	{
		valuetype * value_ptr = (valuetype *)attribute_ptr;
		std::sort(value_ptr, value_ptr + (lastnode_id - firstnode_id + 1), std::greater<valuetype>());
	}

	template<typename valuetype>
	void initialize_startnode(size_t startnode, valuetype value)	{
		if (inmemory)	{
			valuetype * value_ptr = (valuetype *)attribute_ptr;
			value_ptr[startnode] = value;
		}
		else {
			pwrite(attribute_fd, &value, size_valuetype, startnode * size_valuetype);
		}
	}

	template<typename valuetype>
	void set_default_value(valuetype value)	{
		valuetype * value_ptr;
		if (default_value == NULL) default_value = (char *)malloc(size_valuetype);
		value_ptr = (valuetype *)default_value;
		*value_ptr = value;

		if (inmemory) {
			if (attribute_ptr != NULL)	{
				value_ptr = (valuetype *)attribute_ptr;

#pragma omp parallel for
				for (size_t value_id = 0; value_id < num_values; value_id++) {
					value_ptr[value_id] = value;
				}
			}
		}
		else {
			if (pagelog_ptr != NULL) memset(pagelog_ptr, false, num_pages);
		}
	}

	template<typename valuetype>
	valuetype get_value(size_t nodenumber)	{
		valuetype * value_ptr = (valuetype *)attribute_ptr;
		return value_ptr[nodenumber - firstnode_id];
	}

	template<typename valuetype>
	void set_value(size_t nodenumber, valuetype value)	{
		valuetype * value_ptr = (valuetype *)attribute_ptr;
		value_ptr[nodenumber - firstnode_id] = value;
	}

	template<typename valuetype>
	void add_value(size_t nodenumber, valuetype value)	{
		valuetype * value_ptr = (valuetype *)attribute_ptr;
		value_ptr[nodenumber - firstnode_id] += value;
	}
	
	template<typename valuetype>
	void sub_value(size_t nodenumber, valuetype value)	{
		valuetype * value_ptr = (valuetype *)attribute_ptr;
		value_ptr[nodenumber - firstnode_id] -= value;
	}

	template<typename valuetype>
	void lock_set_unlock_value(size_t nodenumber, valuetype value, std::atomic_flag * lock)	{
		while (lock->test_and_set());
		valuetype * value_ptr = (valuetype *)attribute_ptr;
		value_ptr[nodenumber - firstnode_id] = value;
		lock->clear();
	}

	template<typename valuetype>
	void lock_add_unlock_value(size_t nodenumber, valuetype value, std::atomic_flag * lock)	{
		while (lock->test_and_set());
		valuetype * value_ptr = (valuetype *)attribute_ptr;
		value_ptr[nodenumber - firstnode_id] += value;
		lock->clear();
	}

	template<typename valuetype>
	void lock_sub_unlock_value(size_t nodenumber, valuetype value, std::atomic_flag * lock)	{
		while (lock->test_and_set());
		valuetype * value_ptr = (valuetype *)attribute_ptr;
		value_ptr[nodenumber - firstnode_id] -= value;
		lock->clear();
	}

	template<typename valuetype>
	void lock_value(std::atomic_flag * lock)	{
		while (lock->test_and_set());
	}

	template<typename valuetype>
	void unlock_value(std::atomic_flag * lock)	{
		lock->clear();
	}

	void read_chunks(size_t chunk_id)	{
		if (inmemory) return;

		firstnode_id = chunk_id * num_values;
		lastnode_id = (chunk_id + 1 == num_chunks) ? (num_nodes - 1) : (firstnode_id + num_values - 1);
		current_chunk_id = chunk_id;

		size_t fpage_id = firstnode_id >> OFFSET_BITMAP;
		size_t lpage_id = lastnode_id >> OFFSET_BITMAP;
		size_t file_offset = chunk_id * size_chunk;

#pragma omp parallel for schedule(dynamic, 1)
		for (size_t page_id = fpage_id; page_id <= lpage_id; page_id++) {
			pread(attribute_fd, attribute_ptr + (page_id - fpage_id) * size_page, size_page, file_offset + (page_id - fpage_id) * size_page);
		}
	}
};

class attribute_wrapper	{
private:
	size_t max_memory_mb;
	size_t num_nodes;
	size_t num_values;
	size_t num_chunks;
	size_t num_pages;

	std::vector<attribute_handler *> vectors;
	std::vector<bitattribute_handler *> bitvectors;
	std::vector<attribute_handler_info> vector_infos;

	size_t size_total_valuetype;

	size_t src_lowerbound;
	size_t src_upperbound;
	size_t dst_lowerbound;
	size_t dst_upperbound;

	bool * pageflag_ptr;

	size_t lower_pageid;
	size_t upper_pageid;

	bool edge_direction;
	bool inmemory;
public:
	attribute_wrapper() {
	}

	~attribute_wrapper() {
	}

	void set_wrapper(size_t _max_memory_mb, size_t _num_nodes, bool _edge_direction)	{
		inmemory = true;
		edge_direction = _edge_direction;

		max_memory_mb = _max_memory_mb;
		num_nodes = _num_nodes;

		size_total_valuetype = 0;
	}

	void unset_wrapper()	{
		for (size_t i = 0; i < vectors.size(); i++)	{
			vectors[i]->unset();
		}

		for (size_t i = 0; i < bitvectors.size(); i++)	{
			bitvectors[i]->destroy();
		}

		if (!inmemory) free(pageflag_ptr);
		vectors.clear();
		bitvectors.clear();
	}


	/* algorithm related functions */


	attribute_handler * get_vector(size_t vector_index)	{
		return vectors[vector_index];
	}

	void set_vector(std::string filename, size_t size_valuetype, bool existfile)	{
		vector_infos.push_back(attribute_handler_info(filename, size_valuetype, existfile));

		size_total_valuetype += size_valuetype;
	}

	/* create attribute vector instances */
	void build_vectors(size_t * num_max_chunk)	{
		num_values = (((max_memory_mb * 1024 * 1024 / size_total_valuetype) >> OFFSET_BITMAP) + 1) << OFFSET_BITMAP;
		num_values = num_values >= num_nodes ? num_nodes : num_values;
		
		num_chunks = (num_nodes - 1) / num_values + 1;
		inmemory = num_chunks == 1 ? true : false;

		*num_max_chunk = num_chunks;

		num_pages = (num_nodes >> OFFSET_BITMAP) + 1;

		for (size_t vector_index = 0; vector_index < vector_infos.size(); vector_index++)	{
			vectors.push_back(new attribute_handler(vector_infos[vector_index], num_nodes, num_chunks, num_pages, num_values, edge_direction));
		}
	}

	/* initialize attribute vectors with configurations */
	void initialize_vectors() {
		pageflag_ptr = (bool *)malloc(num_pages);
		memset(pageflag_ptr, false, num_pages);
		lower_pageid = num_pages;
		upper_pageid = -1;

		for (size_t vector_index = 0; vector_index < vector_infos.size(); vector_index++) {
			vectors[vector_index]->set(pageflag_ptr);
		}
	}

	bitattribute_handler * get_bitvector(size_t vector_index)	{
		return bitvectors[vector_index];
	}

	void build_bitvector()	{
		bitattribute_handler * bitarray_ptr = new bitattribute_handler();
		bitarray_ptr->create(num_nodes);

		bitvectors.push_back(bitarray_ptr);

		max_memory_mb -= num_nodes / 8 / 1024 / 1024 + 1;
	}


	/* engine related functions */


	void set_pageflag(size_t first_nodeid, size_t last_nodeid)	{
		if (inmemory) return;

		size_t fpage_id = first_nodeid >> OFFSET_BITMAP;
		size_t lpage_id = last_nodeid >> OFFSET_BITMAP;

		for (size_t index_pageflag = fpage_id; index_pageflag <= lpage_id; index_pageflag++)	{
			pageflag_ptr[index_pageflag] = true;
		}

		if (lower_pageid > fpage_id) lower_pageid = fpage_id;
		if (upper_pageid < lpage_id) upper_pageid = lpage_id;
	}

	bool is_load_needed()	{
		for (size_t vector_index = 0; vector_index < vectors.size(); vector_index++)	{
			if (vectors[vector_index]->is_load_needed(lower_pageid, upper_pageid) == false) return false;
		}

		return true;
	}

	/* write current chunk to disk and read next chunk to memory */
	void exchange_chunks()	{
		for (size_t vector_index = 0; vector_index < vectors.size(); vector_index++)	{
			vectors[vector_index]->exchange_chunk();
		}

		src_lowerbound = 0;
		src_upperbound = num_nodes;
		dst_lowerbound = 0;
		dst_upperbound = num_nodes;

		for (size_t vector_index = 0; vector_index < vectors.size(); vector_index++)	{
			attribute_handler * attribute_ptr = vectors[vector_index];
			size_t lower = attribute_ptr->get_firstnode_number();
			size_t upper = attribute_ptr->get_lastnode_number();

			if (attribute_ptr->get_boundtype() == BOUNDTYPE_SRC)	{
				src_lowerbound = src_lowerbound < lower ? lower : src_lowerbound;
				src_upperbound = src_upperbound > upper ? upper : src_upperbound;
			}
			else {
				dst_lowerbound = dst_lowerbound < lower ? lower : dst_lowerbound;
				dst_upperbound = dst_upperbound > upper ? upper : dst_upperbound;
			}
		}
	}

	/* write current chunk to disk */
	void flush_chunks()	{
		for (size_t vector_index = 0; vector_index < vectors.size(); vector_index++)	{
			if (!vectors[vector_index]->is_inmemory())	{
				vectors[vector_index]->flush_chunk();
			}
		}

		memset(pageflag_ptr, false, num_pages);
		lower_pageid = num_pages;
		upper_pageid = -1;
	}


	/* local values related functions */


	bool is_inmemory()	{
		return inmemory;
	}

	size_t get_src_lowerbound()	{
		return src_lowerbound;
	}

	size_t get_src_upperbound()	{
		return src_upperbound;
	}

	size_t get_dst_lowerbound()	{
		return dst_lowerbound;
	}

	size_t get_dst_upperbound()	{
		return dst_upperbound;
	}
};

#endif