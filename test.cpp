#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>

#define EDGETYPE int

bool less_edge(EDGETYPE i, EDGETYPE j)	{
	return i < j;
}

int get_node_id(EDGETYPE ev)	{
	return ev;
}

#include <engine.hpp>
#include <map>
#define bfs_type bool
#define pg_type float
#define wcc_type unsigned int
#define dfs_type unsigned int


#define score_type unsigned int
#define weight 1 
#define hscore 0

#define notvisited -1
#define evaluated -2

using namespace std;

class trianglecount_algorithm : public algorithm_handler {
private:
	information_handler * information_ptr;

	attribute_wrapper * attribute_ptr;
	bitarray_handler * bitarray_ptr;
	parameter_handler * parameter_ptr;
	wiss_handler * wiss_ptr;				// 그래프 자료구조 블록 내의 실제 데이터를 다루기 위한 포인터
	index_handler * index_ptr;				// 그래프 자료구조 블록들의 메타 정보를 다루기 위한 포인터

	time_entry t;

	char * pivot_block;
	int num_blocks;
    const int max_blocks = 3000; //mem_use 16G
    int block_idx = 0;

	int * childlist_ptr = NULL;
	int childlist_len = 0;	

    int n_id;

    atomic<int> trianglecount;

public:
	trianglecount_algorithm() : algorithm_handler(), t("engine") {
	}

	~trianglecount_algorithm() {
	}

	void initialize(information_handler * _information_ptr)	{
		t.start_time();

		/* setting pointer initialization */
		information_ptr = _information_ptr;

		attribute_ptr = information_ptr->attribute_ptr;
		bitarray_ptr = information_ptr->bitarray_ptr;
		parameter_ptr = information_ptr->parameter_ptr;
		wiss_ptr = information_ptr->wiss_ptr;
		index_ptr = information_ptr->index_ptr;

        trianglecount = 0;

		pivot_block = new char[PAGESIZE * max_blocks];

		num_blocks = index_ptr->get_size_index();

        pread(wiss_ptr->get_devaddr(), pivot_block, PAGESIZE, (long)(((long)index_ptr->get_pageid(block_idx)) * PAGESIZE));
	}

	void before_iteration()	{
        if(block_idx < num_blocks - 1) {
            if(block_idx > max_blocks - 1) {
                block_idx += max_blocks - 1;
            }

            if(index_ptr->get_last_nodeid(block_idx) < n_id) {
                block_idx++;
                pread(wiss_ptr->get_devaddr(), pivot_block, PAGESIZE, (long)(((long)index_ptr->get_pageid(block_idx)) * PAGESIZE));
            }

            childlist_len = wiss_ptr->get_record(pivot_block, n_id - index_ptr->get_first_nodeid(block_idx), (char **)&childlist_ptr) / sizeof(EDGETYPE);
        }
	}

	bool request_nextchunk()	{
		return false;
	}

    size_t count_intersection(int node_id, int * adjlist_ptr, int length, int start) {
        c_idx = 0;
        a_idx = start;
        size_t cnt = 0;

        while(a_idx < length && c_idx < childlist_len) {
            size_t dst = childlist_ptr[c_idx];
            size_t a = adjlist_ptr[a_idx];
            if(a == dst) {
                cnt++;
                a_idx++; c_idx++;
            } else {
                c_idx += a < dst;
                a_idx += a > dst;
            }
        }
    }

	void execute_node(int node_id, int * adjlist_ptr, int length, int connectedsize, pagemeta * pagemeta_ptr)	{
        int count = 0;
        if(noid_id < childlist_ptr[childlist_len - 1]) {
            for (int i = 0; i < length; i++) {
                if(adjlist_ptr[i] > node_id) {
                    count += count_intersection(node_id, adjlist_ptr, length, i);
                }
            }
        }
        atomic_fetch_add(&trianglecount, count);
	}


	void after_iteration()	{
        n_idx++;
	}

	void finalize()	{
		t.stop_time();
        cout << "trianglecount : " << trianglecount << endl;
	}
};

class dfs_algorithm : public algorithm_handler{
private:
	information_handler * information_ptr;

	attribute_wrapper * attribute_ptr;
	bitarray_handler * bitarray_ptr;
	parameter_handler * parameter_ptr;

	size_t startnode;

	bitattribute_handler * input_attribute;
	attribute_handler * next_attribute;
	attribute_handler * iter_attribute;

	size_t num_max_chunks;
	size_t next_inputchunk;
	size_t next_outputchunk;
	size_t current_chunk;

	std::atomic<int> total_visiting_nodes;
	std::atomic<int> top_stack;
	std::vector<int> visiting_log;
	bool executed;
	bool ispush;

	time_entry t;

public:
	dfs_algorithm() : algorithm_handler(), t("engine") {
	}

	~dfs_algorithm() {
	}
	void initialize(information_handler * _information_ptr) {
		printf("init start\n");
		t.start_time();

		/* setting pointer initialization */
		information_ptr = _information_ptr;

		attribute_ptr = information_ptr->attribute_ptr;
		bitarray_ptr = information_ptr->bitarray_ptr;
		parameter_ptr = information_ptr->parameter_ptr;

		attribute_ptr->build_bitvector();

		input_attribute = attribute_ptr->get_bitvector(0);

		attribute_ptr->set_vector("testvector1.txt", sizeof(int), false);
		attribute_ptr->set_vector("testvector2.txt", sizeof(int), false);

		/* attribute wrapper initialization */

		attribute_ptr->build_vectors(&num_max_chunks);

		next_attribute = attribute_ptr->get_vector(0);
		iter_attribute = attribute_ptr->get_vector(1);

		attribute_ptr->initialize_vectors();


		/* algorithm specific initialization */
		startnode = parameter_ptr->get_param_int("root", 0);
		input_attribute->set_bit(startnode);
		bitarray_ptr->set_curr_bit(startnode);
		top_stack = 1;
		next_attribute->set_value<int>((int)top_stack, startnode);
		printf("init complete\n");
	}

	bool request_nextchunk() {
		if(next_inputchunk == num_max_chunks) {
			next_inputchunk = 0;
			if(top_stack > 0) {
				if(!executed && top_stack > 0) {
					top_stack--;
					if(top_stack < next_attribute->get_firstnode_number()) {
						next_attribute->request_chunk(--current_chunk);
						next_attribute->exchange_chunk();
					}
				}
				bitarray_ptr->set_next_bit(next_attribute->get_value<dfs_type>(top_stack));
			}
			executed = false;
			return false;
		}
		iter_attribute->request_chunk(next_inputchunk);
		next_inputchunk++;
		return true;
	}


	void before_iteration()	{
		total_visiting_nodes = 0;
	}

	void execute_node(int node_id, int * adjlist_ptr, int length, int connectedsize, pagemeta * pagemeta_ptr)	{
		if(next_attribute->get_value<dfs_type>(top_stack) != node_id) return;

		top_stack--;
		executed = true;

		int visiting_nodes = 0;
		int last_value = -1;

		for (int index = 0; index < length; index++) {
			if(!input_attribute->get_bit(adjlist_ptr[index])) {
				input_attribute->set_bit(adjlist_ptr[index]);
				if(top_stack+1 > next_attribute->get_lastnode_number()) {
					next_attribute->request_chunk(++current_chunk);
					next_attribute->exchange_chunk();
				}
				next_attribute->set_value(++top_stack, adjlist_ptr[index]);
				visiting_nodes++;
				last_value = adjlist_ptr[index];
			}
		}
		if(top_stack > 0) {
			bitarray_ptr->set_next_bit(next_attribute->get_value<dfs_type>(top_stack));
		}
		atomic_fetch_add(&total_visiting_nodes, visiting_nodes);
	}

	int total_sum = 0;
	void after_iteration()	{
		visiting_log.push_back(total_visiting_nodes.load());
		total_sum += total_visiting_nodes.load();
	}


	void finalize()	{
		t.stop_time();

		printf("total : %d\n",total_sum);

		FILE * fp = fopen(parameter_ptr->get_param_string("file", "test_result.txt").c_str(), "at");

		fprintf(fp, "startnode %d\n", (int)startnode);
		for (int i = 0; i < (int)visiting_log.size(); i++)	{
			fprintf(fp, " %d iteration visited %d nodes\n", i, visiting_log[i]);
		}
		fprintf(fp, "time spent %lf\n\n", t.runtime().count());

		fclose(fp);
	}
	

};

class bfs_algorithm : public algorithm_handler {
private:
	information_handler * information_ptr;

	attribute_wrapper * attribute_ptr;
	bitarray_handler * bitarray_ptr;
	parameter_handler * parameter_ptr;

	size_t startnode;

	bitattribute_handler * input_attribute;

	std::atomic<int> total_visiting_nodes;
	std::vector<int> visiting_log;

	time_entry t;
public:
	bfs_algorithm() : algorithm_handler(), t("engine") {
	}

	~bfs_algorithm() {
	}

	void initialize(information_handler * _information_ptr)	{
		t.start_time();

		/* setting pointer initialization */
		information_ptr = _information_ptr;

		attribute_ptr = information_ptr->attribute_ptr;
		bitarray_ptr = information_ptr->bitarray_ptr;
		parameter_ptr = information_ptr->parameter_ptr;

		/* attribute wrapper initialization */
		attribute_ptr->build_bitvector();

		/* input attribute vector initialization */
		input_attribute = attribute_ptr->get_bitvector(0);

		/* algorithm specific initialization */
		startnode = parameter_ptr->get_param_int("root", 0);
		input_attribute->set_bit(startnode);
		bitarray_ptr->set_curr_bit(startnode);
	}

	void before_iteration()	{
		total_visiting_nodes = 0;
	}

	bool request_nextchunk()	{
		return false;
	}

	void execute_node(int node_id, int * adjlist_ptr, int length, int connectedsize, pagemeta * pagemeta_ptr)	{
		int visiting_nodes = 0;

		for (int index = 0; index < length; index++)	{
			if (!input_attribute->get_bit(adjlist_ptr[index]))	{
				if (input_attribute->set_bit(adjlist_ptr[index]))	{
					bitarray_ptr->set_next_bit(adjlist_ptr[index]);
					visiting_nodes++;
				}
			}
		}

		atomic_fetch_add(&total_visiting_nodes, visiting_nodes);
	}

	void after_iteration()	{
		visiting_log.push_back(total_visiting_nodes.load());
	}

	void finalize()	{
		t.stop_time();

		FILE * fp = fopen(parameter_ptr->get_param_string("file", "test_result.txt").c_str(), "at");

		fprintf(fp, "startnode %d\n", (int)startnode);
		for (int i = 0; i < (int)visiting_log.size(); i++)	{
			fprintf(fp, " %d iteration visited %d nodes\n", i, visiting_log[i]);
		}
		fprintf(fp, "time spent %lf\n\n", t.runtime().count());

		fclose(fp);
	}
};




class astar_algorithm : public algorithm_handler {
private:
	information_handler * information_ptr;

	attribute_wrapper * attribute_ptr;
	bitarray_handler * bitarray_ptr;
	parameter_handler * parameter_ptr;

	attribute_handler * node_attribute ;
	attribute_handler * gscore_attribute;
	attribute_handler * camefrom_attribute;

	size_t num_max_chunks;
	size_t next_inputchunk;
	size_t next_outputchunk;

	size_t start;
	size_t goal;
    int next_nodeid = notvisited;

    int upperbound = -1;

	std::vector<size_t> shortest_path;

	time_entry t;

public:

	astar_algorithm() : algorithm_handler(), t("engine") {
	}

	~astar_algorithm() {
	}
	void initialize(information_handler * _information_ptr)	{

		std::cout << "init" << std::endl;
		t.start_time();

		/* setting pointer initialization */
		information_ptr = _information_ptr;

		attribute_ptr = information_ptr->attribute_ptr;
		bitarray_ptr = information_ptr->bitarray_ptr;
		parameter_ptr = information_ptr->parameter_ptr;

		/* attribute wrapper initialization */
	    attribute_ptr->set_vector("testvector1.txt", sizeof(int), false);
        attribute_ptr->set_vector("testvector2.txt", sizeof(score_type), false);
        attribute_ptr->set_vector("testvector3.txt", sizeof(size_t), false);
		attribute_ptr->build_vectors(&num_max_chunks);

		next_inputchunk = 0;
		next_outputchunk = 0;


		/* input attribute vector initialization */

		node_attribute = attribute_ptr->get_vector(0);
		gscore_attribute = attribute_ptr->get_vector(1);
		camefrom_attribute = attribute_ptr->get_vector(2);
       
		/* algorithm specific initialization */
		start = parameter_ptr->get_param_int("root", 2);
       	goal = parameter_ptr->get_param_int("goal", /*rand*/3);

		bitarray_ptr->set_curr_bit(start);

		node_attribute->set_default_value(notvisited);
		node_attribute->set_writefile(true);

		gscore_attribute->set_default_value(0);
        gscore_attribute->set_writefile(true);

		camefrom_attribute->set_default_value(0);
        camefrom_attribute->set_writefile(true);

		attribute_ptr->initialize_vectors();

        shortest_path.push_back(goal);
        upperbound = start;

        std::cout << "max chunk : " << num_max_chunks << std::endl;
	}
    void before_iteration() {

        if(start == goal) { 
            std::cout << "start and goal are equal" << std::endl;
            exit(1);
        }

        next_nodeid = notvisited;
    }

	bool request_nextchunk()	{
		/* chunk id checking */
		if (next_inputchunk == num_max_chunks) {
			next_inputchunk = 0;
			next_outputchunk++;

			if (next_outputchunk == num_max_chunks) {
				next_outputchunk = 0;

				return false;
			}
		}

		/* request a chunk of each attribute */
        node_attribute->request_chunk(next_inputchunk);
        gscore_attribute->request_chunk(next_inputchunk);
        camefrom_attribute->request_chunk(next_inputchunk);

		next_inputchunk++;

		return true;
	}
    void execute_node(int node_id, int * adjlist_ptr, int length, int connectedsize, pagemeta * pagemeta_ptr)   {

        score_type fscore = 0;
        score_type gscore = 0;


        if(node_id > upperbound) upperbound = node_id;

        if(node_id >= node_attribute->get_firstnode_number() && node_id <= node_attribute->get_lastnode_number()) {

		//printf("id : %d fscore : %d bound : %d chunk : %zu first %zu/last %zu\n",node_id,node_attribute->get_value<int>(node_id),upperbound,next_inputchunk,node_attribute->get_firstnode_number(),node_attribute->get_lastnode_number());
            node_attribute->set_value<int>(node_id, evaluated);
            gscore = gscore_attribute->get_value<score_type>(node_id) + weight;
        }

        for(int index = 0 ; index < length ; index++) {

            int neighbor_id = adjlist_ptr[index]; 
            if(neighbor_id > upperbound) upperbound = neighbor_id;
            
            if(neighbor_id >= node_attribute->get_firstnode_number() && neighbor_id <= node_attribute->get_lastnode_number()) {

                if(node_attribute->get_value<int>(neighbor_id) == evaluated) continue;

                if(node_attribute->get_value<int>(neighbor_id) == notvisited) {

                    fscore = gscore + hscore;

                    node_attribute->set_value<int>(neighbor_id, fscore);

                } else if(gscore >= gscore_attribute->get_value<score_type>(neighbor_id)) continue;

                gscore_attribute->set_value<score_type>(neighbor_id, gscore);

                camefrom_attribute->set_value<size_t>(neighbor_id, node_id);
            }
        }

        int tmp_fscore = INT_MAX;
        int value = 0;
        int value_id = upperbound;

        for( ; value_id > 0 ; value_id--) {

            if(value_id >= node_attribute->get_firstnode_number()) { 

                value = node_attribute->get_value<int>(value_id);

                if(value > 0 && value <= tmp_fscore) {

                    tmp_fscore = value;
                    next_nodeid = value_id;

                }   
            }
        }
    }

    void after_iteration() {


        if(next_nodeid == notvisited) { 

            std::cout << "not connected" << std::endl;

        } else if(next_nodeid == goal) {

            std::cout << "found goal!" << std::endl;

        } else {

            bitarray_ptr->set_next_bit(next_nodeid);

        }
    }

    void finalize() {
        // FILE * fp = fopen("astar_result","w");
	FILE * fp = fopen(parameter_ptr->get_param_string("file", "test_result.txt").c_str(), "at");
        size_t j = 0;
        
	for(size_t i = goal ; i == start ; i = j) {
                j = camefrom_attribute->get_value<size_t>(i);
                shortest_path.push_back(j);
	}

        if(shortest_path.size() <= 2) {
		fprintf(fp, "not connected");
		fprintf(fp, "\n");
        } else {
		for(int i = shortest_path.size() - 1 ; i > 0 ; i--) {
			fprintf(fp, "%lu -> ", shortest_path[i]);
		}
        }

        t.stop_time();
 
        fprintf(fp, " time spent %lf\n\n", t.runtime().count());
    }

};



class undirect_wcc_algorithm : public algorithm_handler {
private:
	information_handler * information_ptr;

	attribute_wrapper * attribute_ptr;
	bitarray_handler * bitarray_ptr;
	parameter_handler * parameter_ptr;

	size_t startnode;

    attribute_handler * label_attribute;

    size_t num_max_chunks;
    size_t next_inputchunk;
    size_t curr_arrayindex;
    size_t num_nodes;

    bool changed=false;

    std::atomic<int> total_visiting_nodes;
	time_entry t;
public:
	undirect_wcc_algorithm() : algorithm_handler(), t("engine") {
	}

	~undirect_wcc_algorithm() {
	}

	void initialize(information_handler * _information_ptr)	{
		t.start_time();

		/* setting pointer initialization */
		information_ptr = _information_ptr;

		attribute_ptr = information_ptr->attribute_ptr;
		bitarray_ptr = information_ptr->bitarray_ptr;
		parameter_ptr = information_ptr->parameter_ptr;

		/* attribute wrapper initialization */
      		attribute_ptr->set_vector("testvector1.txt", sizeof(wcc_type), true);
		attribute_ptr->build_vectors(&num_max_chunks);

		/* input attribute vector initialization */
      		  label_attribute = attribute_ptr->get_vector(0);

        
		/* algorithm specific initialization */
		label_attribute->set_writefile(true);
       		label_attribute->set_default_value<wcc_type>(0);

       		 attribute_ptr->initialize_vectors();
 

	for (size_t chunk_index = 0; chunk_index < num_max_chunks; chunk_index++) {
		label_attribute->read_chunks(chunk_index);
 		for (int node_id = label_attribute->get_firstnode_number(); node_id <= label_attribute->get_lastnode_number(); node_id++) {
			bitarray_ptr->set_curr_bit(node_id);
			label_attribute->set_value<wcc_type>(node_id,(wcc_type)(node_id));
		}
	}

	num_nodes = information_ptr->num_nodes;
	printf("initialize complete...\n");
	}

	void before_iteration()	{

		total_visiting_nodes=0;
	}


	bool request_nextchunk()	{
        if(next_inputchunk == num_max_chunks){
            next_inputchunk=0;
            return false;
        }

        label_attribute->request_chunk(next_inputchunk);
        next_inputchunk++;

        return true;


	}

	void execute_node(int node_id, int * adjlist_ptr, int length, int connectedsize, pagemeta * pagemeta_ptr)	{
       int visiting_nodes=0;


		for (int index = 0; index < length; index++)	{
			if(label_attribute->get_value<wcc_type>(node_id) > label_attribute->get_value<wcc_type>(adjlist_ptr[index])){
				label_attribute->set_value<wcc_type>(node_id, label_attribute->get_value<wcc_type>(adjlist_ptr[index]));
				bitarray_ptr->set_next_bit(node_id);
				visiting_nodes++;

			}
			else if(label_attribute->get_value<wcc_type>(node_id) < label_attribute->get_value<wcc_type>(adjlist_ptr[index])){
				label_attribute->set_value<wcc_type>(adjlist_ptr[index], label_attribute->get_value<wcc_type>(node_id));
				bitarray_ptr->set_next_bit(adjlist_ptr[index]);
				visiting_nodes++;
			}



		}

			atomic_fetch_add(&total_visiting_nodes, visiting_nodes);
	}

	void after_iteration()	{

		
	}

	void finalize()	{
		t.stop_time();

        map<unsigned int, int> wcc_map;
        map<unsigned int, int>::iterator it;
        int size_one_component = 0;
	int total_component = 0;
		for (size_t chunk_index = 0; chunk_index < num_max_chunks; chunk_index++) {
			label_attribute->read_chunks(chunk_index);
			for (size_t node_id = label_attribute->get_firstnode_number(); node_id <= label_attribute->get_lastnode_number(); node_id++) {
                wcc_map[label_attribute->get_value<wcc_type>(node_id)]++;

            }
        }
        for(it=wcc_map.begin(); it != wcc_map.end(); it++){
    	    total_component++;
            if(it->second == 1)
                size_one_component++;
        }

	FILE * fp = fopen(parameter_ptr->get_param_string("file", "test_result.txt").c_str(), "at");


        fprintf(fp, "total_components : %d\n", total_component);
        fprintf(fp, "size one component : %d\n", size_one_component);
        fprintf(fp, "number of components without components whose sizes are 1 : %d\n",total_component- size_one_component);
	fprintf(fp, "time spent %lf\n\n", t.runtime().count());
	
	fclose(fp);

	
        printf("total_components : %d\n", total_component);
        printf("size one component : %d\n", size_one_component);
        printf( "number of components without components whose sizes are 1 : %d\n",total_component- size_one_component);
	printf("time spent %lf\n\n", t.runtime().count());


	}
};











class wcc_algorithm : public algorithm_handler {
private:
	information_handler * information_ptr;

	attribute_wrapper * attribute_ptr;
	bitarray_handler * bitarray_ptr;
	parameter_handler * parameter_ptr;

	size_t startnode;

	bitattribute_handler * changed_attribute;
    attribute_handler * label_attribute;

    size_t num_max_chunks;
    size_t next_inputchunk;
    size_t curr_arrayindex;
    size_t num_nodes;

    bool changed=false;

    std::atomic<int> total_visiting_nodes;
	time_entry t;
public:
	wcc_algorithm() : algorithm_handler(), t("engine") {
	}

	~wcc_algorithm() {
	}

	void initialize(information_handler * _information_ptr)	{
		t.start_time();

		/* setting pointer initialization */
		information_ptr = _information_ptr;

		attribute_ptr = information_ptr->attribute_ptr;
		bitarray_ptr = information_ptr->bitarray_ptr;
		parameter_ptr = information_ptr->parameter_ptr;

		/* attribute wrapper initialization */
	attribute_ptr->build_bitvector();
        attribute_ptr->set_vector("testvector1.txt", sizeof(wcc_type), true);
        attribute_ptr->build_vectors(&num_max_chunks);

		/* input attribute vector initialization */
	changed_attribute = attribute_ptr->get_bitvector(0);
        label_attribute = attribute_ptr->get_vector(0);

        
		/* algorithm specific initialization */
		label_attribute->set_writefile(true);
        label_attribute->set_default_value<wcc_type>(0);

        attribute_ptr->initialize_vectors();
 

	for (size_t chunk_index = 0; chunk_index < num_max_chunks; chunk_index++) {
		label_attribute->read_chunks(chunk_index);
 		for (int node_id = label_attribute->get_firstnode_number(); node_id <= label_attribute->get_lastnode_number(); node_id++) {
			bitarray_ptr->set_curr_bit(node_id);
			label_attribute->set_value<wcc_type>(node_id,(wcc_type)(node_id));
		}
	}

	num_nodes = information_ptr->num_nodes;
	printf("initialize complete...\n");
	}

	void before_iteration()	{

		total_visiting_nodes=0;
	}


	bool request_nextchunk()	{
        if(next_inputchunk == num_max_chunks){
            next_inputchunk=0;
            return false;
        }

        label_attribute->request_chunk(next_inputchunk);
        next_inputchunk++;

        return true;


	}

	void execute_node(int node_id, int * adjlist_ptr, int length, int connectedsize, pagemeta * pagemeta_ptr)	{
       int visiting_nodes=0;


		for (int index = 0; index < length; index++)	{
			if(label_attribute->get_value<wcc_type>(node_id) > label_attribute->get_value<wcc_type>(adjlist_ptr[index])){
				label_attribute->set_value<wcc_type>(node_id, label_attribute->get_value<wcc_type>(adjlist_ptr[index]));
				bitarray_ptr->set_next_bit(node_id);
			//	index=-1;
				changed_attribute->set_bit(node_id);
				changed=true;
				visiting_nodes++;

			}
			else if(label_attribute->get_value<wcc_type>(node_id) < label_attribute->get_value<wcc_type>(adjlist_ptr[index])){
				label_attribute->set_value<wcc_type>(adjlist_ptr[index], label_attribute->get_value<wcc_type>(node_id));
				bitarray_ptr->set_next_bit(adjlist_ptr[index]);
				changed_attribute->set_bit(adjlist_ptr[index]);
				changed=true;
				visiting_nodes++;
			}



		}

			atomic_fetch_add(&total_visiting_nodes, visiting_nodes);
	}

	void after_iteration()	{
        if(total_visiting_nodes==0){
			if(changed){
				changed=false;
				for(int node=0; node<num_nodes; node++){
					if(!changed_attribute->get_bit(node)){
						bitarray_ptr->set_next_bit(node);
					}
				}
			}
	
		changed_attribute->clear();	
		}

		
	}

	void finalize()	{
		t.stop_time();

        map<unsigned int, int> wcc_map;
        map<unsigned int, int>::iterator it;
        int size_one_component = 0;
	int total_component = 0;
		for (size_t chunk_index = 0; chunk_index < num_max_chunks; chunk_index++) {
			label_attribute->read_chunks(chunk_index);
			for (size_t node_id = label_attribute->get_firstnode_number(); node_id <= label_attribute->get_lastnode_number(); node_id++) {
                wcc_map[label_attribute->get_value<wcc_type>(node_id)]++;

            }
        }
        for(it=wcc_map.begin(); it != wcc_map.end(); it++){
    	    total_component++;
            if(it->second == 1)
                size_one_component++;
        }


	FILE * fp = fopen(parameter_ptr->get_param_string("file", "test_result.txt").c_str(), "at");


        fprintf(fp, "total_components : %d\n", total_component);
        fprintf(fp, "size one component : %d\n", size_one_component);
        fprintf(fp, "number of components without components whose sizes are 1 : %d\n",total_component- size_one_component);
	fprintf(fp, "time spent %lf\n\n", t.runtime().count());
	
	fclose(fp);
	
        printf("total_components : %d\n", total_component);
        printf("size one component : %d\n", size_one_component);
        printf( "number of components without components whose sizes are 1 : %d\n",total_component- size_one_component);
	printf("time spent %lf\n\n", t.runtime().count());


	}
};

class pg_algorithm : public algorithm_handler {
private:
	information_handler * information_ptr;

	attribute_wrapper * attribute_ptr;
	bitarray_handler * bitarray_ptr;
	parameter_handler * parameter_ptr;

	attribute_handler * input_attribute;
	attribute_handler * output_attribute;
	attribute_handler * outdegree_attribute;

	size_t num_max_chunks;
	size_t next_inputchunk;
	size_t next_outputchunk;

	float initial_prvalue;
	float damped_prvalue;

	bool first_iteration;
	bool last_iteration;

	time_entry t;
public:
	pg_algorithm() : algorithm_handler(), t("engine") {
	}

	~pg_algorithm() {
	}

	void initialize(information_handler * _information_ptr)	{
		t.start_time();

		/* setting pointer initialization */
		information_ptr = _information_ptr;

		attribute_ptr = information_ptr->attribute_ptr;
		bitarray_ptr = information_ptr->bitarray_ptr;
		parameter_ptr = information_ptr->parameter_ptr;

		/* attribute wrapper initialization */
		attribute_ptr->set_vector("testvector1.txt", sizeof(pg_type), false);
		attribute_ptr->set_vector("testvector2.txt", sizeof(pg_type), false);
		attribute_ptr->set_vector(parameter_ptr->get_param_string("outdegreefile", "yahoo.indegree"), sizeof(int), true);
		attribute_ptr->build_vectors(&num_max_chunks);

		next_inputchunk = 0;
		next_outputchunk = 0;

		/* algorithm specific initialization */
		initial_prvalue = 1.0f / information_ptr->num_nodes;
		damped_prvalue = initial_prvalue * 0.15f;

		/* attribute vector initialization */
		input_attribute = attribute_ptr->get_vector(0);
		output_attribute = attribute_ptr->get_vector(1);
		outdegree_attribute = attribute_ptr->get_vector(2);

		input_attribute->set_boundtype(BOUNDTYPE_SRC);
		input_attribute->set_writefile(false);
		input_attribute->set_default_value<pg_type>(0.0);

		outdegree_attribute->set_boundtype(BOUNDTYPE_DST);
		outdegree_attribute->set_writefile(false);

		output_attribute->set_boundtype(BOUNDTYPE_DST);
		output_attribute->set_writefile(true);

		attribute_ptr->initialize_vectors();
	}

	void before_iteration()	{
		first_iteration = information_ptr->cur_iteration == 0;
		last_iteration = information_ptr->cur_iteration == (information_ptr->max_iteration - 1);
	}

	bool request_nextchunk()	{
		/* chunk id checking */
		if (next_inputchunk == num_max_chunks) {
			next_inputchunk = 0;
			next_outputchunk++;

			if (next_outputchunk == num_max_chunks) {
				next_outputchunk = 0;

				return false;
			}
		}

		/* request a chunk of each attribute */
		input_attribute->request_chunk(next_inputchunk);
		outdegree_attribute->request_chunk(next_outputchunk);
		output_attribute->request_chunk(next_outputchunk);

		next_inputchunk++;

		return true;
	}

	void execute_node(int node_id, int * adjlist_ptr, int length, int connectedsize, pagemeta * pagemeta_ptr) {
		float sum = 0.0;

		for (int index = 0; index < length; index++) {
			sum += input_attribute->get_value<pg_type>(adjlist_ptr[index]);
		}

		int outdegree = outdegree_attribute->get_value<int>(node_id);

		if (connectedsize > 1) {
			output_attribute->lock_value<pg_type>(&pagemeta_ptr->lock);

			if (pagemeta_ptr->num_count == 0) {
				output_attribute->set_value<pg_type>(node_id, ((first_iteration ? initial_prvalue : damped_prvalue) + 0.85 * sum) / (last_iteration ? 1 : outdegree));
				pagemeta_ptr->num_count++;
			}
			else
				output_attribute->add_value<pg_type>(node_id, (0.85 * sum) / (last_iteration ? 1 : outdegree));

			output_attribute->unlock_value<pg_type>(&pagemeta_ptr->lock);
		}
		else
		{
			if (input_attribute->get_chunk_id() == 0)	{
				output_attribute->set_value<pg_type>(node_id, ((first_iteration ? initial_prvalue : damped_prvalue) + 0.85 * sum) / ((last_iteration || outdegree == 0) ? 1 : outdegree));
			}
			else {
				output_attribute->add_value<pg_type>(node_id, (0.85 * sum) / ((last_iteration || outdegree == 0) ? 1 : outdegree));
			}
		}
	}

	void after_iteration()	{
		attribute_handler * _ptr = input_attribute;
		input_attribute = output_attribute;
		output_attribute = _ptr;

		input_attribute->set_boundtype(BOUNDTYPE_SRC);
		input_attribute->set_writefile(false);

		output_attribute->set_boundtype(BOUNDTYPE_DST);
		output_attribute->set_writefile(true);
	}

	void finalize()	{
		t.stop_time();

		pg_type toplist[40];
		memset(toplist, 0, 40 * sizeof(pg_type));

		for (size_t chunk_index = 0; chunk_index < num_max_chunks; chunk_index++)	{
			input_attribute->read_chunks(chunk_index);

			input_attribute->sort_chunk<pg_type>();

			input_attribute->get_first20<pg_type>(&toplist[20]);

			std::sort(toplist, toplist + 40, std::greater<float>());
		}

		FILE * fp = fopen(parameter_ptr->get_param_string("file", "test_result.txt").c_str(), "at");
		fprintf(fp, "\npagerank result\n");
		for (size_t i = 0; i < 20; i++)	{
			fprintf(fp, "%lu\t%.9lf\n", i, toplist[i]);
		}
		fprintf(fp, "time spent %lf\n\n", t.runtime().count());
		fclose(fp);
	}
};

int main(int argc, char ** argv)	{
	parameter_handler param(argc, argv);
	std::string param_algorithm = param.get_param_string("algorithm", "wcc");

	if (param_algorithm == "bfs"){
		bfs_algorithm alg;
		engine e(alg, param);

		e.run();
	}
	else if (param_algorithm == "wcc"){
		wcc_algorithm alg;
		engine e(alg, param);

		e.run();
	}
	else if (param_algorithm == "undirect_wcc")	{
		undirect_wcc_algorithm alg;
		engine e(alg, param);

		e.run();
	}
	else if (param_algorithm == "astar")	{
		astar_algorithm alg;
		engine e(alg, param);

		e.run();
	}

	else if (param_algorithm == "pg")	{
		pg_algorithm alg;
		engine e(alg, param);

		e.run();
	}
	else if (param_algorithm == "dfs")	{
		dfs_algorithm alg;
		engine e(alg, param);

		e.run();
	}

	return 0;
}
