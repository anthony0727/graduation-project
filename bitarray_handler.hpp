#ifndef __BITARRAY_HANDLER_HPP__
#define __BITARRAY_HANDLER_HPP__

#include <atomic>

#define WORDTYPE size_t
#define WORDTYPP_BYTE 8
#define WORDRANGE_LOG2 6
#define WORDFILTER 63
#define WORDBIT(nodenumber) (WORDTYPE(1) << ((nodenumber) & WORDFILTER))

class bitarray_handler {
private:
	//bit array
	std::atomic<WORDTYPE> ** curr_hindicator;	/* hierarchical indicator of of current iteration */
	std::atomic<WORDTYPE> ** next_hindicator;	/* hierarchical indicator of of current iteration */

	std::atomic<WORDTYPE> * curr_hindicators;
	std::atomic<WORDTYPE> * next_hindicators;

	size_t size_level;
	size_t size_partition_in_log2;

	size_t num_nodes;

public:
	bitarray_handler() : curr_hindicator(NULL), next_hindicator(NULL), curr_hindicators(NULL), next_hindicators(NULL), size_level(1), size_partition_in_log2(0), num_nodes(0)	{
	}

	~bitarray_handler()	{
	}

	/* memory allocation and deallocation */
	void create(size_t _size_partition_in_log2, size_t _num_nodes)	{
		size_partition_in_log2 = _size_partition_in_log2;
		num_nodes = _num_nodes;

		if (size_partition_in_log2 == 0)	{
			size_level = 1;
		}
		else {
			for (size_t num_bits = num_nodes; num_bits > 64; num_bits = (num_bits >> size_partition_in_log2) + 1, size_level++);
		}

		size_t shift = WORDRANGE_LOG2;
		size_t size_array = 0;

		for (size_t level = 0; level < size_level; level++)	{
			size_array += (num_nodes >> shift) + 1;
			shift += size_partition_in_log2;
		}

		printf("hindicator partition %lu level %lu\n", size_partition_in_log2, size_level);

		curr_hindicator = new std::atomic<WORDTYPE>*[size_level];
		next_hindicator = new std::atomic<WORDTYPE>*[size_level];
		
		curr_hindicators = new std::atomic<WORDTYPE>[size_array];
		next_hindicators = new std::atomic<WORDTYPE>[size_array];

		memset(curr_hindicators, 0, sizeof(std::atomic<WORDTYPE>) * size_array);
		memset(next_hindicators, 0, sizeof(std::atomic<WORDTYPE>) * size_array);

		shift = WORDRANGE_LOG2;
		size_array = 0;
		for (size_t level = 0; level < size_level; level++)	{
			curr_hindicator[level] = &curr_hindicators[size_array];
			next_hindicator[level] = &next_hindicators[size_array];
			
			size_array += (num_nodes >> shift) + 1;
			shift += size_partition_in_log2;
		}
	}
	void destroy()	{
		delete curr_hindicators;
		delete next_hindicators;

		delete curr_hindicator;
		delete next_hindicator;
	}

	void reset_lowerlevel(size_t arrayindex_first, size_t arrayindex_last, size_t shift, size_t level)	{
		if (arrayindex_last > (num_nodes >> shift)) arrayindex_last = (num_nodes >> shift);

		for (size_t index = arrayindex_first; index <= arrayindex_last; index++)	{
			if (next_hindicator[level][index] != 0)	{
				next_hindicator[level][index] = 0;

				if (level > 0) reset_lowerlevel(index << size_partition_in_log2, ((index + 1) << size_partition_in_log2) - 1, shift - size_partition_in_log2, level - 1);
			}
		}
	}

	/* exchange bit array of current with next */
	void exchange()	{
		std::atomic<size_t> ** temp = curr_hindicator;
		curr_hindicator = next_hindicator;
		next_hindicator = temp;

		size_t shift = (size_level - 1) * size_partition_in_log2 + WORDRANGE_LOG2;
		size_t size_array = (num_nodes >> shift);

		reset_lowerlevel(0, size_array, shift, size_level - 1);
	}

	/* is nodenumber set? */
	bool get_curr_bit(size_t nodenumber)	{
		return (curr_hindicator[0][nodenumber >> WORDRANGE_LOG2] & WORDBIT(nodenumber)) != 0;
	}
	bool get_next_bit(size_t nodenumber)	{
		return (next_hindicator[0][nodenumber >> WORDRANGE_LOG2] & WORDBIT(nodenumber)) != 0;
	}

	bool check_lowerlevel(size_t first_nodenumber, size_t last_nodenumber, size_t shift, size_t level)	{
		size_t arrayindex_offset = shift - WORDRANGE_LOG2;
		size_t arrayindex_first = first_nodenumber >> shift;
		size_t arrayindex_last = last_nodenumber >> shift;

		for (size_t index = arrayindex_first + 1; index < arrayindex_last; index++)	{
			if (curr_hindicator[level][index])	{
				return true;
			}
		}
		
		size_t first_uppermask = ~(WORDBIT(first_nodenumber >> arrayindex_offset) - 1);
		size_t last_lowermask = ((WORDBIT(last_nodenumber >> arrayindex_offset) - 1) << 1) + 1;

		if (level != 0)	{
			if ((first_nodenumber >> arrayindex_offset) == (last_nodenumber >> arrayindex_offset)) {
				if ((curr_hindicator[level][arrayindex_last] & ((last_lowermask >> 1) + 1)) != 0)
					if (check_lowerlevel(first_nodenumber, last_nodenumber, shift - size_partition_in_log2, level - 1) == true) return true;
			}
			else {
				if (arrayindex_first == arrayindex_last) {
					if ((curr_hindicator[level][arrayindex_first] & ((first_uppermask & (first_uppermask - 1)) & (last_lowermask >> 1))) != 0) return true;
				}
				else {
					if (((curr_hindicator[level][arrayindex_first] & (first_uppermask << 1)) | (curr_hindicator[level][arrayindex_last] & (last_lowermask >> 1))) != 0) return true;
				}

				if ((curr_hindicator[level][arrayindex_first] & (WORDTYPE(1) << __builtin_ctzll(first_uppermask))) != 0)
					if (check_lowerlevel(first_nodenumber, (((first_nodenumber >> arrayindex_offset) + 1) << arrayindex_offset) - 1, shift - size_partition_in_log2, level - 1) == true) return true;
				if ((curr_hindicator[level][arrayindex_last] & ((last_lowermask >> 1) + 1)) != 0)
					if (check_lowerlevel((last_nodenumber >> arrayindex_offset) << arrayindex_offset, last_nodenumber, shift - size_partition_in_log2, level - 1) == true) return true;
			}
		}
		else {
			if (arrayindex_first == arrayindex_last)	{
				if ((curr_hindicator[0][arrayindex_first] & (first_uppermask & last_lowermask)) != 0) return true;
			}
			else {
				if (((curr_hindicator[0][arrayindex_first] & first_uppermask) | (curr_hindicator[0][arrayindex_last] & last_lowermask)) != 0) return true;
			}
		}

		return false;
	}

	/* is any bit in the nodenumber interval set? */
	bool get_curr_bits(size_t first_nodenumber, size_t last_nodenumber)	{
		return check_lowerlevel(first_nodenumber, last_nodenumber, (size_level - 1) * size_partition_in_log2 + WORDRANGE_LOG2, size_level - 1);
	}

	/* set a bit of nodenumber */
	void set_curr_bit(size_t nodenumber)	{
		for (size_t level = 0; level < size_level; level++)	{
			if ((atomic_fetch_or(&curr_hindicator[level][nodenumber >> WORDRANGE_LOG2], WORDBIT(nodenumber)) & WORDBIT(nodenumber)) != WORDTYPE(0)) break;

			nodenumber >>= size_partition_in_log2;
		}
	}
	void set_next_bit(size_t nodenumber)	{
		for (size_t level = 0; level < size_level; level++)	{
			if ((next_hindicator[level][nodenumber >> WORDRANGE_LOG2] & WORDBIT(nodenumber)) == WORDTYPE(0)) {
				if ((atomic_fetch_or(&next_hindicator[level][nodenumber >> WORDRANGE_LOG2], WORDBIT(nodenumber)) & WORDBIT(nodenumber)) != WORDTYPE(0)) break;
			}

			nodenumber >>= size_partition_in_log2;
		}
	}

	size_t get_curr_word(size_t arrayindex)	{
		return curr_hindicator[0][arrayindex];
	}

	/* set all bits */
	void set_all_curr()	{
		for (size_t level = 0; level < size_level; level++)	{
			size_t size_array = (num_nodes >> (size_level * size_partition_in_log2 + WORDRANGE_LOG2)) + 1;

			for (size_t arrayindex = 0; arrayindex < size_array; arrayindex++)	{
				curr_hindicator[level][arrayindex] = WORDTYPE(-1);
			}
		}
	}
	void set_all_next()	{
		for (size_t level = 0; level < size_level; level++)	{
			size_t size_array = (num_nodes >> (size_level * size_partition_in_log2 + WORDRANGE_LOG2)) + 1;

			for (size_t arrayindex = 0; arrayindex < size_array; arrayindex++)	{
				next_hindicator[level][arrayindex] = WORDTYPE(-1);
			}
		}
	}
};

#endif