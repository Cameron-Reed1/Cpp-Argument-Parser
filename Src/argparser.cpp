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
	: m_name(name), m_short_name(0), m_description(description), m_type(type), m_found(false) {}

Option::Option(const char* name, const char short_name, const char* description, OPTTYPE type)
	: m_name(name), m_short_name(short_name), m_description(description), m_type(type), m_found(false) {}

Option::Option(const char short_name, const char* description, OPTTYPE type)
	: m_name(nullptr), m_short_name(short_name), m_description(description), m_type(type), m_found(false) {}

bool Option::found()
{
	return m_found;
}

PositionalArgument::PositionalArgument(const char* name, bool required, OPTTYPE type /* = STRING */)
	: m_name(name), m_type(type), m_req(required), m_found(false)
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
	: m_program_name(program_name), m_description(nullptr), m_options(), m_positional_args() {}

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

ERROR Parser::parse(int argc, char** argv)
{
	m_pos_index = 0;
	for (m_opt_index = 1; m_opt_index < argc; m_opt_index++)
	{
		if (argv[m_opt_index][0] == '-' && argv[m_opt_index][1] == '-') {

			ERROR ret = handle_long_option(argv[m_opt_index] + 2, m_opt_index + 1 < argc ? argv[m_opt_index + 1] : nullptr);
			if (ret)
				return ret;

		} else if (argv[m_opt_index][0] == '-' && argv[m_opt_index][1] != '-') {

			for (uint8_t i = 1; argv[m_opt_index][i] != 0; i++) {
				bool last = argv[m_opt_index][i + 1] == 0;
				ERROR ret = handle_short_option(argv[m_opt_index][i], m_opt_index + 1 < argc && last ? argv[m_opt_index + 1] : nullptr);
				if (ret)
					return ret;
				if (last)
					break;
			}

		} else {

			ERROR ret = handle_positional_argument(argv[m_opt_index]);
			if (ret)
				return ret;

		}
	}

	if (m_pos_index < m_positional_args.size()) {
		for (; m_pos_index < m_positional_args.size(); m_pos_index++) {
			if (m_positional_args[m_pos_index]->m_req)
				return missing_positional_argument(m_positional_args[m_pos_index]->m_name);
		}
	}

	return NO_ERROR;
}

ERROR Parser::handle_long_option(const char* option, const char* next_value)
{
	Option* matched_option = nullptr;

	for (Option* opt: m_options) {
		if (opt->m_name == nullptr)
			continue;
		if (strcmp(opt->m_name, option) == 0) {
			matched_option = opt;
			break;
		}
	}

	if (matched_option == nullptr) {

		if (strcmp("help", option) == 0)
			return print_help_message();

		return unknown_option(option);

	} else {

		matched_option->m_found = true;
		return get_option_data(matched_option, matched_option->m_name, next_value);

	}

	std::cout << "Congratulations! You have reached an impossible state" << std::endl << std::endl;
	std::cout << "Reality is broken :)" << std::endl;
	return IMPOSSIBLE;
}

ERROR Parser::handle_short_option(const char option, const char* next_value)
{
	Option* matched_option = nullptr;

	for (Option* opt: m_options) {
		if (opt->m_short_name == option) {
			matched_option = opt;
			break;
		}
	}

	if (matched_option == nullptr) {

		if ('h' == option)
			return print_help_message();

		return unknown_option(option);

	} else {

		matched_option->m_found = true;
		char* name = new char[2];
		name[0] = matched_option->m_short_name;
		name[1] = 0;
		return get_option_data(matched_option, name, next_value);
		delete[] name;

	}

	std::cout << "Congratulations! You have reached impossible state #2" << std::endl << std::endl;
	std::cout << "Reality is broken :)" << std::endl;
	return IMPOSSIBLE;
}

ERROR Parser::handle_positional_argument(const char* arg)
{
	if (m_pos_index >= m_positional_args.size())
		return unknown_option(arg);

	PositionalArgument* pos_arg = m_positional_args[m_pos_index];

	if (pos_arg->m_type == INT) {
		int* val = new int;
		if ((*val = stringToInt(arg)) == -1)
			return incorrect_type(pos_arg->m_name, arg);

		pos_arg->data = val;
	} else {
		pos_arg->data = (void*) arg;
	}

	pos_arg->m_found = true;
	m_pos_index++;
	return NO_ERROR;
}

ERROR Parser::get_option_data(Option* opt, const char* option_name, const char* data_str)
{
	if (opt->m_type == FLAG)
		return NO_ERROR;

	if (data_str == nullptr || data_str[0] == '-')
		return missing_argument(option_name);

	if (opt->m_type == INT) {
		int* val = new int;
		if ((*val = stringToInt(data_str)) == -1)
			return incorrect_type(opt->m_name, data_str);
		opt->data = val;
	} else {
		opt->data = (void*) data_str;
	}

	m_opt_index++;
	return NO_ERROR;
}

ERROR Parser::unknown_option(const char* option)
{
	std::cout << m_program_name << ": invalid option '" << option << '\'' << std::endl << std::endl;
	std::cout << "See " << m_program_name << " --help" << std::endl;

	return ERROR_UNKNOWN_OPTION;
}

ERROR Parser::unknown_option(const char option)
{
	std::cout << m_program_name << ": invalid option: " << option << std::endl << std::endl;
	std::cout << "See " << m_program_name << " --help" << std::endl;

	return ERROR_UNKNOWN_OPTION;
}

ERROR Parser::incorrect_type(const char* option, const char* got)
{
	std::cout << m_program_name << ": argument '" << option << "' expects an integer as an argument, got " << got << " instead" << std::endl;

	return ERROR_INCORRECT_TYPE;
}

ERROR Parser::missing_argument(const char* option)
{
	std::cout << m_program_name << ": Missing argument for option '" << option << '\'' << std::endl;

	return ERROR_MISSING_ARGUMENT;
}

ERROR Parser::missing_positional_argument(const char* arg)
{
	std::cout << m_program_name << ": Missing required positional argument '" << arg << "'" << std::endl << std::endl;
	std::cout << "See " << m_program_name << " --help" << std::endl;

	return ERROR_MISSING_POSITIONAL_ARGUMENT;
}

ERROR Parser::print_help_message()
{
	std::cout << "Usage: " << m_program_name << (m_options.size() > 0 ? " [OPTIONS]" : "");
	for (PositionalArgument* arg: m_positional_args)
	{
		std::cout << " " << arg->m_name;
	}
	std::cout << std::endl;


	if (m_options.size() > 0)
		std::cout << std::endl << std::endl << "Options:" << std::endl;
	for (Option* opt: m_options)
	{
		std::cout << "  ";
		if (opt->m_short_name != 0)
			std::cout << '-' << opt->m_short_name;

		if (opt->m_short_name != 0 && opt->m_name != nullptr)
			std::cout << ", ";

		if (opt->m_name != nullptr)
			std::cout << "--" << opt->m_name;

		if (opt->m_name != nullptr) {
			for (uint8_t i = 0; i < 20 - strlen(opt->m_name) - ((opt->m_short_name != 0) * 4); i++)
				std::cout << ' ';
		} else {
			for (uint8_t i = 0; i < 24 - ((opt->m_short_name != 0) * 4); i++)
				std::cout << ' ';
		}

		std::cout << opt->m_description << std::endl;
	}

	if (m_description)
		std::cout << std::endl << std::endl << m_description << std::endl;

	return SPECIAL_CASE_HELP;
}

} // namespace Arguments
} // namespace Cam
