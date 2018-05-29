#include <iostream>
#include <vector>
#include <fstream>
#include <string>
#include <regex>

std::size_t getWordQuantity(const std::string &s)
{
	auto pos = s.find_first_not_of('\t');
	std::size_t count = 0;
	while (pos != std::string::npos)
	{
		++count;
		pos = s.find_first_not_of('\t', s.find('\t', pos + 1));
	}
	return count;
}

[[noreturn]] void mismatchExit()
{
	std::cerr << "格式不匹配" << '\n';
	std::exit(EXIT_FAILURE);
}

int main(int argc, char *argv[])
{
	std::ifstream in(argv[1]);
	if (!in)
	{
		std::cerr << "error" << '\n';
		return EXIT_FAILURE;
	}

	std::string line;
	std::getline(in, line);
	std::string patString(R"(^[\w ]+(\t+[\w ]+)*$)");
	std::regex titlePat(patString);

	std::smatch result;
	std::regex_match(line, result, titlePat);
	std::size_t size = getWordQuantity(result[0].str());
	if (size == 0)
		return 0;

	std::regex rowPat(R"(\t*\d+)");
	std::vector<int> total(size);

	while (std::getline(in, line))
	{
		std::size_t count = 0;
		for (std::sregex_iterator regexBeg(line.cbegin(), line.cend(), rowPat), regexEnd; regexBeg != regexEnd; ++regexBeg)
		{
			if (!(*regexBeg)[0].matched || count == size)
				mismatchExit();
			total[count++] += std::stoi((*regexBeg).str());
		}
		if (count != size)
			mismatchExit();
	}
	std::cout << "total: ";
	std::copy(total.cbegin(), total.cend(), std::ostream_iterator<int>(std::cout, "\t"));
	std::cout << std::endl;
}
