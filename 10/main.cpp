#include <iomanip>
#include <sstream>
#include "Analyzer.h"

TokenStream tcin{std::cin};

void error(const std::string &s) {
	std::cerr << "Error[-]: " << s << '\n';
}

template<typename T>
inline typename Analyzer<T>::result_type calculate(Analyzer<T> &analy)
{
	try {
		return analy.calculate();
	} catch(const ReadTokenError &e) {
		error(e.what());
	} catch(const std::exception &e) {
		error(e.what());
	} catch(...) {
		std::cerr << "error" << std::endl;
	}
	return {};
}

int main(int argc, char *argv[])
{
	Analyzer<long double> analy(tcin);
	analy.addVariable({"pi", 3.141592653589793});
	analy.addVariable({"e", 2.718281828459045});
	std::cout.precision(18);
	if(argc > 1)
	{
		std::stringstream ss;
		for(int i = 0; argv[++i]; ss << argv[i]);
		tcin.setInput(ss);
		auto tmp = calculate(analy);
		//if(analy.errcno == Analyzer<long double>::Errnos::no_error)
		if(tmp)
			std::cout << tmp.value() << '\n';
		return static_cast<int>(analy.errcno);
	}
	std::cout << "计算器v1.0" << std::endl;
	while(tcin)
	{
		if(!analy.owns)
			std::cout << "\ninput >>> ";
		auto tmp = calculate(analy);
		if(tmp)
			std::cout << std::setw(10) << ' ' << tmp.value() << '\n';
		else analy.reset();
		if(std::cin.rdstate() == std::ios::failbit)
		{
			std::cin.clear(std::cin.rdstate() ^ std::ios::failbit);
			tcin.clear();
		}
	}
	std::cout.put('\n');
	return 0;
}
