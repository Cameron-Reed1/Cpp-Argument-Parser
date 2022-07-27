#include <iostream>
#include "argparser.h"

using namespace Cam;

int main(int argc, char **argv) {
	Arguments::Parser parser = Arguments::Parser("test");

	parser.set_description("A test of argument parsing :)");

	Arguments::Option first('f', "First arg", Arguments::STRING);
	Arguments::Option second("second", 's', "Second arg", Arguments::INT);
	Arguments::Option third("third", "Third arg", Arguments::FLAG);

	Arguments::PositionalArgument name("name", true, Arguments::STRING);
	Arguments::PositionalArgument age("age", false, Arguments::INT);

	parser.add_option(&first);
	parser.add_option(&second);
	parser.add_option(&third);

	parser.add_positional_argument(&name);
	parser.add_positional_argument(&age);

	if (parser.parse(argc, argv) != 0) {
		return 0;
	}

	std::cout << "Found:" << std::endl;
	std::cout << "\tFirst:    " << first.found() << std::endl;
	std::cout << "\tSecond:   " << second.found() << std::endl;
	std::cout << "\tThird:    " << third.found() << std::endl;
	std::cout << "\tName:     " << name.found() << std::endl;
	std::cout << "\tAge:      " << age.found() << std::endl;

	if (first.found() || second.found() || name.found() || age.found())
		std::cout << std::endl << "Values:" << std::endl;
	if (first.found())
		std::cout << "\tFirst:    " << (char*) first.data << std::endl;
	if (second.found())
		std::cout << "\tSecond:   " << *((int*) second.data) << std::endl;
	if (name.found())
		std::cout << "\tName:     " << (char*) name.data << std::endl;
	if (age.found())
		std::cout << "\tAge:      " << *((int*) age.data) << std::endl;

    return 0;
}
