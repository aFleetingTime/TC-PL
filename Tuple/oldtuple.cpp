#include <string>
#include <type_traits>
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


#if __cplusplus == 201703
namespace mystd {
	template<bool, typename T = void>
	struct enable_if { using type = T; };

	template<typename T>
	struct enable_if<false, T> { };

	template<bool b, typename T>
	using enable_if_t = typename enable_if<b, T>::type;

	template<typename...>
	using bool_t = bool;

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
	struct select<N, First, Args...> : select<N - 1, Args...> { static_assert(N <= sizeof...(Args), "不存在该元素"); };
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

	template<typename... Types>
	struct Tuple;

	template<typename First, typename... Types>
	struct TupleBase : TupleBase<Types...>
	{
		using Base = TupleBase<Types...>;
		First value;
		constexpr TupleBase(First &&fst, Types&&... args) : Base(std::forward<Types>(args)...), value(std::forward<First>(fst)) { }
		template<typename... Args>
		constexpr TupleBase(TupleBase<Args...> &rhs) : Base(rhs.baseObject()), value(rhs.value) { }
		template<typename... Args>
		constexpr TupleBase(const TupleBase<Args...> &rhs) : Base(rhs.baseObject()), value(rhs.value) { }
		template<typename... Args>
		constexpr TupleBase(TupleBase<Args...> &&rhs) : Base(std::move(rhs.baseObject())), value(std::move(rhs.value)) { }

		friend class Tuple<First, Types...>;
		template<typename F, typename... T>
		friend class TupleBase;

		template<typename... Args>
		TupleBase& operator=(TupleBase<Args...> &rhs)
		{
			Base::operator=(rhs.baseObject());
			value = std::move(rhs.value);
			return *this;
		}
		template<typename... Args>
		TupleBase& operator=(const TupleBase<Args...> &rhs)
		{
			Base::operator=(rhs.baseObject());
			value = std::move(rhs.value);
			return *this;
		}
		template<typename... Args>
		TupleBase& operator=(TupleBase<Args...> &&rhs)
		{
			Base::operator=(std::move(rhs.baseObject()));
			value = std::move(rhs.value);
			return *this;
		}

		Base tail() { return *this; };
		const Base tail() const { return *this; };

		template<std::size_t N>
		constexpr const select_t<N, First, Types...>& get() const {
			return baseValue<N, select_t<N, First, Types...>>();
		}

	protected:
		template<std::size_t N, typename T>
		constexpr T& baseValue()
		{
			if constexpr (N)
				return Base::template baseValue<N - 1, T>();
			else return value;
		}
		template<std::size_t N, typename T>
		constexpr const T& baseValue() const
		{
			if constexpr (N)
				return Base::template baseValue<N - 1, T>();
			else return value;
		}

		constexpr Base& baseObject() {
			return *this;
		}
		constexpr const Base& baseObject() const {
			return *this;
		}
	};
	template<typename Last>
	struct TupleBase<Last>
	{
		using Type = std::remove_reference_t<Last>;
		Last value;
		template<typename T>
		constexpr TupleBase(const T &rhs) : value(rhs) { }
		template<typename T>
		constexpr TupleBase(T &rhs) : value(rhs) { }

		template<typename... Args>
		constexpr TupleBase(TupleBase<Args...> &rhs) : value(rhs.value) { }
		template<typename... Args>
		constexpr TupleBase(const TupleBase<Args...> &rhs) : value(rhs.value) { }
		template<typename... Args>
		constexpr TupleBase(TupleBase<Args...> &&rhs) : value(std::move(rhs.value)) { }

		friend class Tuple<Last>;
		template<typename F, typename... T>
		friend class TupleBase;

		template<typename... Args>
		TupleBase& operator=(TupleBase<Args...> &rhs)
		{
			value = std::move(rhs.value);
			return *this;
		}
		template<typename... Args>
		TupleBase& operator=(const TupleBase<Args...> &rhs)
		{
			value = std::move(rhs.value);
			return *this;
		}
		template<typename... Args>
		TupleBase& operator=(TupleBase<Args...> &&rhs)
		{
			value = std::move(rhs.value);
			return *this;
		}

		template<std::size_t N>
		constexpr const Last& get() const {
			return value;
		}

	protected:
		template<std::size_t N, typename T>
		constexpr T& baseValue() {
			return value;
		}
		template<std::size_t N, typename T>
		constexpr const T& baseValue() const {
			return value;
		}
	};

	template<typename... Types>
	class Tuple : public TupleBase<Types...>
	{
		template<typename... Args> friend class Tuple;
		using Base = TupleBase<Types...>;

	public:
		template<std::size_t N>
		using element = select_t<N, Types...>;
		static inline constexpr std::size_t size = sizeof...(Types);
		
		explicit constexpr Tuple(Types&&... args) : TupleBase<Types...>(std::forward<Types>(args)...) { }

		template<typename... Args, typename = std::enable_if_t<size == sizeof...(Args)>>
		Tuple(Tuple<Args...> &rhs) : Base(rhs.baseObject()) { }
		template<typename... Args, typename = std::enable_if_t<size == sizeof...(Args)>>
		Tuple(const Tuple<Args...> &rhs) : Base(rhs.baseObject()) { }
		template<typename... Args, typename = std::enable_if_t<size == sizeof...(Args)>>
		Tuple(Tuple<Args...> &&rhs) : Base(std::move(rhs.baseObject())) { }

		template<typename... Args, typename = std::enable_if_t<size == sizeof...(Args)>>
		Tuple& operator=(Tuple<Args...> &rhs) { Base::template operator=(rhs.baseObject()); return *this; }
		template<typename... Args, typename = std::enable_if_t<size == sizeof...(Args)>>
		Tuple& operator=(const Tuple<Args...> &rhs) { Base::template operator=(rhs.baseObject()); return *this; }
		template<typename... Args, typename = std::enable_if_t<size == sizeof...(Args)>>
		Tuple& operator=(Tuple<Args...> &&rhs) { Base::template operator=(std::move(rhs.baseObject())); return *this; }

		template<std::size_t N>
		constexpr const select_t<N, Types...>& get() const {
			return baseValue<N>();
		}

		template<std::size_t N>
		constexpr std::enable_if_t<(N < Tuple::size)> set(const std::remove_reference_t<select_t<N, Types...>> &val) {
			baseValue<N>() = val;
		}
		template<std::size_t N>
		constexpr std::enable_if_t<(N < Tuple::size)> set(std::remove_reference_t<select_t<N, Types...>> &&val) {
			baseValue<N>() = std::move(val);
		}

		Base tail() { return *this; };
		const Base tail() const { return *this; };

	private:
		Base& baseObject() { return static_cast<Base&>(*this); };
		const Base& baseObject() const { return static_cast<const Base&>(*this); };

		template<std::size_t N>
		constexpr select_t<N, Types...>& baseValue() {
			return Base::template baseValue<N, select_t<N, Types...>>();
		}
		template<std::size_t N>
		constexpr  const select_t<N, Types...>& baseValue() const {
			return Base::template baseValue<N, select_t<N, Types...>>();
		}
	};
	template<>
	class Tuple<> { };

	template<typename... Args>
	Tuple(Args...) -> Tuple<Args...>;

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
	template<>
	std::ostream& operator<<(std::ostream &os, const Tuple<> &tup)
	{
		return os << "{ }";
	}

	template<typename... Args>
	Tuple<Args...> makeTuple(Args&&... args)
	{
		return Tuple{ std::forward<Args>(args)... };
	}

	//template<typename First, typename... Tail>
	//Tuple<Tail&...> tail(Tuple<First, Tail...> tup)
	//{
	//	constexpr std::size_t diff = sizeof(tup) - sizeof(Tuple<Tail...>);
	//	return *reinterpret_cast<Tuple<Tail...>*>(reinterpret_cast<char*>(&tup) + diff);
	//}
	template<typename First, typename... Tail>
	Tuple<Tail&...> tail(Tuple<First, Tail...> tup)
	{
		constexpr std::size_t diff = sizeof(tup) - sizeof(Tuple<Tail...>);
		return *reinterpret_cast<Tuple<Tail...>*>(reinterpret_cast<char*>(&tup) + diff);
	}
}

template<typename T>
mystd::enable_if_t<std::is_integral_v<T>, std::string> test() { return "true"; }

template<typename T>
mystd::enable_if_t<!std::is_integral_v<T>, bool> test() { return false; }
#endif

template<typename... Args>
void func(mystd::Tuple<Args...> tup)
{
	std::cout << tup << std::endl;
}

template<typename A, typename B, typename C>
class TypeC
{
public:
	A a;
	B b;
	C c;
	TypeC() = default;
	template<typename TA, typename TB, typename TC>
	TypeC(const TypeC<TA, TB, TC> &rhs) : a(rhs.a), b(rhs.b), c(rhs.c) { }
};

int main()
{
	func(mystd::makeTuple(10, 20));
	mystd::Tuple tp(1.2, 10, 'a', 10, "AAA");
	tp.set<4>("GGGG");
	//mystd::Tuple<double&, const int&, const char&, long, const char*> aa(tp);
	//mystd::Tuple bb(aa);
	//bb.set<0>(1000.777);
	//mystd::Tuple<double&, int&, char&, long, const char*> cc(aa);
	mystd::Tuple<> ttt;
	std::cout << tp << std::endl;
	//std::cout << aa << std::endl;
	//std::cout << bb << std::endl;
	//mystd::Tuple<int, char, int, char*> b = tp.tail();
	auto t(tp.tail().tail());
	std::cout << t.get<0>() << std::endl;
	std::cout << tp.get<0>() << std::endl;
	//std::cout << b << std::endl;
}
