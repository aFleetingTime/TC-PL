#include <iostream>
#include <iomanip>
#include <cassert>
#include "Format/Format.hpp"

void print(std::size_t n)
{
	assert(n % 2);
#if 0
	for(std::size_t f = n / 2, i = 1, x = n / 2 + 1; i == 1; ++n, (n > x))
	{
		if(f) std::cout << std::setw(f) << std::setfill(' ') << ' ';
		std::cout << std::setw(i) << std::setfill('*') << '*' << std::endl;
		if(n > x)
			--f, i += 2;
		else
			++f, i -= 2;
	}
#endif
	for(std::size_t i = n / 2, s = i + 1; n; --n)
	{
		if (i) std::cout << std::setw(i) << std::setfill(' ') << ' ';
		std::cout << std::setw(2 * (s - i - 1) + 1) << std::setfill('*') << '*' << std::endl;
		n > s ? --i : ++i;
	}
}

int main()
{
	print(41);
}
