#include <iostream>
#include <string_view>

struct Base
{
	Base() { std::cout << "Base()" << std::endl; }
	Base(std::string_view s) { std::cout << s << std::endl; throw 0; }
	~Base() { std::cout << "~Base()" << std::endl; }
};

int main()
{
	try {
		Base("AAA");
	}
	catch(...) {
		std::cout << "over" << std::endl;
	}
}
