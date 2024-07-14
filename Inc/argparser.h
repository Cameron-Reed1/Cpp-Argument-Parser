#pragma once
#include <vector>
#include <stdint.h>

namespace Arguments
{

enum ERROR {
	NO_ERROR = 0,

	ERROR_UNKNOWN_ARGUMENT,
	ERROR_UNKNOWN_POSITIONAL_ARGUMENT,

	ERROR_MISSING_VALUE,
	ERROR_MISSING_POSITIONAL_ARGUMENT,

    ERROR_INVALID_INT,
};

class Parser
{
public:
    Parser(int argc, char** argv);

    bool hasNext();
    void next();
    const char* value();

private:
    int m_ArgCount;
    char** m_ArgValues;

    int m_Index;
};


class Argument
{
public:
    Argument(const char* name);
    Argument(char name);

    Argument& alias(const char* alias);
    Argument& alias(char alias);

    bool matches(const char*);
    bool matchesShort(char);

    virtual ERROR noValue() = 0;
    virtual ERROR parseValue(Parser& state) = 0;

public:
    bool found;

protected:
    const char* m_Name;
    char m_ShortName;

private:
    std::vector<const char*> m_Aliases;
    std::vector<char> m_ShortAliases;
};


class Int: public Argument
{
public:
    Int(const char* name, int64_t defaultValue);
    Int(char name, int64_t defaultValue);

    ERROR noValue() override;
    ERROR parseValue(Parser& state) override;

public:
    int64_t value;

private:
    void printMissingValueError();
};

class String: public Argument
{
public:
    String(const char* name, const char* defaultValue);
    String(char name, const char* defaultValue);

    ERROR noValue() override;
    ERROR parseValue(Parser& state) override;

public:
    const char* value;

private:
    void printMissingValueError();
};

class Bool: public Argument
{
public:
    Bool(const char* name);
    Bool(char name);

    ERROR noValue() override;
    ERROR parseValue(Parser& state) override;
};


class PositionalArgument
{
public:
    PositionalArgument(bool required);

    virtual ERROR parseValue(Parser& state) = 0;

public:
    bool found;
    bool required;
};


class PositionalInt: public PositionalArgument
{
public:
    PositionalInt(int64_t defaultValue, bool required);

    ERROR parseValue(Parser& state) override;

public:
    int64_t value;
};

class PositionalString: public PositionalArgument
{
public:
    PositionalString(const char* defaultValue, bool required);

    ERROR parseValue(Parser& state) override;

public:
    const char* value;
};


ERROR parse(int argc, char** argv);
ERROR findMatch(Parser& parser, const char* name);
ERROR findShortMatch(Parser& parser, char name, bool last);
ERROR matchPositional(Parser& parser);

void printInvalidIntError(const char* value);
void printUnknownArgError(const char* argument);
void printUnknownArgError(char argument);
void printUnknownPosArgError(const char* value);

} // namespace Arguments

