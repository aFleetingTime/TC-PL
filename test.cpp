#include <string>
#include <thread>
#include <complex>
#include <iomanip>
#include <locale>
#include <valarray>
#include <regex>
#include <ctime>
#include <iterator>
#include <memory>
#include <unordered_map>
#include <scoped_allocator>
#include <chrono>
#include <map>
#include <list>
#include <vector>
#include <array>
#include <type_traits>
#include <bitset>
#include <algorithm>
#include <tuple>
#include <iterator>
#include <experimental/iterator>
#include <cstddef>
#include <iostream>
#include <random>
#include <cmath>
#include <numeric>
#include <unistd.h>

extern "C" int print(const char *p);

constexpr int pow(int n) {
	return n ? 3 * pow(n - 1) : 1;
}

constexpr int toB3(std::initializer_list<char> list, std::size_t i = 0) {
	return i != list.size() ? std::pow(3, list.size() - i) * ((*(list.begin() + i)) - '0') + toB3(list, i + 1) : 0;
}

template<char... Chars>
constexpr int operator""_b3() {
	return toB3({Chars...});
}

struct B
{
	virtual void func() = 0;
	virtual void func3() = 0;
	virtual ~B() = default;
};

struct BB : virtual B
{
};

struct Base : public BB
{
	int a = 100;
	virtual void func1() = 0;
	virtual void func() { func1(); }
};
struct Base2 : public BB
{
	int a = 100;
	virtual void func2() = 0;
	virtual void func() { func2(); }
};


struct D : public Base, public Base2
{
	D() { std::cout << "???" << std::endl; }
	int a = 100;
	int b = 100;
	void func() { }
	void func1() { std::cout << "Dfunc" << std::endl; }
	void func2() { std::cout << "Dfunc" << std::endl; }
	void func3() { std::cout << "Dfunc" << std::endl; }
	operator B*() { return this; }
};

constexpr void (Base::*p)() = &Base::func;

template<typename T>
class Creator : public std::allocator<T>
{
	T* allocate(std::size_t n) noexcept { std::cout << "AAA" << std::endl; return std::malloc(n); }

	void deallocate(T p, std::size_t n) noexcept { std::cout << "???" << std::endl; }
	
	template<typename U, typename... Argv>
	void construct(U *p, Argv&&... argv) noexcept { ::new (p) T(std::forward<Argv>(argv)...); }

	void destroy(T *p) noexcept { }
};

struct CCC
{
};

template<typename T>
T&& func(T &&v)
{
	return std::forward<T>(v);
}

std::size_t fibo(std::size_t n)
{
	return n <= 1 ? 1 : fibo(n - 1) + fibo(n - 2);
}

constexpr std::size_t max = 15;

auto fun(int num)
{
	struct C { C(int i) : num(i) { } int num; };
	return C(num);
}

template<typename T>
constexpr bool isCompare(int)
{
	return false;
}

template<void (*P)()>
constexpr void isCompare()
{
}

namespace {
static void fff() { }
}

template<typename T, int I>
struct Class { };

template<typename T>
struct Class<T, 1> 
{ 
	template<typename U>
	void get(U i) { }
};

template struct Class<int, 2>;


struct MyC { };

void g(int) { }


template<typename T>
void ff(T &v)
{
	sort(v.begin(), v.end());
}
template<typename T>
void sort(T beg, T end)
{
	std::cout << "??" << std::endl;
}

struct MB1 { virtual void func1() = 0; };
struct MB2 { virtual void func2() = 0; };
struct MC1 : MB1, MB2 { };

namespace ipr {
struct Node { virtual void nodeFunc() = 0; };
struct Expr : Node { virtual void exprFunc() = 0; };
struct Stmt : Node { virtual void stmtFunc() = 0; };
struct Decl : Stmt { virtual void declFunc() = 0; };
struct Vari : Decl { virtual void variFunc() = 0; };
}
//实现
namespace impl {
struct Node : ipr::Node { void nodeFunc() override { } };
struct Expr : ipr::Expr, Node { void exprFunc() override { } };
struct Stmt : ipr::Stmt, Node { void stmtFunc() override { } };
struct Decl : ipr::Decl, Stmt { void declFunc() override { } };
struct Vari : ipr::Vari, Decl { void variFunc() override { } };
}

//constexpr std::size_t compare(

//编译时线性容器
template<std::size_t X, typename T, std::size_t N = 0>
struct M : M<(X < N) ? 0 : X, T, (N != X) ? N + 1 : std::numeric_limits<std::size_t>::max()>
{
	T value;
};

template<std::size_t X, typename T>
struct M<X, T, std::numeric_limits<std::size_t>::max()> 
{ 
	using value_type = T;
	template<std::size_t L>
	using get = M<X, T, L>;
	T value; 
};

template<typename T, std::size_t N>
struct M<0, T, N>
{ 
	static_assert(!(0 < N), "E");
};

template<typename T>
using MakeUnsigned = typename std::make_unsigned<T>::type;

template<template<typename...> typename F, typename... Args>
using Delay = F<Args...>;

template<typename T>
void check()
{
	std::conditional<(std::is_integral_v<T> > 1), typename std::make_unsigned<T>::type, std::false_type> a;
	//std::conditional<std::is_integral<T>::value, Delay<MakeUnsigned, T>, std::false_type> a;
}

namespace mystd {
	template<bool, typename T = void>
	struct enable_if { using type = T; };

	template<typename T>
	struct enable_if<false, T> { };

	template<bool b, typename T>
	using enable_if_t = typename enable_if<b, T>::type;

	template<typename...>
	using bool_t = bool;

#if 0
	template<typename F, typename... Args>
	constexpr auto has_f(F func, Args&&... args) -> bool_t<decltype(func(std::forward<Args>(args)...))>
	{
		return true;
	}
#endif
	// 缺点：args类型必须与func的参数类型严格匹配。
	template<typename T, typename... Args>
	constexpr auto has_f(T(*func)(Args...), Args&&... args) -> bool_t<decltype(func(std::forward<Args>(args)...))>
	{
		return true;
	}
	constexpr bool has_f(...)
	{
		return false;
	}

	template<std::size_t N, typename... Args>
	struct select;
	template<std::size_t N, typename First, typename... Args>
	struct select<N, First, Args...> : select<N - 1, Args...> { };
	template<typename Last, typename... Args>
	struct select<0, Last, Args...> { using type = Last; };

	template<std::size_t N, typename... Args>
	using select_t = typename select<N, Args...>::type;

	template<std::size_t Start, typename First, typename... Args>
	struct accumSize : accumSize<Start - 1, Args...> { };
	template<typename First, typename... Args>
	struct accumSize<0, First, Args...> {
		static inline constexpr std::size_t value = accumSize<0, Args...>::value + sizeof(First);
	};
	template<typename First>
	struct accumSize<0, First> {
		static inline constexpr std::size_t value = sizeof(First);
	};
	template<std::size_t Start, typename... Args>
	struct accumTypeSize : accumSize<Start - 1, Args...> { };

	constexpr std::size_t abs(int num) { return num < 0 ? num * -1 : num; }

	template<typename T>
	struct is_bottom_const;
	template<typename T>
	struct is_bottom_const<T*> : std::false_type { };
	template<typename T>
	struct is_bottom_const<const T*> : std::true_type { };
	template<typename T>
	constexpr bool is_bottom_const_v = is_bottom_const<T>::value;

	template<typename First, typename... Types>
	struct TupleBase : TupleBase<Types...>
	{
		using Base = TupleBase<Types...>;
		First value;
		TupleBase(First &&fst, Types&&... args) : Base(std::forward<Types>(args)...), value(std::forward<First>(fst)) { }
		template<std::size_t N, typename T>
		constexpr T& base()
		{
			if constexpr (N)
				return Base::template base<N - 1, T>();
			else
				return value;
		}
		template<std::size_t N, typename T>
		constexpr const T& base() const
		{
			if constexpr (N)
				return Base::template base<N - 1, T>();
			else
				return value;
		}
	};
	template<typename Last>
	struct TupleBase<Last>
	{
		Last value;
		TupleBase(Last &&lst) : value(std::forward<Last>(lst)) { }
		template<std::size_t N, typename T>
		constexpr T& base()
		{
			return value;
		}
		template<std::size_t N, typename T>
		constexpr const T& base() const
		{
			return value;
		}
	};

	template<typename... Types>
	class /*alignas(std::aligned_union<sizeof...(Types), Types...>::alignment_value)*/ Tuple : private TupleBase<Types...>
	{
		using Base = TupleBase<Types...>;
	public:
		template<std::size_t N>
		using element = select_t<N, Types...>;
		static inline constexpr std::size_t size = sizeof...(Types);
		
		constexpr Tuple(Types&&... args) : Base(std::forward<Types>(args)...) { }

		template<std::size_t N>
		constexpr auto get() const -> const select_t<N, Types...>&
		{
			static_assert(N < size, "不存在该元素");
			using CurrentType = select_t<N, Types...>;
			return const_cast<Tuple*>(this)->base<N, CurrentType>();
		}

		template<std::size_t N>
		constexpr void set(const select_t<N, Types...> &val)
		{
			using CurrentType = select_t<N, Types...>;
			base<N, CurrentType>() = val;
		}
		template<std::size_t N>
		constexpr void set(select_t<N, Types...> &&val)
		{
			using CurrentType = select_t<N, Types...>;
			base<N, CurrentType>() = std::move(val);
		}
	private:
#if 0
		template<std::size_t N, typename T>
		constexpr auto base() -> std::conditional_t<is_bottom_const_v<decltype(this)>, T, T&>
		{
			return Base::template base<N, T>();
		}
#endif
		template<std::size_t N, typename T>
		constexpr T& base()
		{
			return Base::template base<N, T>();
		}
		template<std::size_t N, typename T>
		constexpr const T& base() const
		{
			return Base::template base<N, T>();
		}
	};
	template<>
	class Tuple<> { };

	template<std::size_t N, typename... Types>
	struct TupleElement;
	template<std::size_t N, typename First, typename... Types>
	struct TupleElement<N, Tuple<First, Types...>> : TupleElement<N - 1, Tuple<Types...>> { };
	template<typename First, typename... Types>
	struct TupleElement<0, Tuple<First, Types...>> { using type = First; };

	template<typename T, std::size_t N = T::size>
	std::ostream& printTuple(std::ostream &os, const T &tup)
	{
		static_assert(N != 0, "没有可遍历元素");
		if constexpr (N > 1)
		{
			printTuple<T, N - 1>(os, tup);
			os << ", ";
		}
		os << tup.template get<N - 1>();
		return os;
	}
	template<typename... Args>
	std::ostream& operator<<(std::ostream &os, const Tuple<Args...> &tup)
	{
		os << "{ ";
		printTuple(os, tup);
		return os << " }";
	}

	template<typename Ret, typename... Args>
	struct functionArgs;
	template<typename Ret, typename... Args>
	struct functionArgs<Ret(*)(Args...)>
	{
		template<std::size_t Index>
		using get = select_t<Index, Args...>;
		using ret = Ret;
		static inline constexpr std::size_t size = sizeof...(Args);
	};
	template<typename Ret, typename Class, typename... Args>
	struct functionArgs<Ret(Class::*)(Args...)>
	{
		template<std::size_t Index>
		using get = select_t<Index, Args...>;
		using className = Class;
		using ret = Ret;
		static inline constexpr std::size_t size = sizeof...(Args);
	};
}

template<typename T>
mystd::enable_if_t<std::is_integral_v<T>, std::string> test() { return "true"; }

template<typename T>
mystd::enable_if_t<!std::is_integral_v<T>, bool> test() { return false; }

class TypeC
{
public:
	TypeC() = default;
	TypeC(const TypeC&) { std::cout << "copy" << std::endl; }
	TypeC(TypeC &&) { std::cout << "move" << std::endl; }
	void operator()(int i) { }
};

void funa(int, int) { }
void funa(std::string, int) { std::cout << "??" << std::endl; }
template<typename T>
void fff(T &val) { std::cout << typeid(val).name() << std::endl; }

template<typename T, std::size_t N>
struct init { using type = std::initializer_list<typename init<T, N - 1>::type>; };
template<typename T>
struct init<T, 1> { using type = std::initializer_list<T>; };
template<typename T>
struct init<T, 0>;
template<typename T, std::size_t N>
using initList = typename init<T, N>::type;

template<std::size_t N, typename T, T... V>
initList<T, N> list = { list<N - 1, T, V...> };
template<typename T, T... V>
initList<T, 1> list<1, T, V...> = { V... };


template<std::size_t N>
struct placeholder { };

template<typename T>
struct is_placeholder : std::false_type { };
template<std::size_t N>
struct is_placeholder<placeholder<N>> : std::true_type { };

template<unsigned long N>
struct binary { static constexpr unsigned long value = N % 10 + binary<N / 10>::value * 2; };
template<>
struct binary<0> { static constexpr unsigned long value = 0; };


std::tuple<int, int, int> fuuu()
{
	return { 1, 2, 3 };
}

void ffff(const int * &pr) { }

template<std::size_t N>
struct Index
{
	static constexpr std::size_t value = N;
	static constexpr std::size_t next = N + 1;
	using next_type = Index<N + 1>;
};

template<typename Indexes>
void print()
{
	std::cout << Indexes::value << std::endl;
	if constexpr (Indexes::value < 1000)
		print<typename Indexes::next_type>();
};

template<typename T>
struct ClassA
{
	using value_type = T;
	using pointer = T*;

	T* allocate(std::size_t n) {
		std::cout << "A allocator: " << n << std::endl;
		return reinterpret_cast<T*>(new char[n * sizeof(T)]);
	}
	void deallocate(T *p, std::size_t n) {
		std::cout << "A deallocator" << std::endl;
		delete[] reinterpret_cast<char*>(p);
	}
	bool operator==(const ClassA &rhs) { return &rhs == this; }
};
template<typename T>
struct ClassB
{
	using value_type = T;
	using pointer = T*;

	T* allocate(std::size_t n) {
		std::cout << "B allocator" << std::endl;
		return reinterpret_cast<T*>(new char[n * sizeof(T)]);
	}
	void deallocate(T *p, std::size_t n) {
		std::cout << "B deallocator" << std::endl;
		delete[] reinterpret_cast<char*>(p);
	}
	bool operator==(const ClassB &rhs) { return &rhs == this; }
};

template<typename T> struct TEst
{
	using type1 = typename T::value_type;
	using type2 = typename T::pointer;
};

namespace chrono = std::chrono;

struct Fct
{
	int operator()(char) { return 10; }
	Fct(Fct &&) = delete;
	Fct& operator=(Fct &&) = delete;
	Fct(const Fct &) = default;
	Fct() = default;
	//int* operator()(std::string) { return nullptr; }
};


template<typename T, typename U>
void fill(T &ary, const U &val)
{
	for (std::size_t i = 0; i != ary.size(); ++i)
	{
		if (i <= ary.size() / 2)
		{
			ary[i][ary[i].size() / 2 + i] = val;
			ary[i][ary[i].size() / 2 - i] = val;
		}
		else
		{
			ary[i][i - ary[i].size() / 2] = val;
			ary[i][ary[i].size() - (i - ary[i].size() / 2) - 1] = val;
		}
		std::cout << ary[i] << std::endl;
	}
}

void start(const std::regex &pat)
{
	std::cout << std::boolalpha;
	for (std::string in; std::getline(std::cin, in); std::cout << std::regex_match(in, pat) << std::endl);
	std::cout << std::noboolalpha;
}

template<typename C, std::size_t MaxSize>
class MyStreamBuf : public std::basic_streambuf<C>
{
	using Base = std::basic_streambuf<C>;

public:
	MyStreamBuf() {
		Base::setp(buf.begin(), buf.end());
	}

	std::array<C, MaxSize> buf;
};

struct Money
{
	Money(long double val) : value(val) { }
	long double value;
};

template<typename T = char>
struct MyMoneyPunct : std::moneypunct<T>
{
	using Base = std::moneypunct<T>;
public:
	using Base::Base;

protected:
	typename Base::string_type do_positive_sign() const override { return "+"; }
	typename Base::string_type do_negative_sign() const override { return "()"; }
	int do_frac_digits() const override { return 2; }
	typename Base::string_type do_curr_symbol() const override { return "$"; }
	typename Base::pattern do_pos_format() const override { return pat; }
	typename Base::pattern do_neg_format() const override { return pat; }

	static constexpr typename Base::pattern pat{ Base::sign, Base::symbol, Base::space, Base::value };
};

std::ostream& operator<<(std::ostream &os, const Money &money)
{
	std::ostream::sentry stry(os);
	if (!stry) return os;
	try {
		const std::money_put<char> &mp = std::use_facet<std::money_put<char>>(os.getloc());
		if (mp.put(os, false, os, os.fill(), money.value).failed())
			os.setstate(std::ios_base::badbit);
	} catch (...) {
		if (os.exceptions() & std::ios_base::badbit)
			throw;
	}
	return os;
}

constexpr void* fuu() { return nullptr; }
constexpr const char* fiuuu() { return nullptr; }

template<auto V, auto V2>
struct Cl { static constexpr auto value = V; static constexpr auto value2 = V2; };

void Cppfunc() { std::cout << "ASF" << std::endl; }
extern "C" void Cfunc() { Cppfunc(); }

int main()
{
	Cfunc();
	int restrict;
	std::wcout << "ABCEFG" u"DEFLLL" << std::endl;
	auto i(10);
	std::thread t(fuu);
	std::this_thread::sleep_for(std::chrono::seconds(1));
	t.detach();
	t.~thread();
	//t.join();
	std::random_device rd;
	std::cout << rd.entropy() << std::endl;
	std::valarray<int> va {
		1, 2, 3,
		4, 5, 6,
		7, 8, 9,
		10, 11, 12
	};
	int num = 1;
	auto gva = va[std::gslice(4, { 3, 2 }, { 3, 1 })];
	gva = 0;
	va[std::gslice(4, { 3, 2 }, { 3, 1 })] = 0;
	for (int v : va)
	{
		std::cout << v;
		std::cout << (num++ % 3 == 0 ? '\n' : ' ');
	}
#if 0
	std::complex<float> d { 1.33333333333333333333333333333333333333333333333333333333333333333333 };
	std::cout << d << std::endl;
	std::locale::global(std::locale(""));
	std::cout.imbue(std::locale(""));
	std::time_t tm = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
	std::time_t tp = std::time(0);
	std::cout << std::put_time(std::localtime(&tm), "%c") << std::endl;
	std::cout << std::put_time(std::localtime(&tp), "%c") << std::endl;
	std::cout << std::ctime(&tm) << std::endl;
	std::cout << std::ctime(&tp) << std::endl;

	Cl<fuu(), fiuuu()> cl;
	const auto *p = "🍌asbasd";
	std::cout << p << std::endl;
	const auto &ret = std::cout.imbue({std::cout.getloc(), new MyMoneyPunct()});
	//std::cout.imbue(std::locale("en_US.UTF-8"));
	//std::cout << std::use_facet<std::moneypunct<char>>(std::cout.getloc()).curr_symbol() << std::endl;
	//std::cout << Money(-100.8) << std::endl;
	//std::cout << Money(100.8) << std::endl;
	std::cout << std::showbase << std::put_money(12345.2134, true) << std::endl;
	std::cout << std::showbase << std::put_money(-12345.2134) << std::endl;
	std::locale loc("");

	std::wstring s(L"ABCabc");
	std::wcout << s << std::endl;
	std::wcout << std::use_facet<std::collate<wchar_t>>(loc).transform(s.data(), s.data() + s.size()) << std::endl;
	std::cout << loc.name() << std::endl;
	//std::copy(std::istreambuf_iterator<char>(std::cin), std::istreambuf_iterator<char>(), std::ostreambuf_iterator<char>(std::cout));
	//std::copy(std::istream_iterator<char>(std::cin), std::istream_iterator<char>(), std::ostream_iterator<char>(std::cout, "\n"));

	//MyStreamBuf<char, 1> msb;
	std::streambuf *oldBuf = std::cout.rdbuf(new MyStreamBuf<char, 1>());
	std::cout << "hello, world" << std::endl;
	MyStreamBuf<char, 1> *p = dynamic_cast<MyStreamBuf<char, 1>*>(std::cout.rdbuf(oldBuf));
	std::cout << p->buf[0] << std::endl;
	//std::array<char, 1> bf;
	//buffer.setp(bf.begin(), bf.end());

	std::streambuf *buffer = std::cout.rdbuf();
	std::array<char, 10> c{};
	std::cout << "abcd";
	std::cout << buffer->sputc('e') << std::endl;

	std::valarray vr { 10, 20, 30, 40, 50 };
	//std::cout << vr << std::endl;

	//std::regex pat(R"([_[:alpha:]][_[:alnum:]]*)");
	//std::regex pat2(R"((\s*\w*)+)");
	std::cout << sizeof(std::regex) << std::endl;
	std::regex pat2(R"((s{2})\0)");
	std::string s;
	std::getline(std::cin, s);
	std::smatch matches;
	std::regex_search(s, matches, pat2);
	std::cout << "0: " << matches[0] << std::endl;
	std::cout << "1: " << matches[1] << std::endl;
	std::cout << "2: " << matches[2] << std::endl;

	std::result_of_t<decltype(&fffff)(int)> v;
	std::cout << typeid(decltype(v)).name() << std::endl;
	typename std::add_rvalue_reference<int&>::type rv = i;

	auto hsct = chrono::high_resolution_clock::now();
	auto sct = chrono::system_clock::now();
	auto stct = chrono::steady_clock::now();

	std::cout << chrono::steady_clock::period::num << ' ' << chrono::steady_clock::period::den << std::endl;
	std::cout << chrono::system_clock::period::num << ' ' << chrono::system_clock::period::den << std::endl;
	std::cout << chrono::high_resolution_clock::period::num << ' ' << chrono::high_resolution_clock::period::den << std::endl;
	std::cout << sct.time_since_epoch().count() << std::endl;
	std::cout << stct.time_since_epoch().count() << std::endl;
	std::cout << hsct.time_since_epoch().count() << std::endl;
	std::cout << chrono::treat_as_floating_point<chrono::duration<float>>::value << std::endl;

	typename TEst<ClassB>::type1 a;
	std::cout << sizeof(ClassB) << std::endl;
	std::shared_ptr<int> p(new int);
	std::shared_ptr p1(p);
	std::weak_ptr wp1(p), wp2(p), wp3(p);
	std::cout << wp1.use_count() << ' ' << wp2.use_count() << ' ' << wp3.use_count() << std::endl;
	std::shared_ptr<int> pp(new int);
	std::cout << pp.unique() << std::endl;

	print<Index<0>>();
	std::tuple<int, int, int> i = { 1, 2, 3 };
	std::tuple<std::string, std::vector<double>, int> t = std::make_tuple("AAA", std::vector<double>(), 'c');
	std::cout << sizeof(std::bitset<65>) << std::endl;
	std::bitset<64> bs;
	std::bitset<64> bs1;
	decltype(auto) d = 10;
	std::vector<bool> vec(10000);
#endif
#if 0
	vec[0] = 1;
	std::cout << *vec.begin() << std::endl;
	std::cout << *(vec.begin() + 1) << std::endl;
	bs = 100;
	bs.set();
	std::cout << bs.to_string() << std::endl;
	bs = bs1;
	std::cout << bs << std::endl;
	std::array<int, 0> a;

	//for (auto b : l)
	//constexpr bool b = mystd::has_f(static_cast<void(*)(std::string, int)>(funa), 20, 20);
	//std::cout << mystd::has_f(static_cast<void(*)(std::string, int)>(funa), 10, 20) << std::endl;
	//check<double>();
	//M<10, int> m;
	//m.template get<3>::value = 100;
	//sizeof(decltype(m)::value_type);
	//std::cout << m.template get<3>::value << '\n';
	//std::cout << m.template get<4>::value << '\n';
	//ff(vec);
	//std::cout << vec.front() << std::endl;
	//decltype(N::Class<int, 2>::c) *p;
	//N::Class<int, 2>::c;
	C a;
	//C c(std::move_if_noexcept(a));
	std::cout << std::boolalpha << b << std::noboolalpha << std::endl;
	std::vector<int> v;
	while(true)
		v.pop_back();
	A a { 10 };
	using namespace std;
	std::variant<int, long> i(10);
	//std::variant<int, std::string, const char*> isc (std::in_place, { 10, "ABC"s, "AAAAA" });
	using namespace std::literals;
	std::unique_ptr<char[]> p(new char[4]{'a', 'b', 'c'});
	"dfg"sv.size() == "dfg"sv.size();
	std::cout << "ABC\0edf\0WW" << std::endl;
	std::char_traits<char>::compare("A", "B", 2);
	std::cout << p.get() << std::endl;
	std::cout << s << std::endl;
#endif
#if 0
	std::string line;
	std::getline(std::cin, line);
	int *p = nullptr;
	fb(p);
	for(char &ch : line)
		if('A' <= ch && ch <= 'Z')
			ch = ch - (ch - 'A') + ('Z' - ch);
		else if('a' <= ch && ch <= 'z')
			ch = ch - (ch - 'a') + ('z' - ch);
	std::cout << line << std::endl;
#endif
}
#undef PRINT
