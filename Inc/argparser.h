#pragma once
#include <vector>

namespace Cam
{
namespace Arguments
{

enum OPTTYPE
{
	FLAG = 0,
	STRING,
	INT
};

class Option
{
public:
	Option(const char* name, const char* description, OPTTYPE type);
	Option(const char* name, const char* short_name, const char* description, OPTTYPE type);
	bool found();
	void* data;
private:
	const char* m_name;
	const char* m_short_name;
	const char* m_description;
	OPTTYPE m_type;
	bool m_found;

	friend class Parser;
};

class PositionalArgument
{
public:
	PositionalArgument(const char* name, bool required, OPTTYPE type = STRING);
	bool found();
	void* data;
private:
	const char* m_name;
	OPTTYPE m_type;
	bool m_req;
	bool m_found;

	friend class Parser;
};

class Parser
{
public:
	Parser(const char* program_name);
	void set_description(const char* description);
	void add_option(Option* opt);
	void add_positional_argument(PositionalArgument* arg);
	int parse(int argc, char** argv);
private:
	void print_help_message();
private:
	const char* m_program_name;
	const char* m_description;
	std::vector<Option*> m_options;
	std::vector<PositionalArgument*> m_positional_args;
};

} // namespace Arguments
} // namespace Cam
