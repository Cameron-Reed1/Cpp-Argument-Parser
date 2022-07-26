#include "argparser.h"
#include <stdint.h>
#include <string.h>
#include <iostream>

int stringToInt(const char* input)
{
	int value = 0;
	for (int i = 0; input[i] != 0; i++)
	{
		if (input[i] < '0' || input[i] > '9')
			return -1;
		value = (value * 10) + input[i] - '0';
	}

	return value;
}

namespace Cam
{
namespace Arguments
{

Option::Option(const char* name, const char* description, OPTTYPE type)
	: m_name(name), m_short_name(nullptr), m_description(description), m_type(type), m_found(false) {}

Option::Option(const char* name, const char* short_name, const char* description, OPTTYPE type)
	: m_name(name), m_short_name(short_name), m_description(description), m_type(type), m_found(false) {}

bool Option::found()
{
	return m_found;
}

PositionalArgument::PositionalArgument(const char* name, bool required, OPTTYPE type /* = STRING */)
	: m_name(name), m_type(type), m_req(required)
{
	if (type == FLAG) {
		std::cout << "Warning OPTTYPE 'FLAG' is not meant to be used with positional arguments" << std::endl;
		std::cout << "Assuming type 'STRING' instead" << std::endl;
		m_type = STRING;
	}
}

bool PositionalArgument::found()
{
	return m_found;
}

Parser::Parser(const char* program_name)
	: m_program_name(program_name), m_description(nullptr), m_options() {}

void Parser::set_description(const char* description)
{
	m_description = description;
}

void Parser::add_option(Option* opt)
{
	m_options.push_back(opt);
}

void Parser::add_positional_argument(PositionalArgument* arg)
{
	m_positional_args.push_back(arg);
}

int Parser::parse(int argc, char ** argv)
{
	uint8_t next_pos_arg = 0;
	bool all_pos_args_found = m_positional_args.size() == 0;
	for (uint16_t i = 1; i < argc; i++)
	{
		if (argv[i][0] == '-') {
			bool match_found = false;
			bool use_long_name = argv[i][1] == '-';
			Option* matched_arg;
			const char* option_name;

			for (Option* arg: m_options)
			{
				if ((use_long_name ? strcmp(argv[i] + 2, arg->m_name) : strcmp(argv[i] + 1, arg->m_short_name)) == 0) {
					arg->m_found = true;
					match_found = true;
					option_name = (use_long_name ? arg->m_name : arg->m_short_name);
					matched_arg = arg;

					break;
				}
			}

			if (!match_found) {
				if (use_long_name && strcmp(argv[i] + 2, "help") == 0) {
					print_help_message();
					return 1;
				}
				std::cout << m_program_name << ": invalid option: " << argv[i] << std::endl << std::endl;
				std::cout << "See " << m_program_name << " --help" << std::endl;

				return -1;
			} else {
				if (matched_arg->m_type != FLAG) {
					i++;
					if (i < argc && argv[i][0] != '-') {
						if (matched_arg->m_type == STRING) {
							matched_arg->data = argv[i];
						} else if (matched_arg->m_type == INT) {
							int* val = new int;
							*val = stringToInt(argv[i]);
							if (*val == -1) {
								std::cout << m_program_name << ": option '" << option_name << "' expects an integer as an argument, got " << argv[i] << " instead" << std::endl;
								return -4;
							}
							matched_arg->data = val;
						} else {
							std::cout << "You appear to have forgotten to add a handler for this type of argument: " << matched_arg->m_type << std::endl;
						}
					} else {
						std::cout << m_program_name << ": Missing argument for option '" << option_name << '\'' << std::endl;
						return -3;
					}
				}
			}
		} else if (!all_pos_args_found) {
			PositionalArgument* arg = m_positional_args[next_pos_arg];
			if (arg->m_type == INT) {
				int* val = new int;
				*val = stringToInt(argv[i]);
				if (*val == -1) {
					std::cout << m_program_name << ": argument '" << arg->m_name << "' expects an integer as an argument, got " << argv[i] << " instead" << std::endl;
					return -4;
				}
				arg->data = val;
			} else {
				arg->data = argv[i];
			}
			arg->m_found = true;
			next_pos_arg++;
			all_pos_args_found = m_positional_args.size() <= next_pos_arg;
		} else {
			std::cout << m_program_name << ": invalid option '" << argv[i] << '\'' << std::endl << std::endl;
			std::cout << "See " << m_program_name << " --help" << std::endl;

			return -2;
		}
	}

	if (!all_pos_args_found) {
		for (; next_pos_arg < m_positional_args.size(); next_pos_arg++) {
			if (m_positional_args[next_pos_arg]->m_req) {
				std::cout << m_program_name << ": Missing required positional argument '" << m_positional_args[next_pos_arg]->m_name << "'" << std::endl << std::endl;
				std::cout << "See " << m_program_name << " --help" << std::endl;

				return -5;
			}
		}
	}

	return 0;
}

void Parser::print_help_message()
{
	std::cout << "Usage: " << m_program_name << (m_options.size() > 0 ? " [OPTIONS]" : "");
	for (PositionalArgument* arg: m_positional_args)
	{
		std::cout << " " << arg->m_name;
	}
	std::cout << std::endl << std::endl << std::endl;


	std::cout << "Options:" << std::endl;
	for (Option* opt: m_options)
	{
		std::cout << "  -";
		if (opt->m_short_name != nullptr)
			std::cout << opt->m_short_name << ", -";

		std::cout << '-' << opt->m_name;

		if (opt->m_short_name != nullptr) {
			for (uint8_t i = 0; i < 20 - strlen(opt->m_name) - strlen(opt->m_short_name); i++)
				std::cout << ' ';
		} else {
			for (uint8_t i = 0; i < 20 - strlen(opt->m_name); i++)
				std::cout << ' ';
		}

		std::cout << opt->m_description << std::endl;
	}

	std::cout << std::endl << std::endl << m_description << std::endl;
}

} // namespace Arguments
} // namespace Cam
