#include <random>
#include <iomanip>
#include <map>
#include <string>
#include <iostream>
#include <functional>

template<typename T, typename R>
void discrete(T &sample, R rand, std::size_t count)
{
	while (count--)
	{
		std::size_t temp = 0;
		for (std::size_t i = 0; i < 5; ++i)
			temp += rand();
		++sample[temp];
	}
}

template<typename T>
void print(const T &smp)
{
	for (auto [num, count] : smp)
		std::cout << std::setw(2) << num << ": " << std::setw(2) << count << ' ' << std::string(count, '*') << std::endl;
}

int main()
{
	double a, b;
	std::cin >> a >> b;
	auto rand = std::bind(std::discrete_distribution<>({a, b}), std::default_random_engine(std::random_device()()));
	std::map<std::size_t, std::size_t> sample;
	discrete(sample, rand, 200);
	print(sample);
}
