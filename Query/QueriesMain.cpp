#include <unistd.h>
#include <fstream>
#include "TextQuery.h"
#include "QueryResult.h"
#include "AndQuery.h"
#include "NotQuery.h"
#include "QueryDrive.h"

std::string getPath(const std::string opInfo)
{
	std::string path;
	do
	{
		while(std::cout << opInfo && !(std::cin >> path))
		{
			std::cerr << "\n输入无效, 请重新输入文件路径...\n";
			std::cin.clear(std::cin.rdstate() & ~std::cin.failbit & ~std::cin.eofbit);
		}
	} while(access(path.c_str(), F_OK) && std::cerr << "指定文件不存在, 请重新输入...\n");
	std::cin.ignore(256, '\n');
	return path;
}

void getBinaryNum(std::istream &is, std::size_t &a, std::size_t &b, char &op)
{
	while(!(is >> a >> op >> b))
	{
		std::cerr << "\n输入无效, 请重新输入: ";
		is.clear();
		is.ignore(256, '\n');
	}
	is.ignore(256, '\n');
}

void runQueries(std::ifstream &text)
{
	Chapter10::TextQuery query(text);
	std::string input;
	Chapter15::QueryDrive drive;
	while(std::cout << "待查询字符: " && std::getline(std::cin, input) && input != "q")
	{
		std::cout << '\n';
		if(input == "r")
		{
			text.close();
			text.open(getPath("重新指定目标文件路径: "));
			query.retext(text);
		}
		else if(input == "h")
			std::cout << "历史记录:\n" << drive.getHistory();
#if 0
		else if(input == "hc")
		{
			std::size_t a = 0, b = 0;
			char op = 0;
			std::cout << "历史记录组合模式: ";
			getBinaryNum(std::cin, a, b, op);
			try {
				std::cout << '\n' << drive.combination(--a, --b, op).eval(query);
			} catch(std::out_of_range &exc) {
				std::cout << exc.what() << std::endl;
			}
		}
#endif
		else
			std::cout << drive.createQuery(input).eval(query);
		std::cout << std::endl;
	}
}

int main()
{
	std::ifstream textFile(getPath("目标文件路径: "));
	std::cout << std::endl;
	std::cin.clear(std::cin.rdstate() & ~std::cin.failbit & ~std::cin.eofbit);
	runQueries(textFile);
}
