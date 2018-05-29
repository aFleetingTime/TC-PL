#include <locale>
#include <sstream>
#include <iostream>
#include <chrono>
#include <thread>
#include <string_view>
#include <signal.h>

[[noreturn]] void showCursor(int)
{
	std::system("clear");
	std::cout << "\033[?25h";
	std::exit(0);
}

int main()
{
	constexpr std::string_view fmt("%c");
	constexpr std::string_view moveUp("\033[1A");

	std::stringstream timeStream;
	timeStream.imbue(std::locale(""));
	const std::time_put<char> &tp = std::use_facet<std::time_put<char>>(timeStream.getloc());

	signal(SIGINT, showCursor);
	std::cout << "\033[?25l";

	while (true)
	{
		const std::time_t curTime = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
		tp.put(timeStream, timeStream, timeStream.fill(), std::localtime(&curTime), fmt.cbegin(), fmt.cend());
		std::cerr << timeStream.str() << std::endl << moveUp;
		timeStream.str("");
		std::this_thread::sleep_for(std::chrono::seconds(1));
	}
}
