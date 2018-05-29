#include <fstream>
#include <iostream>
#include <random>
#include <locale>
#include <functional>
#include <filesystem>

constexpr std::size_t encSeed = 35179754124;

class Encryption
{
public:
	explicit Encryption(std::size_t sd) : seed(sd) { }

	void enc(const std::string &fileName)
	{
		auto rand = std::bind(std::uniform_int_distribution<>(0, std::numeric_limits<char>::max()), std::default_random_engine(seed));
		std::fstream fs(fileName);
		if (!fs) throw std::invalid_argument("文件打开失败");
		std::uintmax_t fileSize = std::filesystem::file_size(fileName);
		for (char c; fs.get(c); fs.seekp(-1, std::ios_base::cur).put(c ^ rand()));
		std::cout << "转换: " << fileSize << "byte" << std::endl;
	}

private:
	std::size_t seed;
};

[[noreturn]] void pexit(std::string_view einfo)
{
	std::cerr << einfo;
	std::exit(EXIT_FAILURE);
}

int main(int argc, char *argv[])
{
	if (argc != 3)
		pexit("参数数量过少");
	unsigned long seed;
	try {
		seed = std::stoul(argv[2]);
	} catch(const std::invalid_argument &invalid) {
		pexit("密码无效");
	}
	Encryption e(seed);
	try {
		e.enc(argv[1]);
	} catch (const std::exception &ex) {
		std::cerr << ex.what() << '\n';
	}
}
