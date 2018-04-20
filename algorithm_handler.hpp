#ifndef __ALGORITHM_HANDLER_HPP__
#define __ALGORITHM_HANDLER_HPP__

#include <information_handler.hpp>

class algorithm_handler	{
public:
	virtual void initialize(information_handler * information_ptr) {}
	virtual void before_iteration() {}
	virtual bool request_nextchunk()	{}
	virtual void execute_node(int node_id, int * adjlist_ptr, int length, int connectedsize, pagemeta * pagemeta_ptr) {}
	virtual void after_iteration() {}
	virtual void finalize() {}
};

#endif