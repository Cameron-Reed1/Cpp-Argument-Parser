#include <iostream>

#include "argparser.h"


int main(int argc, char** argv) {
    Arguments::String first('f', "none");
    first.alias("first");

	Arguments::Int second("second", 0);
    second.alias('s');

	Arguments::Bool third("third");
    third.alias("abc").alias('t');

	Arguments::PositionalString name("", true);
	Arguments::PositionalInt age(0, false);

	if (Arguments::parse(argc, argv) != Arguments::NO_ERROR) {
		return 0;
	}

	std::cout << "Found:" << std::endl;
	std::cout << "\tFirst:    " << first.found << std::endl;
	std::cout << "\tSecond:   " << second.found << std::endl;
	std::cout << "\tThird:    " << third.found << std::endl;
	std::cout << "\tName:     " << name.found << std::endl;
	std::cout << "\tAge:      " << age.found << std::endl;

	std::cout << std::endl << "Values:" << std::endl;
	std::cout << "\tFirst:    " << first.value << std::endl;
	std::cout << "\tSecond:   " << second.value << std::endl;
	std::cout << "\tName:     " << name.value << std::endl;
	std::cout << "\tAge:      " << age.value << std::endl;

    return 0;
}
