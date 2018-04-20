#ifndef __TIMER_HANDLER_HPP__
#define __TIMER_HANDLER_HPP__

#include <chrono>
#include <map>
#include <atomic>

typedef struct time_count	{
	double time;
	size_t count;

	time_count(double _time)	{
		time = _time;
		count = 0;
	}
}time_count;

typedef struct time_entry	{
	std::string timename;

	std::chrono::system_clock::time_point start;
	std::chrono::system_clock::time_point stop;

	time_entry(std::string _timename)	{
		timename = _timename;
	}

	void start_time()	{
		start = std::chrono::system_clock::now();
	}

	void stop_time()	{
		stop = std::chrono::system_clock::now();
	}

	std::chrono::duration<double> runtime()	{
		return stop - start;
	}
}time_entry;

class timer_handler	{
private:
	std::map<std::string, time_count> timelist;
	std::chrono::system_clock::time_point start;
	std::chrono::system_clock::time_point stop;
	std::chrono::duration<double> running_time;
	std::atomic_flag flag;

public:
	timer_handler()	{
		flag.clear();
	}

	void add_entry(time_entry entry)	{
		while (flag.test_and_set());

		std::map<std::string, time_count>::iterator itr = timelist.find(entry.timename);

		if (itr == timelist.end())	{
			timelist.insert(std::pair<std::string, time_count>(entry.timename, time_count(entry.runtime().count())));
		}
		else {
			itr->second.time += entry.runtime().count();
			itr->second.count++;
		}

		flag.clear();
	}

	void print_timelist()	{
		printf("timename\ttimevalue\n");
		for (std::map<std::string, time_count>::iterator itr = timelist.begin(); itr != timelist.end(); itr++)	{
			printf(" %s\t%lf\t%lu\n", itr->first.c_str(), itr->second.time, itr->second.count);
		}
	}
};

#endif