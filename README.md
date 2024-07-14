# v2

I never really liked the interface of v1, so after coming back to this project, I spent some time thinking about how it could be improved,
took some inspiration from Golangs flag module, and made v2, which has a simpler, more flexible interface with better type safety

For example:
 - It doesn't cast char* to uint64_t and require you to cast it back
 - It doesn't require you to create a parser object, you just call parse
 - You don't have to add the arguments to that parser object, it's taken care of just by calling the constructor
 - You aren't required to provide descriptions for you program and every argument

That last one means that v2 will _not_ take care of --help and -h for you. If you want a help menu, you have to create it yourself.
But help menus aren't always required and when you do want one, this gives you control over what it looks like


# How do I use this

Check out [test.cpp](test.cpp). It demonstrates the full scope of this library

