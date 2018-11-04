#include <chrono>
#include <iostream>

constexpr size_t N = 100000;
std::chrono::high_resolution_clock::time_point sTime, eTime;

void throwTime(std::size_t n)
{
	if(n != 0)
		throwTime(n - 1);
	eTime = std::chrono::high_resolution_clock::now();
	throw '\0';
}

void returnTime(std::size_t n)
{
	if(n != 0)
	{
		returnTime(n - 1);
		return;
	}
	eTime = std::chrono::high_resolution_clock::now();
}

int main()
{
#ifdef THROW
	try {
		sTime = std::chrono::high_resolution_clock::now();
		throwTime(N);
	} catch(char c) {
		std::cout << "time: " << std::chrono::duration_cast<std::chrono::nanoseconds>(eTime - sTime).count() << std::endl;	
	}
#else
	sTime = std::chrono::high_resolution_clock::now();
	returnTime(N);
	std::cout << "time: " << std::chrono::duration_cast<std::chrono::nanoseconds>(eTime - sTime).count() << std::endl;	
#endif
}
