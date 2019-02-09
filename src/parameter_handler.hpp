#ifndef __PARAMETER_HANDLER_HPP__
#define __PARAMETER_HANDLER_HPP__

#include <string>
#include <vector>

class parameter_handler	{
	int argc;
	char ** argv;

public:
	parameter_handler(int _argc, char ** _argv)	{
		argc = _argc;
		argv = _argv;
	}

	int get_param_int(const char * parameter_name, int default_value)	{
		for (int command_index = 0; command_index < argc - 1; command_index++)	{
			if (strcmp(argv[command_index], parameter_name) == 0)	{
				return atoi(argv[command_index + 1]);
			}
		}

		return default_value;
	}

	std::string get_param_string(const char * parameter_name, std::string default_value)	{
		for (int command_index = 0; command_index < argc - 1; command_index++)	{
			if (strcmp(argv[command_index], parameter_name) == 0)	{
				return std::string(argv[command_index + 1]);
			}
		}

		return default_value;
	}
};

#endif