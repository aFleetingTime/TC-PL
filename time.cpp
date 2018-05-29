#include <chrono>
#include <iostream>
#include <string>
#include <string_view>

std::chrono::high_resolution_clock::time_point sTime;
std::chrono::high_resolution_clock::time_point eTime;

void func(std::size_t n)
{
	if(n != 0)
		func(n - 1);
	eTime = std::chrono::high_resolution_clock::now();
	throw 0;
}

std::size_t func2(std::size_t n)
{
	if(n != 0)
		return func2(n - 1);
	eTime = std::chrono::high_resolution_clock::now();
	return 0;
}

int main()
{
#if 1
	std::size_t count = 100000;
	std::string str("ABCDEFG");
	while(--count)
		const std::string_view &s = str;
#endif
#if 0
	try {
		sTime = std::chrono::high_resolution_clock::now();
		func(100000);
	}
	catch(int number) {
		std::cout << "time: " << std::chrono::duration_cast<std::chrono::nanoseconds>(eTime - sTime).count() << std::endl;	
	}
#endif
}
