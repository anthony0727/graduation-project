#include <bitarray_handler.hpp>
#include <index_handler.hpp>
#include <wiss_handler.hpp>
#include <algorithm_handler.hpp>
#include <timer_handler.hpp>
#include <attribute_handler.hpp>

#include <thread>
#include <omp.h>

#include <boost/thread.hpp>
#include <boost/asio.hpp>
#include <boost/bind.hpp>

#include <sstream>

#define IN true
#define OUT false

// 블록 단위 처리 방식으로 사용할 엔진 헤더 파일임

#define MASK 1048575
//#define ANALYSIS_STATIC_HINDICATOR

#ifdef ANALYSIS_STATIC_HINDICATOR
#define ANALYSIS_STATIC_BASIC
#define ANALYSIS_STATIC_MAIN_BASIC
#define ANALYSIS_STATIC_MAIN_HTIME
#endif

//#define ANALYSIS_STATIC_WORKLOAD

#ifdef ANALYSIS_STATIC_WORKLOAD
#define ANALYSIS_STATIC_BASIC
#define ANALYSIS_STATIC_MAIN_BASIC
#define ANALYSIS_STATIC_MAIN_TIME
#define ANALYSIS_STATIC_WORKER_BASIC
#define ANALYSIS_STATIC_WORKER_COUNT
#define ANALYSIS_STATIC_WORKER_TIME
#endif

#ifdef ANALYSIS_STATIC_BASIC
#define NUM_NODES 0
#define NUM_EDGES 1
#define NUM_IOS 2

#define TIME_PROC 0
#define TIME_IO 1
#define TIME_ALLOC 2

#define TIME_SCAN 0
#define TIME_RESET 1
#endif

extern int num_maxbufs;

/* engine class */
class engine	{
private:
	index_handler index;					/* graph index and buffer index */
	bitarray_handler bitarray;				/* indicator vector with bit array */
	wiss_handler wiss;						/* graph data functions */
	information_handler information;		/* informations about graph processing */
	timer_handler timer;					/* execution time logging */
	algorithm_handler &algorithm;			/* custom graph algorithm */
	parameter_handler &parameter;			/* parameters from shell command */
	attribute_wrapper attribute;			/* vector space for algorithm */

	bool * needed_pagelist;
	std::vector<int> loaded_pagelist;		/* list of pages already in the buffer */
	std::vector<int> unloaded_pagelist;		/* list of pages not in the buffer */

	std::atomic<int> num_unfinished_tasks;	/* number of unfinished tasks with boost threads */

	boost::asio::io_service io_service;		/* callback thread pool of threads */
	boost::asio::io_service::work work;		/* needed to keep threads waiting instead of exiting when there is no work to do */
	boost::thread_group threads;			/* a thread group which process unloaded pages (boost implementation) */

	std::string volumename;					/* wiss volume name : device name which stores wiss relation of graph data */
	std::string filename;					/* wiss relation name : graph data pages of node adjacency lists which is stored in device*/
	std::string dataname;					/* original input filename : raw graph data of edgelists*/

	int cur_iteration;						/* number of current iteration */
	int max_iteration;						/* number of max iteration */

	int num_threads;						/* number of threads */

	size_t memory_total_mb;
	size_t memory_buffer_mb;
	size_t memory_vector_mb;

	size_t hindicator_partition;

	bool set_preprocess;					/* preprocessing on & off */
	bool set_enginerun;						/* just for testing preprocessing chunk */
	bool set_bitarray;						/* bitarray scheduling on & off */
	bool edge_direction;					/* edge direction in the page - IN : true, OUT : false */

	size_t dst_lowerbound;					/* lowerbound nodenumber to be processed for now */
	size_t dst_upperbound;					/* upperbound nodenumber to be processed for now */
	size_t src_lowerbound;					/* lowerbound edgenumber to be processed for now */
	size_t src_upperbound;					/* lowerbound edgenumber to be processed for now */

#ifdef ANALYSIS_STATIC_BASIC
	size_t ** counter;
	size_t * counter_;
	double ** stopwatch;
	double * stopwatch_;
	bool * threaded;
#endif

#ifdef ANALYSIS_STATIC_MAIN_BASIC
	size_t main_thread_id;
#endif

public:
	engine(algorithm_handler & _algorithm, parameter_handler & _parameter) : work(io_service), algorithm(_algorithm), parameter(_parameter)	{
		/* initialization of parameters */
		set_basic_parameters();

		num_unfinished_tasks = 0;

		omp_set_num_threads(num_threads);

		for (int i = 0; i < num_threads; ++i)
		{
			threads.create_thread(boost::bind(&boost::asio::io_service::run, &io_service));
		}

#ifdef ANALYSIS_STATIC_BASIC
		counter = new size_t*[(MASK + 1)];
		counter_ = new size_t[(MASK + 1) * 3];
		stopwatch = new double*[(MASK + 1)];
		stopwatch_ = new double[(MASK + 1) * 3];

		for (int i = 0; i < (MASK + 1); i++)	{
			counter[i] = &counter_[3 * i];
			stopwatch[i] = &stopwatch_[3 * i];
		}

		threaded = new bool[(MASK + 1)];

		memset(counter_, 0, sizeof(size_t) * (MASK + 1) * 3);
		memset(stopwatch_, 0, sizeof(double) * (MASK + 1) * 3);
		memset(threaded, false, sizeof(bool) * (MASK + 1));
#endif

	}

	~engine()	{
		io_service.stop();

		threads.join_all();

		timer.print_timelist();

#ifdef ANALYSIS_STATIC_BASIC
		delete counter_;
		delete counter;
		delete stopwatch_;
		delete stopwatch;
		delete threaded;
#endif

	}


	/* initializers */

	void set_basic_parameters()	{
		volumename = parameter.get_param_string("volume", "hp0a");
		filename = parameter.get_param_string("file", "livejournalrel");
		dataname = parameter.get_param_string("data", "livejournal_.txt");

		set_preprocess = parameter.get_param_int("preprocess", 0) == 1 ? true : false;
		set_enginerun = parameter.get_param_int("enginerun", 1) == 1 ? true : false;
		set_bitarray = parameter.get_param_int("bitarray", 1) == 1 ? true : false;
		edge_direction = parameter.get_param_string("edgeway", "in") == "in" ? IN : OUT;

		max_iteration = parameter.get_param_int("maxiters", set_bitarray ? INT_MAX : 20);

		num_threads = parameter.get_param_int("threads", boost::thread::hardware_concurrency());

		memory_total_mb = (size_t)parameter.get_param_int("memoryuse_mb", 1024);
		memory_vector_mb = memory_total_mb / 5;
		memory_buffer_mb = memory_total_mb * 4 / 5;

		hindicator_partition = parameter.get_param_int("hindicator_partition", 10);

		num_maxbufs = memory_buffer_mb;
	}

	void create_basic_components()	{
		/* load metadata of graph pages */
		index.load_graphindex(filename);

		/* create bitarray of indicator vector */
		if (set_bitarray)	{
			if (index.get_num_nodes() < (size_t(1) << hindicator_partition))	{
				index.unload_graphindex();
				exit(1);
			}

			bitarray.create(hindicator_partition, index.get_num_nodes());

			num_maxbufs -= (2 * (index.get_num_nodes() * (1.0 + 1.0 / (pow(2, hindicator_partition) - 1))) / 8 / 1048576);

			needed_pagelist = new bool[index.get_size_index()];
		}

		/* mount wiss with graph pages */
		wiss.initialize_wiss();
		wiss.mount_volume(volumename);
		wiss.open_file(filename);

		attribute.set_wrapper(memory_vector_mb, index.get_num_nodes(), edge_direction);
	}

	void destroy_basic_components()	{
		attribute.unset_wrapper();

		/* dismount wiss with graph pages */
		wiss.close_file();
		wiss.dismount_volume();
		wiss.finalize_wiss();

		/* destroy bitarray of indicator vector */
		if (set_bitarray)	{
			bitarray.destroy();

			delete needed_pagelist;
		}

		/* unload metadata of graph pages */
		index.unload_graphindex();
	}


	/* main function */
	void run()	{

#ifdef ANALYSIS_STATIC_MAIN_BASIC
		std::stringstream ss;
		ss << std::this_thread::get_id();
		ss >> main_thread_id;

		main_thread_id = main_thread_id & MASK;

		if (threaded[main_thread_id] == false) threaded[main_thread_id] = true;
#endif

		/* preprocess original graph data */
		if (set_preprocess)	{
			time_entry tp("preprocess");
			tp.start_time();
			wiss.initialize_wiss();
			wiss.preprocessing(volumename, filename, dataname);
			wiss.finalize_wiss();
			tp.stop_time();
			timer.add_entry(tp);
		}

		/* just to debug preprocess, no special meaning*/
		if (!set_enginerun) return;

		/* start engine runtime */
		time_entry t("runtime");
		t.start_time();

		/* initialization of basic components */
		create_basic_components();

		/* initialization of information */
		information.num_nodes = index.get_num_nodes();
		information.num_threads = num_threads;
		information.max_iteration = max_iteration;
		information.bitarray_ptr = &bitarray;
		information.parameter_ptr = &parameter;
		information.attribute_ptr = &attribute;
		information.wiss_ptr = &wiss;
		information.index_ptr = &index;

		/* before starting the process */
		algorithm.initialize(&information);

		for (cur_iteration = 0; cur_iteration < max_iteration; cur_iteration++)	{
			information.cur_iteration = cur_iteration;

			/* before starting the iteration */
			algorithm.before_iteration();

			/* start iteration */
			{
				/* convert bitarray to pagelist, pinned (if already in the buffer) and unpinned (if not) */
				pinpage();

				/* exit if no vertices are scheduled so that no page is listed in both pagelist */
				if (loaded_pagelist.size() == 0 && unloaded_pagelist.size() == 0) break;

				int num_alloced = 0;

#ifdef ANALYSIS_STATIC_MAIN_TIME
				std::chrono::system_clock::time_point start_1 = std::chrono::system_clock::now();
#endif

				if (attribute.is_inmemory())	{
					/* start processing loaded pages */
					executepage_threadpool(loaded_pagelist, 0, (int)loaded_pagelist.size());

					/* start loading unloaded pages */
					for (int num_processed_unloaded = 0; num_processed_unloaded < (int)unloaded_pagelist.size(); num_processed_unloaded += num_alloced)	{
						/* start loading unloaded pages */
						num_alloced = loadpage(num_processed_unloaded);
					}

					/* busy wait until every page is processed and unpinned */
					while (num_unfinished_tasks > 0);
				}
				else {
					if (loaded_pagelist.size() != 0)	{
						/* load first chunk of a vector */
						while (algorithm.request_nextchunk())	{
							if (attribute.is_load_needed() == false) continue;

#ifdef ANALYSIS_STATIC_MAIN_TIME
							std::chrono::system_clock::time_point start_2 = std::chrono::system_clock::now();
#endif

							attribute.exchange_chunks();

#ifdef ANALYSIS_STATIC_MAIN_TIME
							std::chrono::duration<double> interval_2 = std::chrono::system_clock::now() - start_2;

							stopwatch[main_thread_id][TIME_IO] += interval_2.count();
#endif

							//executepage_parallel(loaded_pagelist, 0, (int)loaded_pagelist.size());
							executepage_threadpool(loaded_pagelist, 0, (int)loaded_pagelist.size());

							while (num_unfinished_tasks > 0);
						}

#ifdef ANALYSIS_STATIC_MAIN_TIME
						std::chrono::system_clock::time_point start_2 = std::chrono::system_clock::now();
#endif

						attribute.flush_chunks();

#ifdef ANALYSIS_STATIC_MAIN_TIME
						std::chrono::duration<double> interval_2 = std::chrono::system_clock::now() - start_2;

						stopwatch[main_thread_id][TIME_IO] += interval_2.count();
#endif

						for (int i = 0; i < (int)loaded_pagelist.size(); i++)	{
							wiss.unpin_page(index.get_pageid(loaded_pagelist[i]), index.get_bufferindex(loaded_pagelist[i]));
						}
					}

					for (int num_processed_unloaded = 0; num_processed_unloaded < (int)unloaded_pagelist.size(); num_processed_unloaded += num_alloced)	{
						/* start loading unloaded pages */
						num_alloced = loadpage(num_processed_unloaded);

						while (num_unfinished_tasks > 0);

						while (algorithm.request_nextchunk())	{
							if (attribute.is_load_needed() == false) continue;

#ifdef ANALYSIS_STATIC_MAIN_TIME
							std::chrono::system_clock::time_point start_2 = std::chrono::system_clock::now();
#endif

							attribute.exchange_chunks();

#ifdef ANALYSIS_STATIC_MAIN_TIME
							std::chrono::duration<double> interval_2 = std::chrono::system_clock::now() - start_2;

							stopwatch[main_thread_id][TIME_IO] += interval_2.count();
#endif

							//executepage_parallel(unloaded_pagelist, num_processed_unloaded, num_processed_unloaded + num_alloced);
							executepage_threadpool(unloaded_pagelist, num_processed_unloaded, num_processed_unloaded + num_alloced);

							while (num_unfinished_tasks > 0);
						}

#ifdef ANALYSIS_STATIC_MAIN_TIME
						std::chrono::system_clock::time_point start_2 = std::chrono::system_clock::now();
#endif

						attribute.flush_chunks();

#ifdef ANALYSIS_STATIC_MAIN_TIME
						std::chrono::duration<double> interval_2 = std::chrono::system_clock::now() - start_2;

						stopwatch[main_thread_id][TIME_IO] += interval_2.count();
#endif

						for (int i = num_processed_unloaded; i < num_processed_unloaded + num_alloced; i++)	{
							wiss.release_page(index.get_pageid(unloaded_pagelist[i]), index.get_bufferindex(unloaded_pagelist[i]));
						}
					}
				}

#ifdef ANALYSIS_STATIC_MAIN_TIME
				std::chrono::duration<double> interval_1 = std::chrono::system_clock::now() - start_1;

				stopwatch[main_thread_id][TIME_ALLOC] += interval_1.count();
#endif

			}
			/* stop iteration */

			/* after stopping the iteration */
			algorithm.after_iteration();

			index.clear_pagemeta();

			/* one iteration ended, change the current & next iteration bitarrays */
			if (set_bitarray)	{

#ifdef ANALYSIS_STATIC_MAIN_HTIME
				std::chrono::system_clock::time_point start = std::chrono::system_clock::now();
#endif

				bitarray.exchange();

#ifdef ANALYSIS_STATIC_MAIN_HTIME
				std::chrono::duration<double> interval = std::chrono::system_clock::now() - start;

				stopwatch[main_thread_id][TIME_RESET] += interval.count();
#endif

			}

#ifdef ANALYSIS_STATIC_BASIC
			for (int i = 0; i < (MASK + 1); i++)	{
				if (threaded[i] == true)	{
					for (int j = 0; j < 3; j++)	{
						printf("%d %d %d %lu\n", i, j, cur_iteration, counter[i][j]);
						counter[i][j] = 0;
					}
					for (int j = 3; j < 6; j++)	{
						printf("%d %d %d %lf\n", i, j, cur_iteration, stopwatch[i][j - 3]);
						stopwatch[i][j - 3] = 0.0;
					}
				}
			}
#endif

		}

		/* after stopping the process */
		algorithm.finalize();

		/* finalization of basic components */
		destroy_basic_components();

		/* stop engine runtime */
		t.stop_time();

		timer.add_entry(t);
	}

	/* pin pages which are already loaded */
	void pinpage()	{
		int tablesize = index.get_size_index();
		char * pageptr = NULL;

		/* clear all pagelist */
		loaded_pagelist.clear();
		unloaded_pagelist.clear();

		if (set_bitarray)	{

#ifdef ANALYSIS_STATIC_MAIN_HTIME
			std::chrono::system_clock::time_point start = std::chrono::system_clock::now();
#endif

			/* group pagelist into pinned and unpinned */
#pragma omp parallel for schedule(dynamic, 1)
			for (int tableindex = 0; tableindex < tablesize; tableindex++)  {
				struct pageinfo pageinfo = index.get_pageinfo(tableindex);

				/* is there any node to process? */
				needed_pagelist[tableindex] = bitarray.get_curr_bits(pageinfo.first_nodeid, pageinfo.last_nodeid);
			}

#ifdef ANALYSIS_STATIC_MAIN_HTIME
			std::chrono::duration<double> interval = std::chrono::system_clock::now() - start;

			stopwatch[main_thread_id][TIME_SCAN] += interval.count();
#endif

		}

		/* group pagelist into pinned and unpinned */
		for (int tableindex = 0; tableindex < tablesize; tableindex++)	{
			if (set_bitarray && !needed_pagelist[tableindex]) continue;

			/* pin the required page if possible */
			wiss.pin_page(index.get_pageid(tableindex), &pageptr);

			/* if page is not loaded then get page number into the unloaded list */
			if (pageptr == NULL)	{
				unloaded_pagelist.push_back(tableindex);
			}
			/* else then get page number into the loaded list */
			else {
				loaded_pagelist.push_back(tableindex);

				attribute.set_pageflag(index.get_first_nodeid(tableindex), index.get_last_nodeid(tableindex));
			}

			/* set address of buffer where page is loaded */
			index.set_bufferindex(tableindex, pageptr);
		}
	}

	/* execute already loaded pages */
	void executepage_threadpool(std::vector<int> & pagelist, int startindex, int stopindex)	{
		if (edge_direction == IN)	{
			dst_lowerbound = attribute.get_dst_lowerbound();
			dst_upperbound = attribute.get_dst_upperbound();
			src_lowerbound = attribute.get_src_lowerbound();
			src_upperbound = attribute.get_src_upperbound();
		}
		else {
			dst_lowerbound = attribute.get_src_lowerbound();
			dst_upperbound = attribute.get_src_upperbound();
			src_lowerbound = attribute.get_dst_lowerbound();
			src_upperbound = attribute.get_dst_upperbound();
		}

		/* start posting unloaded pages */
		for (int index = startindex; index < stopindex; index++)	{
			num_unfinished_tasks++;

			/* post asynchronous io request to boost handler */
			io_service.post(boost::bind(&engine::handler_loaded, this, pagelist[index]));
		}
	}

	/* loaded handler */
	void handler_loaded(int tableindex)	{

#ifdef ANALYSIS_STATIC_WORKER_BASIC
		size_t thread_id;
		std::stringstream ss;
		ss << std::this_thread::get_id();
		ss >> thread_id;

		thread_id = thread_id & MASK;

		if (threaded[thread_id] == false) threaded[thread_id] = true;
#endif

#ifdef ANALYSIS_STATIC_WORKER_TIME
		std::chrono::system_clock::time_point start = std::chrono::system_clock::now();
#endif

		/* nodes in the page */
		struct pageinfo pageinfo = index.get_pageinfo(tableindex);
		char * page_ptr = index.get_bufferindex(tableindex);
		int connectedsize = index.get_connectedsize(tableindex);
		pagemeta * pagemeta_ptr = index.get_pagemeta_ptr(tableindex);

		/* process each node if node bit is scheduled */
		int * adjlist_ptr = NULL;
		int length = 0;

#ifdef ANALYSIS_STATIC_WORKER_COUNT
		size_t num_nodes = 0;
		size_t num_edges = 0;
#endif

		if (attribute.is_inmemory() == true)	{
			for (int nodeindex = pageinfo.first_nodeid; nodeindex <= pageinfo.last_nodeid; nodeindex++)	{
				if (!set_bitarray || bitarray.get_curr_bit(nodeindex))	{
					length = wiss.get_record(page_ptr, nodeindex - pageinfo.first_nodeid, (char **)&adjlist_ptr) / sizeof(EDGETYPE);

					algorithm.execute_node(nodeindex, adjlist_ptr, length, connectedsize, pagemeta_ptr);

#ifdef ANALYSIS_STATIC_WORKER_COUNT
					num_nodes++;
					num_edges += (size_t)length;
#endif

				}

				if (set_bitarray && (nodeindex & WORDFILTER) == WORDFILTER)     {
					for (nodeindex = nodeindex + 1; nodeindex < pageinfo.last_nodeid; nodeindex += 64)       {
						if (bitarray.get_curr_word(nodeindex >> WORDRANGE_LOG2) != 0) {
							break;
						}
					}

					nodeindex--;
				}
			}

			wiss.unpin_page(pageinfo.page_id, page_ptr);
		}
		else {
			int track_lower = pageinfo.first_nodeid < dst_lowerbound ? dst_lowerbound : pageinfo.first_nodeid;
			int track_upper = pageinfo.last_nodeid > dst_upperbound ? dst_upperbound : pageinfo.last_nodeid;
			int * firstindex = NULL;
			int * lastindex = NULL;

			for (int nodeindex = track_lower; nodeindex <= track_upper; nodeindex++)	{
				if (!set_bitarray || bitarray.get_curr_bit(nodeindex))	{
					length = wiss.get_record(page_ptr, nodeindex - pageinfo.first_nodeid, (char **)&adjlist_ptr) / sizeof(EDGETYPE);

					firstindex = std::lower_bound(adjlist_ptr, adjlist_ptr + length, (int)src_lowerbound);
					lastindex = std::upper_bound(adjlist_ptr, adjlist_ptr + length, (int)src_upperbound);

					algorithm.execute_node(nodeindex, firstindex, lastindex - firstindex, connectedsize, pagemeta_ptr);

#ifdef ANALYSIS_STATIC_WORKER_COUNT
					num_nodes++;
					num_edges += (size_t)(lastindex - firstindex);
#endif

				}

				if (set_bitarray && (nodeindex & WORDFILTER) == WORDFILTER)     {
					for (nodeindex = nodeindex + 1; nodeindex < track_upper; nodeindex += 64)       {
						if (bitarray.get_curr_word(nodeindex >> WORDRANGE_LOG2) != 0) {
							break;
						}
					}

					nodeindex--;
				}
			}
		}

		num_unfinished_tasks--;

#ifdef ANALYSIS_STATIC_WORKER_COUNT
		counter[thread_id][NUM_NODES] += num_nodes;
		counter[thread_id][NUM_EDGES] += num_edges;
#endif

#ifdef ANALYSIS_STATIC_WORKER_TIME
		std::chrono::duration<double> interval = std::chrono::system_clock::now() - start;

		stopwatch[thread_id][TIME_PROC] += interval.count();
#endif

	}

	void executepage_parallel(std::vector<int> & pagelist, int startindex, int stopindex)	{
		if (edge_direction == IN)	{
			dst_lowerbound = attribute.get_dst_lowerbound();
			dst_upperbound = attribute.get_dst_upperbound();
			src_lowerbound = attribute.get_src_lowerbound();
			src_upperbound = attribute.get_src_upperbound();
		}
		else {
			dst_lowerbound = attribute.get_src_lowerbound();
			dst_upperbound = attribute.get_src_upperbound();
			src_lowerbound = attribute.get_dst_lowerbound();
			src_upperbound = attribute.get_dst_upperbound();
		}

		/* start posting unloaded pages */
#pragma omp parallel for schedule(dynamic, 1)
		for (int idx = startindex; idx < stopindex; idx++)	{

#ifdef ANALYSIS_STATIC_WORKER_BASIC
			size_t thread_id;
			std::stringstream ss;
			ss << std::this_thread::get_id();
			ss >> thread_id;

			thread_id = thread_id & MASK;

			if (threaded[thread_id] == false) threaded[thread_id] = true;
#endif

#ifdef ANALYSIS_STATIC_WORKER_TIME
			std::chrono::system_clock::time_point start = std::chrono::system_clock::now();
#endif

			/* nodes in the page */
			int tableindex = pagelist[idx];
			struct pageinfo pageinfo = index.get_pageinfo(tableindex);
			char * page_ptr = index.get_bufferindex(tableindex);
			int connectedsize = index.get_connectedsize(tableindex);
			pagemeta * pagemeta_ptr = index.get_pagemeta_ptr(tableindex);

			/* process each node if node bit is scheduled */
			int * adjlist_ptr = NULL;
			int length = 0;

#ifdef ANALYSIS_STATIC_WORKER_COUNT
			size_t num_nodes = 0;
			size_t num_edges = 0;
#endif

			if (attribute.is_inmemory() == true)	{
				for (int nodeindex = pageinfo.first_nodeid; nodeindex <= pageinfo.last_nodeid; nodeindex++)	{
					if (!set_bitarray || bitarray.get_curr_bit(nodeindex))	{
						length = wiss.get_record(page_ptr, nodeindex - pageinfo.first_nodeid, (char **)&adjlist_ptr) / sizeof(EDGETYPE);

						algorithm.execute_node(nodeindex, adjlist_ptr, length, connectedsize, pagemeta_ptr);

#ifdef ANALYSIS_STATIC_WORKER_COUNT
						num_nodes++;
						num_edges += (size_t)length;
#endif

					}

					if (set_bitarray && (nodeindex & WORDFILTER) == WORDFILTER)     {
						for (nodeindex = nodeindex + 1; nodeindex < pageinfo.last_nodeid; nodeindex += 64)       {
							if (bitarray.get_curr_word(nodeindex >> WORDRANGE_LOG2) != 0) {
								break;
							}
						}

						nodeindex--;
					}
				}

				wiss.unpin_page(pageinfo.page_id, page_ptr);
			}
			else {
				int track_lower = pageinfo.first_nodeid < dst_lowerbound ? dst_lowerbound : pageinfo.first_nodeid;
				int track_upper = pageinfo.last_nodeid > dst_upperbound ? dst_upperbound : pageinfo.last_nodeid;
				int * firstindex = NULL;
				int * lastindex = NULL;

				for (int nodeindex = track_lower; nodeindex <= track_upper; nodeindex++)	{
					if (!set_bitarray || bitarray.get_curr_bit(nodeindex))	{
						length = wiss.get_record(page_ptr, nodeindex - pageinfo.first_nodeid, (char **)&adjlist_ptr) / sizeof(EDGETYPE);

						firstindex = std::lower_bound(adjlist_ptr, adjlist_ptr + length, (int)src_lowerbound);
						lastindex = std::upper_bound(adjlist_ptr, adjlist_ptr + length, (int)src_upperbound);

						algorithm.execute_node(nodeindex, firstindex, lastindex - firstindex, connectedsize, pagemeta_ptr);

#ifdef ANALYSIS_STATIC_WORKER_COUNT
						num_nodes++;
						num_edges += (size_t)(lastindex - firstindex);
#endif

					}

					if (set_bitarray && (nodeindex & WORDFILTER) == WORDFILTER)     {
						for (nodeindex = nodeindex + 1; nodeindex < track_upper; nodeindex += 64)       {
							if (bitarray.get_curr_word(nodeindex >> WORDRANGE_LOG2) != 0) {
								break;
							}
						}

						nodeindex--;
					}
				}

			}

#ifdef ANALYSIS_STATIC_WORKER_COUNT
			counter[thread_id][NUM_NODES] += num_nodes;
			counter[thread_id][NUM_EDGES] += num_edges;
#endif

#ifdef ANALYSIS_STATIC_WORKER_TIME
			std::chrono::duration<double> interval = std::chrono::system_clock::now() - start;

			stopwatch[thread_id][TIME_PROC] += interval.count();
#endif

		}
	}

	/* post request to process unloaded pages */
	int loadpage(int num_processed_unloaded)	{
		int listsize = (int)unloaded_pagelist.size();
		int tableindex;
		char * pageptr = NULL;

		/* busy wait until enough free space exists */
		while (wiss.get_freesize() < 1);

		/* get the free pages */
		int num_alloced = wiss.reserve_alloc_pages(&unloaded_pagelist[num_processed_unloaded], index.get_bufferindex_ptr(), listsize - num_processed_unloaded);

		/* start posting unloaded pages */
		for (int num_posted_unloaded = 0; num_posted_unloaded < num_alloced; num_posted_unloaded++)	{
			tableindex = unloaded_pagelist[num_processed_unloaded + num_posted_unloaded];
			pageptr = index.get_bufferindex(tableindex);

			/* fix the buffer place to load a page */
			wiss.reserve_lock_page(index.get_pageid(tableindex), &pageptr);

			attribute.set_pageflag(index.get_first_nodeid(tableindex), index.get_last_nodeid(tableindex));

			num_unfinished_tasks++;

			/* post asynchronous io request to boost handler */
			io_service.post(boost::bind(&engine::handler_unloaded, this, tableindex));
		}

		return num_alloced;
	}

	/* unloaded handler */
	void handler_unloaded(int tableindex)	{

#ifdef ANALYSIS_STATIC_WORKER_BASIC
		size_t thread_id;
		std::stringstream ss;
		ss << std::this_thread::get_id();
		ss >> thread_id;

		thread_id = thread_id & MASK;

		if (threaded[thread_id] == false) threaded[thread_id] = true;
#endif

#ifdef ANALYSIS_STATIC_WORKER_TIME
		std::chrono::system_clock::time_point start = std::chrono::system_clock::now();
#endif

		/* nodes in the page */
		struct pageinfo pageinfo = index.get_pageinfo(tableindex);
		char * page_ptr = index.get_bufferindex(tableindex);
		int connectedsize = index.get_connectedsize(tableindex);
		pagemeta * pagemeta_ptr = index.get_pagemeta_ptr(tableindex);

		/* file read is performed in this handling function */
		pread(wiss.get_devaddr(), page_ptr, PAGESIZE, (long)(((long)pageinfo.page_id) * PAGESIZE));

#ifdef ANALYSIS_STATIC_WORKER_TIME
		std::chrono::duration<double> interval_1 = std::chrono::system_clock::now() - start;

		stopwatch[thread_id][TIME_IO] += interval_1.count();
#endif

#ifdef ANALYSIS_STATIC_WORKER_COUNT
		counter[thread_id][NUM_IOS] += 1;

		size_t num_nodes = 0;
		size_t num_edges = 0;
#endif

		if (attribute.is_inmemory() == true)	{
			/* process each node if node bit is scheduled */
			int * adjlist_ptr = NULL;
			int length = 0;

			for (int nodeindex = pageinfo.first_nodeid; nodeindex <= pageinfo.last_nodeid; nodeindex++)	{
				if (!set_bitarray || bitarray.get_curr_bit(nodeindex))	{
					length = wiss.get_record(page_ptr, nodeindex - pageinfo.first_nodeid, (char **)&adjlist_ptr) / sizeof(EDGETYPE);

					algorithm.execute_node(nodeindex, adjlist_ptr, length, connectedsize, pagemeta_ptr);

#ifdef ANALYSIS_STATIC_WORKER_COUNT
					num_nodes++;
					num_edges += (size_t)length;
#endif

				}

				if (set_bitarray && (nodeindex & WORDFILTER) == WORDFILTER)     {
					for (nodeindex = nodeindex + 1; nodeindex < pageinfo.last_nodeid; nodeindex += 64)       {
						if (bitarray.get_curr_word(nodeindex >> WORDRANGE_LOG2) != 0) {
							break;
						}
					}

					nodeindex--;
				}
			}

			wiss.release_page(pageinfo.page_id, page_ptr);
		}

		num_unfinished_tasks--;

#ifdef ANALYSIS_STATIC_WORKER_COUNT
		counter[thread_id][NUM_NODES] += num_nodes;
		counter[thread_id][NUM_EDGES] += num_edges;
#endif

#ifdef ANALYSIS_STATIC_WORKER_TIME
		std::chrono::duration<double> interval_2 = std::chrono::system_clock::now() - start;

		stopwatch[thread_id][TIME_PROC] += interval_2.count();
#endif

	}
};
