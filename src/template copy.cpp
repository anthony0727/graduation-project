#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <engine.hpp>

/* 이터레이션마다 그래프 자료구조의 모든 블록들을 한 번씩 읽는 코드 샘플 */

class temp_algorithm : public algorithm_handler {
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
public:
	temp_algorithm() : algorithm_handler(), t("engine") {
	}

	~temp_algorithm() {
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

		/* 그래프 자료구조 블록을 직접 읽어오기 */

		// 블록 임시 저장 공간 할당, PAGESIZE: 블록 하나의 크기
		pivot_block = new char[PAGESIZE];

		// 그래프 데이터가 차지하는 총 블록 개수 가져옴
		num_blocks = index_ptr->get_size_index();

		for (int block_index = 0; block_index < num_blocks; block_index++) {
			// 여러 개의 스레드가 같은 파일 디스크립터를 사용하기 때문에 파일 오프셋을 변경하지 않는 pread를 사용함
			pread(wiss_ptr->get_devaddr(), pivot_block, PAGESIZE, (long)(((long)index_ptr->get_pageid(block_index)) * PAGESIZE));

			int * adjlist_ptr = NULL;	// 노드 인접리스트의 시작 위치를 저장하기 위한 포인터
			int length = 0;				// 노드 인접리스트의 길이를 저장하기 위한 변수

			for (int nodeindex = index_ptr->get_first_nodeid(block_index); nodeindex <= index_ptr->get_last_nodeid(block_index); nodeindex++)	{
				// get_record 함수는 해당 블록 내에서 지정한 노드의 인접리스트가 차지하는 바이트 수와 그 시작 위치를 알려줌
				// 따라서 바이트 수를 한 인접 노드가 차지하는 바이트 수로 나눠 주어야 정확한 길이를 알 수 있음

				length = wiss_ptr->get_record(pivot_block, nodeindex - index_ptr->get_first_nodeid(block_index), (char **)&adjlist_ptr) / sizeof(EDGETYPE);
                for (int i = 0; i < length; i++) {
                    
                }
				length = wiss_ptr->get_record(pivot_block, nodeindex - index_ptr->get_first_nodeid(block_index), (char **)&adjlist_ptr) / sizeof(EDGETYPE);

				// 그래프 알고리즘 수행
				// nodeindex: 노드 번호
				// adjlist_ptr: 인접리스트 포인터
				// length: 인접리스트 길이
				// connectedsize: 해당 노드의 인접리스트가 저장된 블록의 총 개수
				// pagemeta: 여러 블록에 저장된 인접리스트일 경우 스레드 간 충돌 방지용으로 사용하는 락
				execute_node(nodeindex, adjlist_ptr, length, index_ptr->get_connectedsize(block_index), index_ptr->get_pagemeta_ptr(block_index));
			}
		}
	}

	void before_iteration()	{
	}

	bool request_nextchunk()	{
		return false;
	}

	void execute_node(int node_id, int * adjlist_ptr, int length, int connectedsize, pagemeta * pagemeta_ptr)	{
	}

	void after_iteration()	{
	}

	void finalize()	{
		t.stop_time();
	}
};

int main(int argc, char ** argv)	{
	parameter_handler param(argc, argv);

	temp_algorithm alg;
	engine e(alg, param);

	e.run();

	return 0;
}
