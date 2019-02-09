#ifndef __INFORMATION_HANDLER_HPP__
#define __INFORMATION_HANDLER_HPP__

#include <bitarray_handler.hpp>
#include <parameter_handler.hpp>
#include <attribute_handler.hpp>

struct information_handler	{
	bitarray_handler * bitarray_ptr;		/* indicator vector with bit array */
	parameter_handler * parameter_ptr;		/* parameters from shell command */
	attribute_wrapper * attribute_ptr;		/* attribute space for algorithm */
	wiss_handler * wiss_ptr;				/* graph data structure management */
	index_handler * index_ptr;				/* graph metadata management */
	
	int num_nodes;						/* total number of nodes */
	int num_threads;
	int cur_iteration;					/* number of current iteration */
	int max_iteration;					/* number of max iteration */
};

#endif