#include <locale>
#include <sstream>
#include <iostream>
#include <chrono>
#include <thread>
#include <string_view>
#include <iomanip>
#include <signal.h>

[[noreturn]] void showCursor(int)
{
	std::system("clear");
	std::cout << "\033[?25h";
	std::exit(0);
}

int main()
{
	constexpr std::string_view fmt("%Z %z %c%n");
	constexpr std::string_view moveUp("\033[1A");

	std::cout.imbue(std::locale(""));
	std::ios_base::sync_with_stdio(false);

	signal(SIGINT, showCursor);
	std::cout << "\033[?25l";

	while (true)
	{
		const std::time_t curTime = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
		std::cout << std::put_time(std::localtime(&curTime), fmt.data()) << moveUp << std::flush;
		std::this_thread::sleep_for(std::chrono::seconds(1));
	}
}
