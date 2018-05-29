#include <iostream>
#include <charconv>
#include <string>
#include <string_view>
#include <chrono>
#include <vector>
#include <filesystem>
#include <numeric>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <cstring>

constexpr std::size_t minArgc = 3;

[[noreturn]] inline void errorExit(std::string_view info)
{
	std::perror(info.data());
	std::exit(EXIT_FAILURE);
}

[[noreturn]] inline void logExit(std::string_view info)
{
	std::cerr << info << '\n';
	std::exit(EXIT_FAILURE);
}

int main(int argc, char *argv[])
{
	if(argc < minArgc)
		logExit("需要提供测试次数与目标程序");
	if(!std::filesystem::is_regular_file(argv[2]))
		logExit("非可执行程序");
	std::size_t count = 0;
	if(std::from_chars(argv[1], argv[1] + std::char_traits<char>::length(argv[1]), count).ec != std::errc() || !count)
		logExit("测试次数必须为大于0的正整数");
	std::vector<std::chrono::high_resolution_clock::rep> times(count);
	for(std::size_t i = 0; i != count; ++i)
	{
		auto start = std::chrono::high_resolution_clock::now();
		if(pid_t pid = fork(); !pid)
		{
			execv(argv[2], argv + 3);
			errorExit(*argv);
			return EXIT_FAILURE;
		}
		else
		{
			auto mid = std::chrono::high_resolution_clock::now();
			if(wait(nullptr) == -1)
				errorExit(*argv);
			times[i] = (std::chrono::high_resolution_clock::now() - start - (mid - start)).count();
		}
	}
	std::cout << "平均用时: " << std::accumulate(std::next(times.cbegin()), times.cend(), static_cast<long long>(times.front() / count), 
			[count](const auto &old, const auto &time) { return old + time / count; }) << "ns" << std::endl;
	return EXIT_SUCCESS;
}
