#include "copyow_ptr.h"
#include <tuple>

#if 0
struct C
{
};

template<typename... Argv>
constexpr auto operator,(int x, const std::tuple<Argv...> &tup)
{
	return std::make_tuple(x, tup);
}
template<typename... Argv>
constexpr auto operator,(int x, std::tuple<Argv...> &&tup)
{
	return std::make_tuple(x, tup);
}

std::ostream& operator<<(std::ostream &os, C c)
{
	return os << std::endl;
}

template<typename... Argv>
std::ostream& prints(Argv&&... argv)
{
	return (std::cout << ... << argv) << std::endl;
}

template<typename... Argv, std::size_t... N>
void prints(const std::tuple<Argv...> &argv, std::index_sequence<N...>)
{
	(prints(std::get<N>(argv)...));
}
#endif

struct Int
{
	constexpr Int(unsigned long long n) : i(n) { }
	constexpr Int(const Int &n) : i(n.i) { }
private:
	unsigned long long i;
};
struct Char
{
	char c;
};

constexpr Int operator""i(unsigned long long i) { return {i * 3}; }

int main()
{
	std::cout << -+100 << std::endl;
}
