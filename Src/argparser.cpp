#include <iostream>
#include <cctype>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <vector>

#include "argparser.h"


namespace Arguments
{
static std::vector<Argument*> arguments;
static std::vector<PositionalArgument*> positionalArguments;
static size_t posArgIdx = 0;


Parser::Parser(int argc, char** argv)
    : m_ArgCount(argc), m_ArgValues(argv), m_Index(0) { };

const char* Parser::value()
{
    if (m_Index >= m_ArgCount) {
        return nullptr;
    } else {
        return m_ArgValues[m_Index];
    }
}

bool Parser::hasNext()
{
    return m_Index + 1 < m_ArgCount;
}

void Parser::next()
{
    m_Index++;
}


Argument::Argument(const char* name)
    : found(false), m_Name(name), m_ShortName(0)
{
    arguments.push_back(this);
};

Argument::Argument(char name)
    : m_Name(nullptr), m_ShortName(name)
{
    arguments.push_back(this);
};

Argument& Argument::description(const char* desc)
{
    m_Description = desc;
    return *this;
}

Argument& Argument::alias(const char* alias)
{
    m_Aliases.push_back(alias);
    return *this;
}

Argument& Argument::alias(char alias)
{
    m_ShortAliases.push_back(alias);
    return *this;
}

bool Argument::matches(const char* arg)
{
    if (m_Name != nullptr) {
        if (std::strlen(arg) == std::strlen(m_Name)) {
            if (std::strcmp(arg, m_Name) == 0) {
                return true;
            }
        }
    }

    for (const char* alias: m_Aliases) {
        if (std::strlen(arg) == std::strlen(alias)) {
            if (std::strcmp(arg, alias) == 0) {
                return true;
            }
        }
    }

    return false;
}

bool Argument::matchesShort(char arg)
{
    if (arg == 0) {
        return false;
    }

    if (arg == m_ShortName) {
        return true;
    }

    for (char alias: m_ShortAliases) {
        if (arg == alias) {
            return true;
        }
    }

    return false;
}


Int::Int(const char* name, int64_t defaultValue)
    : Argument(name), value(defaultValue) { }

Int::Int(char name, int64_t defaultValue)
    : Argument(name), value(defaultValue) { }

ERROR Int::noValue()
{
    printMissingValueError();
    return ERROR_MISSING_VALUE;
}

ERROR Int::parseValue(Parser& state)
{
    if (!state.hasNext()) {
        printMissingValueError();
        return ERROR_MISSING_VALUE;
    }

    state.next();

    const char* val = state.value();
    if (*val == '-' || *val == '+') {
        val++;
    }

    while (*val != '\0') {
        if (*val < '0' || *val > '9') {
            printInvalidIntError(state.value());
            return ERROR_INVALID_INT;
        }
        val++;
    }

    found = true;
    value = std::atoi(state.value());

    return NO_ERROR;
}

void Int::printMissingValueError()
{
    std::cerr << "Missing integer value for argument ";
    if (m_Name != nullptr) {
        std::cerr << "--" << m_Name << std::endl;
    } else {
        std::cerr << "-" << m_ShortName << std::endl;
    }
}


String::String(const char* name, const char* defaultValue)
    : Argument(name), value(defaultValue) { }

String::String(char name, const char* defaultValue)
    : Argument(name), value(defaultValue) { }

ERROR String::noValue()
{
    printMissingValueError();
    return ERROR_MISSING_VALUE;
}

ERROR String::parseValue(Parser& state)
{
    if (!state.hasNext()) {
        printMissingValueError();
        return ERROR_MISSING_VALUE;
    }

    found = true;

    state.next();
    value = state.value();

    return NO_ERROR;
}

void String::printMissingValueError()
{
    std::cerr << "Missing value for argument ";
    if (m_Name != nullptr) {
        std::cerr << "--" << m_Name << std::endl;
    } else {
        std::cerr << "-" << m_ShortName << std::endl;
    }
}


Bool::Bool(const char* name)
    : Argument(name) { }

Bool::Bool(char name)
    : Argument(name) { }

ERROR Bool::noValue()
{
    found = true;

    return NO_ERROR;
}

ERROR Bool::parseValue(Parser& state)
{
    (void) state;

    found = true;

    return NO_ERROR;
}


PositionalArgument::PositionalArgument(bool required)
    : found(false), required(required), m_Description(nullptr)
{
    if (required) {
        for (PositionalArgument* arg: positionalArguments) {
            if (!arg->required) {
                std::cerr << "All required positional arguments must be registered before any optional positional arguments" << std::endl;
                exit(-1);
            }
        }
    }

    positionalArguments.push_back(this);
}

void PositionalArgument::description(const char* desc)
{
    m_Description = desc;
}


PositionalInt::PositionalInt(int64_t defaultValue, bool required)
    : PositionalArgument(required), value(defaultValue) { }

ERROR PositionalInt::parseValue(Parser& state)
{
    const char* val = state.value();
    if (*val == '-' || *val == '+') {
        val++;
    }

    while (*val != '\0') {
        if (*val < '0' || *val > '9') {
            printInvalidIntError(state.value());
            return ERROR_INVALID_INT;
        }
        val++;
    }

    found = true;
    value = std::atoi(state.value());

    return NO_ERROR;
}


PositionalString::PositionalString(const char* defaultValue, bool required)
    : PositionalArgument(required), value(defaultValue) { }

ERROR PositionalString::parseValue(Parser& state)
{
    found = true;
    value = state.value();

    return NO_ERROR;
}


ERROR parse(int argc, char** argv)
{
    Parser parser(argc, argv);

    while (parser.hasNext()) {
        parser.next(); // This is OK on the first iteration because it skips the executable name

        const char* value = parser.value();
        bool firstDash = value[0] == '-';
        bool secondDash = value[1] == '-';

        if (firstDash && secondDash) {
            ERROR err = findMatch(parser, value + 2);
            if (err != NO_ERROR) {
                return err;
            }
        } else if (firstDash) {
            size_t len = std::strlen(value);
            for (size_t i = 1; i < len; i++) {
                ERROR err = findShortMatch(parser, value[i], i == len - 1);
                if (err != NO_ERROR) {
                    return err;
                }
            }
        } else {
            ERROR err = matchPositional(parser);
            if (err != NO_ERROR) {
                return err;
            }
        }
    }

    if (posArgIdx < positionalArguments.size()) {
        if (positionalArguments[posArgIdx]->required) {
            std::cerr << "Missing required positional argument" << std::endl;
            return ERROR_MISSING_POSITIONAL_ARGUMENT;
        }
    }

    return NO_ERROR;
}

ERROR findMatch(Parser& parser, const char* name)
{
    for (Argument* arg: arguments) {
        if (arg->matches(name)) {
            return arg->parseValue(parser);
        }
    }

    printUnknownArgError(name);
    return ERROR_UNKNOWN_ARGUMENT;
}

ERROR findShortMatch(Parser &parser, char name, bool last)
{
    for (Argument* arg: arguments) {
        if (arg->matchesShort(name)) {
            if (last) {
                return arg->parseValue(parser);
            }

            return arg->noValue();
        }
    }

    printUnknownArgError(name);
    return ERROR_UNKNOWN_ARGUMENT;
}

ERROR matchPositional(Parser &parser)
{
    if (posArgIdx >= positionalArguments.size()) {
        printUnknownPosArgError(parser.value());
        return ERROR_UNKNOWN_POSITIONAL_ARGUMENT;
    }

    return positionalArguments[posArgIdx++]->parseValue(parser);
}

void printInvalidIntError(const char* value)
{
    std::cerr << value << " is not an integer" << std::endl;
}

void printUnknownArgError(const char* argument)
{
    std::cerr << "Unknown argument --" << argument << std::endl;
}

void printUnknownArgError(char argument)
{
    std::cerr << "Unknown argument -" << argument << std::endl;
}

void printUnknownPosArgError(const char* value)
{
    std::cerr << "Extra positional argument " << value << std::endl;
}

} // namespace Arguments

