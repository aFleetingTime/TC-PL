#include "Format.hpp"
#include <iostream>

constexpr Format hexFloat(4, 0, std::ios_base::scientific | std::ios_base::fixed);
constexpr Format scien(0, 0, std::ios_base::scientific);
constexpr Format longfixed(16, 0, std::ios_base::fixed);

int main()
{
	std::cout << hexFloat(100.888) << ' ' << 100.888 << std::endl;
	std::cout << scien(777.2929) << ' ' << 777.2929 << std::endl;
	std::cout << longfixed(777.29299999999999999999) << ' ' << 777.29299999999999999999 << std::endl;
}
