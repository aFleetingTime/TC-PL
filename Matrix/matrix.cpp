#include <array>
#include <string>
#include <vector>
#include <numeric>
#include <iostream>
#include <stdexcept>
#include <functional>
#include <type_traits>

// 13增：slice切片、输出运算格式化。
// 14日新增：矩阵运算、col函数, 内存占用优化
// 不知道啥时候新增：Slice切片元素间距

namespace mystd::matrix::impl {
template<typename T>
using InitList = std::initializer_list<T>;

struct ExtentsError : std::logic_error {
	using std::logic_error::logic_error;
};

template<std::size_t Dime, typename OutputIt>
void formStrides(const std::array<std::size_t, Dime> &extents, std::size_t size, OutputIt dest) {
	if (std::any_of(extents.cbegin(), extents.cend(), [](std::size_t v) { return v == 0; }))
		throw ExtentsError("extents cannot be zero");
	for (std::size_t extent : extents)
		*dest++ = extent ? size /= extent : 0;
}

template<typename T>
void debugPrint(T t)
{
	for (auto v : t)
		std::cout << v << ' ';
	std::cout << std::endl;
}

template<std::size_t N>
std::size_t offset(const std::array<std::size_t, N> &exts, std::size_t n) {
	return n * std::accumulate(exts.cbegin() + 1, exts.cend(), std::size_t(1), std::multiplies<std::size_t>());
}

template<typename T>
struct PeelOffInitList { using type = T; };
template<typename T>
struct PeelOffInitList<InitList<T>> : PeelOffInitList<T> { };

template<typename Type, std::size_t Dime>
struct MatrixInitList {
	using init_list_type = InitList<typename MatrixInitList<Type, Dime - 1>::init_list_type>;
	static constexpr inline std::size_t dime = Dime;
};
template<typename Type>
struct MatrixInitList<Type, 1> {
	using init_list_type = InitList<Type>;
	static constexpr inline std::size_t dime = 1;
};
template<typename Type>
struct MatrixInitList<Type, 0>;

template<typename T, typename First, typename... Tail>
struct IsAll : std::bool_constant<std::is_same_v<First, T> && IsAll<T, Tail...>::value> { };
template<typename T, typename Last>
struct IsAll<T, Last> : std::bool_constant<std::is_same_v<Last, T>> { };

template<template<typename> typename Template, typename First, typename... Tail>
struct TemplateIsAll : std::bool_constant<Template<First>::value && TemplateIsAll<Template, Tail...>::value> { };
template<template<typename> typename Template, typename Last>
struct TemplateIsAll<Template, Last> : std::bool_constant<Template<Last>::value> { };

template<typename T>
struct IsReferenceWrp : std::false_type { };
template<typename T>
struct IsReferenceWrp<std::reference_wrapper<T>> : std::true_type { };

template<typename Type, std::size_t Dime>
std::enable_if_t<(Dime > 1), std::size_t> maxChildInit(const typename MatrixInitList<Type, Dime>::init_list_type &init)
{
	std::size_t max = 0;
	for (auto beg = init.begin(); beg != init.end(); ++beg)
		if (max < beg->size())
			max = beg->size();
	return max;
}
template<typename Type, std::size_t Dime>
std::enable_if_t<(Dime == 1), std::size_t> maxChildInit(const typename MatrixInitList<Type, Dime>::init_list_type &init)
{
	return init.size();
}

template<typename Type, std::size_t Dime, typename OutputIt>
std::enable_if_t<(Dime == 1), typename std::iterator_traits<OutputIt>::value_type> addExtents
(OutputIt iter, const typename MatrixInitList<Type, Dime>::init_list_type &init)
{
	return init.size();
}
template<typename Type, std::size_t Dime, typename OutputIt>
std::enable_if_t<(Dime > 1), typename std::iterator_traits<OutputIt>::value_type> addExtents
(OutputIt iter, const typename MatrixInitList<Type, Dime>::init_list_type &init)
{
	typename std::iterator_traits<OutputIt>::value_type max = 0;
	for (auto beg = init.begin(); beg != init.end(); ++beg)
		if (auto ret = addExtents<Type, Dime - 1>(iter + 1, *beg); max < ret)
			max = ret;
	if (*iter < max) *iter = max;
	return init.size();
}

template<typename Type, std::size_t Dime>
std::array<std::size_t, Dime> deriveExtents(const typename MatrixInitList<Type, Dime>::init_list_type &init)
{
	std::array<std::size_t, Dime> extents{};
	extents.front() = addExtents<Type, Dime>(extents.begin() + 1, init);
	return extents;
}
}

namespace mystd::matrix {
template<typename T>
using InitList = std::initializer_list<T>;

template<typename Type, std::size_t Dime>
using MatrixInitList = typename impl::MatrixInitList<Type, Dime>::init_list_type;

template<typename T>
constexpr bool IsReferenceWrp = impl::IsReferenceWrp<T>::value;

class Slice;
template<typename Type, std::size_t Dime>
class Matrix;
template<typename Type, std::size_t Dime>
class MatrixIterator;
template<typename Type, std::size_t Dime>
class ConstMatrixIterator;
template<typename Type, std::size_t Dime>
using MatrixRef = Matrix<std::reference_wrapper<Type>, Dime>;
template<typename Type, std::size_t Dime>
using ConstMatrixRef = Matrix<std::reference_wrapper<const Type>, Dime>;

template<typename T>
struct IsSliceOrIntegral : std::bool_constant<std::is_same_v<T, Slice> || std::is_integral_v<T>> { };
template<typename First, typename... Tail>
constexpr bool IsAllSlice = impl::TemplateIsAll<IsSliceOrIntegral, First, Tail...>::value;
template<typename First, typename... Tail>
constexpr bool IsAllIntegral = impl::TemplateIsAll<std::is_integral, First, Tail...>::value;

template<typename T>
struct IsMatrixIter : std::false_type { };
template<typename T, std::size_t D>
struct IsMatrixIter<MatrixIterator<T, D>> : std::true_type { };
template<typename T, std::size_t D>
struct IsMatrixIter<ConstMatrixIterator<T, D>> : std::true_type { };

// Slice切片访问
struct Slice
{
	static inline constexpr std::size_t npos = std::numeric_limits<std::size_t>::max();
	constexpr explicit Slice(std::size_t s = 0, std::size_t len = npos, std::size_t stri = 1) : start(s), length(len), stride(stri) { }

	constexpr std::size_t operator()(std::size_t n) const noexcept { return stride * n + start; }

	static constexpr Slice createSlice(std::size_t start) {
		return Slice(start, 1);
	}
	static constexpr const Slice& createSlice(const Slice &slice) {
		return slice;
	}

	std::size_t start, length, stride;
};

// MatrixInitList解析器
template<typename Type, std::size_t Dime>
class MatrixParser
{
	static_assert(Dime != 0, "there is no 0 dimension matrix");
public:
	using init_list = MatrixInitList<Type, Dime>;

	MatrixParser(const MatrixParser&) = default;
	MatrixParser(MatrixParser&&) = default;
	MatrixParser& operator=(const MatrixParser&) = default;
	MatrixParser& operator=(MatrixParser&&) = default;

	MatrixParser(const init_list &init, const std::array<std::size_t, Dime> &exts) : extents(exts), dimeMaxSize(minDimeMaxSize<Dime>(init))
	{
		list.resize(std::accumulate(extents.cbegin(), extents.cend(), std::size_t(1), std::multiplies<std::size_t>()));
		initStrides();
		parser<Dime>(init, list.begin());
		list.shrink_to_fit();
	}
	MatrixParser(const init_list &init) : extents(impl::deriveExtents<Type, Dime>(init)), dimeMaxSize(minDimeMaxSize<Dime>(init))
	{
		list.resize(std::accumulate(extents.cbegin(), extents.cend(), std::size_t(1), std::multiplies<std::size_t>()));
		initStrides();
		parser<Dime>(init, list.begin());
		list.shrink_to_fit();
	}

	const std::vector<Type>& parser() const noexcept {
		return list;
	}
	std::vector<Type>& parser() noexcept {
		return list;
	}

	constexpr std::size_t dime() const noexcept {
		return Dime;
	}

	void copy(std::vector<Type> &dest) const {
		dest = list;
	}

	const std::array<std::size_t, Dime>& getExtents() const noexcept {
		return extents;
	}

private:
	template<std::size_t N, typename T>
	std::enable_if_t<N == 1> parser(const MatrixInitList<Type, N> &init, T &&it)
	{
		it = std::copy(init.begin(), init.end(), it);
		if (init.size() < extents.back())
			it += strides[Dime - N] * (extents.back() - init.size());
	}
	template<std::size_t N, typename T>
	std::enable_if_t<(N > 1)> parser(const MatrixInitList<Type, N> &init, T &&it)
	{
		for (auto beg = init.begin(); beg != init.end(); ++beg)
			parser<N - 1>(*beg, it);
		if (init.size() < extents[Dime - N])
			it += strides[Dime - N] * (extents[Dime - N] - init.size());
	}

	template<std::size_t N>
	std::enable_if_t<N == 1, std::size_t> minDimeMaxSize(const MatrixInitList<Type, N> &list) const noexcept
	{
		return list.size();
	}
	template<std::size_t N>
	std::enable_if_t<(N > 1), std::size_t> minDimeMaxSize(const MatrixInitList<Type, N> &list) const noexcept
	{
		std::size_t max = 0;
		for (auto beg = list.begin(); beg != list.end(); ++beg)
			if (std::size_t ret = minDimeMaxSize<N - 1>(*beg); max < ret)
				max = ret;
		return max;
	}

	void initStrides() noexcept
	{
		std::size_t temp = std::accumulate(extents.cbegin(), extents.cend(), std::size_t(1), std::multiplies<std::size_t>());
		auto dest = strides.begin();
		for (auto beg = extents.cbegin(); beg != extents.cend(); ++beg, ++dest)
			*dest = temp /= *beg;
	}

	std::vector<Type> list;
	const std::array<std::size_t, Dime> extents;
	std::array<std::size_t, Dime> strides;
	const std::size_t dimeMaxSize;
};

// Matrix切片
template<std::size_t Dime>
class MatrixSlice
{
	static_assert(Dime != 0, "there is no 0 dimension matrix");
	template<typename T, std::size_t D> friend class Matrix;
	template<typename T, std::size_t D> friend class MatrixBase;

public:
	using init_list = InitList<std::size_t>;
	using array = std::array<std::size_t, Dime>;

	MatrixSlice& operator=(const MatrixSlice&) = default;

	MatrixSlice() : extents{}, strides{}, size(0), start(0) { }

	MatrixSlice(std::size_t s, const array &exts)
		: extents(exts), size(initSize()), start(s) { impl::formStrides(extents, size, strides.begin()); }
	MatrixSlice(std::size_t s, const array &exts, const array &strs)
		: extents(exts), strides(strs), size(initSize()), start(s) { }

	MatrixSlice(const array &stas, const array &exts, const MatrixSlice &other) :
		start(other.start)
	{
		initExtsStrs(stas, exts, other.extents, other.strides);
		size = initSize();
		impl::formStrides(extents, size, strides.begin());
		strides = other.strides;
		if (checkArgs(stas, extents, other.extents))
			throw std::out_of_range("MatrixSlice stas exts: out of range");
	}

	template<typename... Exts, typename = std::enable_if_t<IsAllIntegral<Exts...>>>
	explicit MatrixSlice(Exts... exts)  : extents{exts...}, size(initSize()), start(0) { impl::formStrides(extents, size, strides.begin()); }

	template<typename... Dimes, typename = std::enable_if_t<IsAllIntegral<Dimes...>>>
	std::size_t operator()(Dimes... dimes) const
	{
		static_assert(sizeof...(Dimes) == Dime, "dimensions do not match");
		array temp { std::size_t(dimes)... };
		if (checkDimes(temp))
			throw std::out_of_range("dimes operator(): out of range");
		return std::inner_product(temp.cbegin(), temp.cend(), strides.cbegin(), std::size_t(0)) + start;
	}
	template<typename T>
	std::size_t operator()(std::array<T, Dime> dimes) const
	{
		static_assert(std::is_convertible_v<T, std::size_t>);
		array temp;
		std::copy(dimes.cbegin(), dimes.cend(), temp.begin());
		if (checkDimes(temp))
			throw std::out_of_range("dimes operator(): out of range");
		return std::inner_product(temp.cbegin(), temp.cend(), strides.cbegin(), std::size_t(0)) + start;
	}

	std::size_t rows() const noexcept { return extents.front(); }
	std::size_t cols() const noexcept { return extents.back(); }

	const auto& getExtents() const noexcept { return extents; }
	const auto& getStrides() const noexcept { return strides; }
	std::size_t extent(std::size_t n) const noexcept { return extents[n]; }
	std::size_t stride(std::size_t n) const noexcept { return strides[n]; }
	std::size_t getStart() const noexcept { return start; }
	std::size_t getSize() const noexcept { return size; }

private:
	std::size_t initSize() {
		return std::accumulate(extents.cbegin(), extents.cend(), std::size_t(1), std::multiplies<std::size_t>());
	}

	void initExtsStrs(const array &stas, const array &exts, const array &oldExts, const array &oldStrs)
	{
		for (std::size_t i = 0; i < Dime; ++i)
		{
			extents[i] = (exts[i] == Slice::npos) ? oldExts[i] - stas[i] : exts[i];
			start += stas[i] * oldStrs[i];
		}
	}

	bool checkDimes(const array &dimes) const noexcept
	{
		auto beg = extents.cbegin();
		for (auto dime : dimes)
			if (*beg++ <= dime)
				return true;
		return false;
	}

	bool checkArgs(const array &stas, const array &exts, const array &oldExts)
	{
		for (std::size_t i = 0; i < Dime; ++i)
			if ((oldExts[i] < (stas[i] + exts[i])))
				return true;
		return false;
	}

	array extents;
	array strides;
	std::size_t size;
	std::size_t start;
};

// Matrix迭代器公共基类
template<typename Type, std::size_t Dime>
class MatrixIterBase
{
protected:
	using array = std::array<std::size_t, Dime>;

	MatrixIterBase() : slice(nullptr) { }
	MatrixIterBase(const MatrixSlice<Dime> *sle) : slice(sle) { }

	std::size_t toIndex(std::size_t sz) const noexcept
	{
		array indexes{}, strides;
		impl::formStrides(slice->getExtents(), slice->getSize(), strides.begin());
		for (std::size_t i = 0; i < Dime - 1; ++i)
		{
			auto result = std::ldiv(sz, strides[i]);
			indexes[i] = result.quot;
			sz = result.rem;
			if (result.rem == 0)
				break;
		}
		indexes.back() = sz / strides.back();
		return (*slice)(indexes);
	}

	const MatrixSlice<Dime> *slice{};
};

// Matrix迭代器
template<typename Type, std::size_t Dime>
class MatrixIterator : private MatrixIterBase<Type, Dime>
{
	using Base = MatrixIterBase<Type, Dime>;

public:
	using size_type = std::size_t;
	using difference_type = std::ptrdiff_t;
	using value_type = Type;
	using pointer = value_type*;
	using const_pointer = const value_type*;
	using reference = value_type&;
	using const_reference = const value_type&;
	using iterator_category = std::random_access_iterator_tag;

	MatrixIterator(const MatrixIterator&) = default;
	MatrixIterator(MatrixIterator&&) = default;
	MatrixIterator& operator=(const MatrixIterator&) = default;
	MatrixIterator& operator=(MatrixIterator&&) = default;

	MatrixIterator() : elems(nullptr), current(nullptr) { }
	MatrixIterator(const MatrixSlice<Dime> *sle, Type *elms) : Base(sle), elems(elms), current(elms) { }
	MatrixIterator(const MatrixSlice<Dime> *sle, Type *elms, Type *last) : Base(sle), elems(elms), current(last) { }

	Type& operator*() noexcept {
		return *(elems + this->toIndex(current - elems));
	}
	const Type& operator*() const noexcept {
		return *(elems + this->toIndex(current - elems));
	}
	Type* operator->() noexcept {
		return elems + this->toIndex(current - elems);
	}
	const Type* operator->() const noexcept {
		return elems + this->toIndex(current - elems);
	}

	MatrixIterator& operator++() noexcept
	{
		++current;
		return *this;
	}
	MatrixIterator& operator--() noexcept
	{
		--current;
		return *this;
	}

	MatrixIterator operator++(int) noexcept
	{
		MatrixIterator ret(*this);
		++current;
		return ret;
	}
	MatrixIterator operator--(int) noexcept
	{
		MatrixIterator ret(*this);
		--current;
		return ret;
	}

	MatrixIterator& operator+=(std::ptrdiff_t n) noexcept
	{
		current += n;
		return *this;
	}
	MatrixIterator& operator-=(std::ptrdiff_t n) noexcept
	{
		current -= n;
		return *this;
	}

	MatrixIterator operator+(std::ptrdiff_t n) const noexcept
	{
		return MatrixIterator(*this) += n;
	}
	MatrixIterator operator-(std::ptrdiff_t n) const noexcept
	{
		return MatrixIterator(*this) -= n;
	}

	std::ptrdiff_t operator-(const MatrixIterator &rhs) const noexcept {
		return current - rhs.current;
	}

	Type* get() noexcept { return current; };
	const Type* get() const noexcept { return current; };

private:
	Type *elems;
	Type *current;
};

// Const Matrix迭代器
template<typename Type, std::size_t Dime>
class ConstMatrixIterator : private MatrixIterBase<Type, Dime>
{
	using Base = MatrixIterBase<Type, Dime>;

public:
	using size_type = std::size_t;
	using difference_type = std::ptrdiff_t;
	using value_type = Type;
	using pointer = value_type*;
	using const_pointer = const value_type*;
	using reference = value_type&;
	using const_reference = const value_type&;
	using iterator_category = std::random_access_iterator_tag;

	ConstMatrixIterator() : elems(nullptr) { }
	ConstMatrixIterator(const MatrixSlice<Dime> *sle, const Type *elms) :
		Base(sle), elems(elms), current(elms) { }
	ConstMatrixIterator(const MatrixSlice<Dime> *sle, const Type *elms, const Type *last) :
		Base(sle), elems(elms), current(last) { }

	const Type& operator*() const noexcept {
		return *(elems + this->toIndex(current - elems));
	}

	const Type* operator->() const noexcept {
		return elems + this->toIndex(current - elems);
	}

	ConstMatrixIterator& operator++() noexcept
	{
		++current;
		return *this;
	}
	ConstMatrixIterator& operator--() noexcept
	{
		--current;
		return *this;
	}

	ConstMatrixIterator operator++(int) noexcept
	{
		MatrixIterator ret(*this);
		++current;
		return ret;
	}
	ConstMatrixIterator operator--(int) noexcept
	{
		MatrixIterator ret(*this);
		--current;
		return ret;
	}

	ConstMatrixIterator& operator+=(std::ptrdiff_t n) noexcept
	{
		current += n;
		return *this;
	}
	ConstMatrixIterator& operator-=(std::ptrdiff_t n) noexcept
	{
		current -= n;
		return *this;
	}

	ConstMatrixIterator operator+(std::ptrdiff_t n) const noexcept
	{
		return ConstMatrixIterator(*this) += n;
	}
	ConstMatrixIterator operator-(std::ptrdiff_t n) const noexcept
	{
		return ConstMatrixIterator(*this) -= n;
	}

	std::ptrdiff_t operator-(const ConstMatrixIterator &rhs) const noexcept {
		return current - rhs.current;
	}

	const Type* get() const noexcept { return current; };

private:
	const Type *elems;
	const Type *current;
};

// Matrix元素容器
template<typename Type>
class MatrixElements : public std::vector<Type>
{
public:
	using Base = std::vector<Type>;
	using Base::Base;
	using value_type = Type;

	template<std::size_t Dime>
	MatrixElements(MatrixParser<Type, Dime> &&init) noexcept : Base(std::move(init.parser())) { }

	template<typename T, std::size_t Dime, typename = std::enable_if_t<std::is_convertible_v<T, Type>>>
	MatrixElements(const MatrixParser<T, Dime> &init) : Base(init.parser().cbegin(), init.parser().cend()) { }

	template<std::size_t Dime>
	MatrixElements& operator=(MatrixParser<Type, Dime> &&init) noexcept
	{
		Base::operator=(std::move(init.parser()));
		return *this;
	}
	template<typename T, std::size_t Dime>
	MatrixElements& operator=(const MatrixParser<T, Dime> &init)
	{
		static_assert(std::is_convertible_v<T, Type>);
		Base::assign(init.parser().cbegin(), init.parser().cend());
		return *this;
	}
};

// Matrix矩阵公共接口
template<typename Type, std::size_t Dime>
class MatrixBase
{
protected:
	static inline constexpr std::size_t DimeReduction = Dime - 1;
	using array = std::array<std::size_t, DimeReduction>;

	template<typename T>
	MatrixRef<Type, DimeReduction> rowImpl(Matrix<T, Dime> &mat, std::size_t n)
	{
		if constexpr (Dime == 1)
			return mat.elems[mat.slice(n)];
		else
		{
			if (n >= mat.slice.rows())
				throw std::out_of_range("row: out of range");
			array extents, strides;
			std::copy(mat.slice.extents.cbegin() + 1, mat.slice.extents.cend(), extents.begin());
			std::copy(mat.slice.strides.cbegin() + 1, mat.slice.strides.cend(), strides.begin());
			return { MatrixSlice<DimeReduction>(mat.slice.start + (n * mat.slice.strides.front()), extents, strides), mat.elems };
		}
	}
	template<typename T>
	ConstMatrixRef<Type, DimeReduction> rowImpl(const Matrix<T, Dime> &mat, std::size_t n) const
	{
		if constexpr (Dime == 1)
			return mat.elems[mat.slice(n)];
		else
		{
			if (n >= mat.slice.rows())
				throw std::out_of_range("row: out of range");
			array extents, strides;
			std::copy(mat.slice.extents.cbegin() + 1, mat.slice.extents.cend(), extents.begin());
			std::copy(mat.slice.strides.cbegin() + 1, mat.slice.strides.cend(), strides.begin());
			return { MatrixSlice<DimeReduction>(mat.slice.start + (n * mat.slice.strides.front()), extents, strides), mat.elems };
		}
	}

	template<typename T>
	MatrixRef<Type, DimeReduction> colImpl(Matrix<T, Dime> &mat, std::size_t n)
	{
		if constexpr (Dime == 1)
			return mat.elems[mat.slice(n)];
		else
		{
			if (n >= mat.slice.cols())
				throw std::out_of_range("col: out of range");
			array extents, strides;
			std::copy(mat.slice.extents.cbegin(), mat.slice.extents.cend() - 1, extents.begin());
			std::copy(mat.slice.strides.cbegin(), mat.slice.strides.cend() - 1, strides.begin());
			return { MatrixSlice<DimeReduction>(mat.slice.start + (n * mat.slice.strides.back()), extents, strides), mat.elems };
		}
	}
	template<typename T>
	ConstMatrixRef<Type, DimeReduction> colImpl(const Matrix<T, Dime> &mat, std::size_t n) const
	{
		if constexpr (Dime == 1)
			return mat.elems[mat.slice(n)];
		else
		{
			if (n >= mat.slice.cols())
				throw std::out_of_range("col: out of range");
			array extents, strides;
			std::copy(mat.slice.extents.cbegin(), mat.slice.extents.cend() - 1, extents.begin());
			std::copy(mat.slice.strides.cbegin(), mat.slice.strides.cend() - 1, strides.begin());
			return { MatrixSlice<DimeReduction>(mat.slice.start + (n * mat.slice.strides.back()), extents, strides), mat.elems };
		}
	}
};


// Matrix矩阵
template<typename Type, std::size_t Dime>
class Matrix : MatrixBase<Type, Dime>
{
	template<typename T, std::size_t D> friend class Matrix;
	friend class MatrixBase<Type, Dime>;
	using Base = MatrixBase<Type, Dime>;

public:
	using Base::DimeReduction;
	using value_type = Type;
	using reference = value_type&;
	using const_reference = const value_type&;
	using size_type = std::size_t;
	using iterator = typename std::vector<Type>::iterator;
	using const_iterator = typename std::vector<Type>::const_iterator;

	Matrix() = default;
	Matrix(const Matrix &) = default;
	Matrix& operator=(const Matrix &) = default;
	Matrix(Matrix &&) = default;
	Matrix& operator=(Matrix &&rhs) noexcept
	{
		if (&rhs != this)
		{
			slice = rhs.slice;
			elems = std::move(rhs.elems);
		}
		return *this;
	}

	template<typename... Exts, typename = std::enable_if_t<sizeof...(Exts) == Dime && IsAllIntegral<Exts...>>>
	explicit Matrix(Exts... exts) : slice(std::size_t(exts)...), elems(slice.size) { }
	template<typename... Exts, typename = std::enable_if_t<sizeof...(Exts) == Dime && !IsAllIntegral<Exts...>>>
	explicit Matrix(Type val, Exts... exts) : slice(std::size_t(exts)...), elems(slice.size, val) { }

	Matrix(const MatrixInitList<Type, Dime> &init) :
		slice(0, impl::deriveExtents<Type, Dime>(init)), elems(MatrixParser<Type, Dime>(init, slice.getExtents())) { }

	Matrix& operator=(const MatrixInitList<Type, Dime> &init)
	{
		slice = MatrixSlice<Dime>(0, impl::deriveExtents<Type, Dime>(init));
		elems = MatrixParser<Type, Dime>(init);
		return *this;
	}
	template<typename T>
	Matrix& operator=(const MatrixParser<T, Dime> &init)
	{
		static_assert(std::is_convertible_v<T, Type>);
		slice = MatrixSlice<Dime>(0, init.getExtents());
		elems = init;
		return *this;
	}
	Matrix& operator=(MatrixParser<Type, Dime> &&init)
	{
		slice = MatrixSlice<Dime>(0, init.getExtents());
		elems = std::move(init);
		return *this;
	}

	template<typename T>
	Matrix& operator=(const Matrix<std::reference_wrapper<T>, Dime> &init)
	{
		static_assert(std::is_convertible_v<T, Type>);
		slice = MatrixSlice<Dime>(0, init.slice.extents);
		elems.assign(init.begin(), init.end());
		return *this;
	}
	template<typename T>
	Matrix& operator=(const Matrix<T, Dime> &init)
	{
		static_assert(std::is_convertible_v<T, Type>);
		slice = MatrixSlice<Dime>(0, init.slice.extents);
		elems.assign(init.begin(), init.end());
		return *this;
	}

	template<typename T, typename = std::enable_if_t<std::is_convertible_v<T, Type>>>
	Matrix(const Matrix<T, Dime> &rhs) : slice(rhs.slice), elems(rhs.elems.data(), rhs.elems.data() + rhs.elems.size()) { }

	template<typename T, typename = std::enable_if_t<std::is_convertible_v<T, Type>>>
	Matrix(const MatrixParser<T, Dime> &psr) : slice(0, psr.getExtents()), elems(psr) { }

	Matrix(MatrixParser<Type, Dime> &&psr) : slice(0, psr.getExtents()), elems(std::move(psr)) { }

	std::size_t size() const noexcept { return slice.size; }
	std::size_t extent(std::size_t n = 0) const noexcept { return slice.extent(n); }
	constexpr static std::size_t dimensions() { return Dime; }
	std::size_t rows() const noexcept { return slice.rows(); }
	std::size_t cols() const noexcept { return slice.cols(); }

	template<typename F>
	Matrix& apply(F func) {
		for (Type &elem : *this)
			func(elem);
		return *this;
	}
	template<typename M, typename F>
	Matrix& apply(const M &mat, F func) {
		for (auto lhsBegin = begin(), rhsBegin = mat.cbegin(); lhsBegin != end(); ++lhsBegin, ++rhsBegin)
			func(*lhsBegin, *rhsBegin);
		return *this;
	}
	template<typename F>
	Matrix& apply(F func) const {
		for (Type &elem : *this)
			func(elem);
		return *this;
	}
	template<typename M, typename F>
	Matrix& apply(const M &mat, F func) const {
		for (auto lhsBegin = begin(), rhsBegin = mat.cbegin(); lhsBegin != end(); ++lhsBegin, ++rhsBegin)
			func(*lhsBegin, *rhsBegin);
		return *this;
	}

	Matrix& operator+=(const Type &val) { return apply([&val](auto &elem){ elem += val; }); }
	Matrix& operator-=(const Type &val) { return apply([&val](auto &elem){ elem -= val; }); }
	Matrix& operator*=(const Type &val) { return apply([&val](auto &elem){ elem *= val; }); }
	Matrix& operator/=(const Type &val) { return apply([&val](auto &elem){ elem /= val; }); }
	Matrix& operator%=(const Type &val) { return apply([&val](auto &elem){ elem %= val; }); }

	iterator begin() noexcept { return elems.begin(); }
	iterator end() noexcept { return elems.end(); }
	const_iterator begin() const noexcept { return elems.cbegin(); }
	const_iterator end() const noexcept { return elems.cend(); }
	const_iterator cbegin() const noexcept { return elems.cbegin(); }
	const_iterator cend() const noexcept { return elems.cend(); }

	MatrixRef<Type, DimeReduction> row(std::size_t n) {
		return Base::rowImpl(*this, n);
	}
	ConstMatrixRef<Type, DimeReduction> row(std::size_t n) const {
		return Base::rowImpl(*this, n);
	}

	MatrixRef<Type, DimeReduction> col(std::size_t n) {
		return Base::colImpl(*this, n);
	}
	ConstMatrixRef<Type, DimeReduction> col(std::size_t n) const {
		return Base::colImpl(*this, n);
	}

	MatrixRef<Type, DimeReduction> operator[](std::size_t n) {
		return row(n);
	}
	ConstMatrixRef<Type, DimeReduction> operator[](std::size_t n) const {
		return row(n);
	}

	template<typename... Indexs>
	std::enable_if_t<IsAllIntegral<Indexs...>, value_type&> operator()(Indexs... indexes) {
		static_assert(sizeof...(Indexs) == Dime);
		return elems[slice(indexes...)];
	}
	template<typename... Indexs>
	std::enable_if_t<IsAllIntegral<Indexs...>, const value_type&> operator()(Indexs... indexes) const {
		static_assert(sizeof...(Indexs) == Dime);
		return elems[slice(indexes...)];
	}

	template<typename... Slices>
	std::enable_if_t<!IsAllIntegral<Slices...> && IsAllSlice<Slices...>, MatrixRef<Type, Dime>> operator()(const Slices&... slices) {
		static_assert(sizeof...(Slices) == Dime);
		return { MatrixSlice<Dime>({ Slice::createSlice(slices).start... },
								   { Slice::createSlice(slices).length... }, slice), elems };
	}
	template<typename... Slices>
	std::enable_if_t<!IsAllIntegral<Slices...> && IsAllSlice<Slices...>, ConstMatrixRef<Type, Dime>> operator()(const Slices&... slices) const {
		static_assert(sizeof...(Slices) == Dime);
		return { MatrixSlice<Dime>({ Slice::createSlice(slices).start... },
								   { Slice::createSlice(slices).length... }, slice), elems };
	}

private:
	MatrixSlice<Dime> slice;
	MatrixElements<Type> elems;
};

struct InsufficientCapacity : public std::runtime_error
{
	using std::runtime_error::runtime_error;
};

// Matrix引用
template<typename Type, std::size_t Dime>
class Matrix<std::reference_wrapper<Type>, Dime> : private MatrixBase<Type, Dime>
{
	template<typename T, std::size_t D> friend class Matrix;
	friend class MatrixBase<Type, Dime>;
	using Base = MatrixBase<Type, Dime>;

public:
	using Base::DimeReduction;
	using value_type = Type;
	using reference = value_type&;
	using const_reference = const value_type&;
	using size_type = std::size_t;
	using iterator = MatrixIterator<Type, Dime>;
	using const_iterator = ConstMatrixIterator<Type, Dime>;

	Matrix(const Matrix &) = default;
	Matrix(Matrix &&) = default;
	Matrix& operator=(Matrix &&) = delete;

	Matrix() = delete;
	Matrix& operator=(const Matrix &rhs)
	{
		if (rhs.size() > size())
			throw InsufficientCapacity("insufficient Capacity");
		std::copy(rhs.cbegin(), rhs.cend(), begin());
		return *this;
	}
	template<typename T>
	Matrix& operator=(const Matrix<std::reference_wrapper<T>, Dime> &rhs)
	{
		if (rhs.size() > size())
			throw InsufficientCapacity("insufficient Capacity");
		std::copy(rhs.cbegin(), rhs.cend(), begin());
		return *this;
	}
	
	template<typename T, typename = std::enable_if_t<!IsReferenceWrp<T>>>
	Matrix(Matrix<T, Dime> &rhs) : slice(rhs.slice), elems(rhs.elems.data() + slice.start) { }
	template<typename T>
	Matrix(const MatrixSlice<Dime> &sle, T *elms) :
		slice(sle), elems(elms) { }
	template<typename T>
	Matrix(const MatrixSlice<Dime> &sle, MatrixElements<T> &elms) :
		slice(sle), elems(elms.data()) { }

	std::size_t size() const noexcept { return slice.size; }
	std::size_t extent(std::size_t n = 0) const noexcept { return slice.extent(n); }
	constexpr static std::size_t dimensions() { return Dime; }
	std::size_t rows() const noexcept { return slice.rows(); }
	std::size_t cols() const noexcept { return slice.cols(); }

	template<typename F>
	Matrix& apply(F func) {
		for (Type &elem : *this)
			func(elem);
		return *this;
	}
	template<typename M, typename F>
	Matrix& apply(const M &mat, F func) {
		for (auto lhsBegin = begin(), rhsBegin = mat.cbegin(); lhsBegin != end(); ++lhsBegin, ++rhsBegin)
			func(*lhsBegin, *rhsBegin);
		return *this;
	}
	template<typename F>
	Matrix& apply(F func) const {
		for (Type &elem : *this)
			func(elem);
		return *this;
	}
	template<typename M, typename F>
	Matrix& apply(const M &mat, F func) const {
		for (auto lhsBegin = begin(), rhsBegin = mat.cbegin(); lhsBegin != end(); ++lhsBegin, ++rhsBegin)
			func(*lhsBegin, *rhsBegin);
		return *this;
	}

	Matrix& operator+=(const Type &val) { return apply([&val](auto &elem){ elem += val; }); }
	Matrix& operator-=(const Type &val) { return apply([&val](auto &elem){ elem -= val; }); }
	Matrix& operator*=(const Type &val) { return apply([&val](auto &elem){ elem *= val; }); }
	Matrix& operator/=(const Type &val) { return apply([&val](auto &elem){ elem /= val; }); }
	Matrix& operator%=(const Type &val) { return apply([&val](auto &elem){ elem %= val; }); }

	iterator begin() noexcept { return { &slice, elems }; }
	iterator end() noexcept { return { &slice, elems, elems + slice.size }; }
	const_iterator begin() const noexcept { return const_iterator{ &slice, elems }; }
	const_iterator end() const noexcept { return const_iterator{ &slice, elems, elems + slice.size }; }
	const_iterator cbegin() const noexcept { return { &slice, elems }; }
	const_iterator cend() const noexcept { return { &slice, elems, elems + slice.size }; }

	MatrixRef<Type, DimeReduction> row(std::size_t n) {
		return Base::rowImpl(*this, n);
	}
	ConstMatrixRef<Type, DimeReduction> row(std::size_t n) const {
		return Base::rowImpl(*this, n);
	}

	MatrixRef<Type, DimeReduction> col(std::size_t n) {
		return Base::colImpl(*this, n);
	}
	ConstMatrixRef<Type, DimeReduction> col(std::size_t n) const {
		return Base::colImpl(*this, n);
	}

	MatrixRef<Type, DimeReduction> operator[](std::size_t n) {
		return row(n);
	}
	ConstMatrixRef<Type, DimeReduction> operator[](std::size_t n) const {
		return row(n);
	}
	template<typename... Indexs>
	std::enable_if_t<IsAllIntegral<Indexs...>, value_type&> operator()(Indexs... indexes) {
		return elems[slice(indexes...)];
	}
	template<typename... Indexs>
	std::enable_if_t<IsAllIntegral<Indexs...>, const value_type&> operator()(Indexs... indexes) const {
		return elems[slice(indexes...)];
	}

	template<typename... Slices>
	std::enable_if_t<!IsAllIntegral<Slices...> && IsAllSlice<Slices...>, MatrixRef<Type, Dime>> operator()(const Slices&... slices) {
		static_assert(sizeof...(Slices) == Dime);
		return { MatrixSlice<Dime>({ Slice::createSlice(slices).start... },
								   { Slice::createSlice(slices).length... }, slice), elems };
	}
	template<typename... Slices>
	std::enable_if_t<!IsAllIntegral<Slices...> && IsAllSlice<Slices...>, ConstMatrixRef<Type, Dime>> operator()(const Slices&... slices) const {
		static_assert(sizeof...(Slices) == Dime);
		return { MatrixSlice<Dime>({ Slice::createSlice(slices).start... },
								   { Slice::createSlice(slices).length... }, slice), elems };
	}

private:
	const MatrixSlice<Dime> slice;
	Type *const elems;
};

// Const Matrix引用
template<typename Type, std::size_t Dime>
class Matrix<std::reference_wrapper<const Type>, Dime> : private MatrixBase<Type, Dime>
{
	template<typename T, std::size_t D> friend class Matrix;
	friend class MatrixBase<Type, Dime>;
	using Base = MatrixBase<Type, Dime>;

public:
	using Base::DimeReduction;
	using value_type = Type;
	using reference = value_type&;
	using const_reference = const value_type&;
	using size_type = std::size_t;
	using iterator = MatrixIterator<Type, Dime>;
	using const_iterator = ConstMatrixIterator<Type, Dime>;

	Matrix(const Matrix &rhs) : slice(rhs.slice), elems(rhs.elems.data()) { }
	Matrix(Matrix &&) = default;

	Matrix() = delete;
	Matrix& operator=(const Matrix&) = delete;
	Matrix& operator=(Matrix &&) = delete;

	template<typename T>
	Matrix(const MatrixRef<T, Dime> &rhs) : slice(rhs.slice), elems(rhs.elems) { }
	template<typename T>
	Matrix(const Matrix<T, Dime> &rhs) : slice(rhs.slice), elems(rhs.elems.data()) { }

	template<typename T>
	Matrix(const MatrixSlice<Dime> &sle, const T *elms) : 
		slice(sle), elems(elms) { }
	template<typename T>
	Matrix(const MatrixSlice<Dime> &sle, const MatrixElements<T> &elms) :
		slice(sle), elems(elms.data()) { }

	std::size_t size() const noexcept { return slice.size; }
	std::size_t extent(std::size_t n = 0) const noexcept { return slice.extent(n); }
	constexpr static std::size_t dimensions() { return Dime; }
	std::size_t rows() const noexcept { return slice.rows(); }
	std::size_t cols() const noexcept { return slice.cols(); }

	template<typename F>
	Matrix& apply(F func) const {
		for (Type &elem : *this)
			func(elem);
		return *this;
	}
	template<typename M, typename F>
	Matrix& apply(const M &mat, F func) const {
		for (auto lhsBegin = cbegin(), rhsBegin = mat.cbegin(); lhsBegin != cend(); ++lhsBegin, ++rhsBegin)
			func(*lhsBegin, *rhsBegin);
		return *this;
	}

	const_iterator begin() const noexcept { return cbegin(); }
	const_iterator end() const noexcept { return cend(); }
	const_iterator cbegin() const noexcept { return { &slice, elems }; }
	const_iterator cend() const noexcept { return { &slice, elems, elems + slice.size }; }

	ConstMatrixRef<Type, DimeReduction> row(std::size_t n) const {
		return Base::rowImpl(*this, n);
	}

	ConstMatrixRef<Type, DimeReduction> col(std::size_t n) const {
		return Base::colImpl(*this, n);
	}

	ConstMatrixRef<Type, DimeReduction> operator[](std::size_t n) const {
		return row(n);
	}

	template<typename... Indexs>
	std::enable_if_t<IsAllIntegral<Indexs...>, const value_type&> operator()(Indexs... indexes) const {
		return elems[slice(indexes...)];
	}

	template<typename... Slices>
	std::enable_if_t<!IsAllIntegral<Slices...> && IsAllSlice<Slices...>, ConstMatrixRef<Type, Dime>> operator()(const Slices&... slices) const {
		static_assert(sizeof...(Slices) == Dime);
		return { MatrixSlice<Dime>({ Slice::createSlice(slices).start... },
								   { Slice::createSlice(slices).length... }, slice), elems };
	}

private:
	const MatrixSlice<Dime> slice;
	const Type *const elems;
};

template<typename Type>
struct Matrix<std::reference_wrapper<Type>, 0> : public std::reference_wrapper<Type>
{
public:
	using reference = std::reference_wrapper<Type>;
	using reference::reference;

	template<typename T>
	Type& operator=(T &&val) { return static_cast<Type&>(*this) = std::forward<T>(val); }
};

template<typename Type>
struct Matrix<std::reference_wrapper<const Type>, 0> : public std::reference_wrapper<const Type>
{
	using reference = std::reference_wrapper<const Type>;
	using reference::reference;

	template<typename T>
	Type& operator=(T &&val) { return static_cast<Type&>(*this) = std::forward<T>(val); }
};

template<typename Type>
class Matrix<Type, 0>;

template<typename Type>
using _1Dlist = InitList<Type>;
template<typename Type>
using _2Dlist = InitList<InitList<Type>>;
template<typename Type>
using _3Dlist = InitList<InitList<InitList<Type>>>;
template<typename Type>
using _4Dlist = InitList<InitList<InitList<InitList<Type>>>>;
template<typename Type>
using _5Dlist = InitList<InitList<InitList<InitList<InitList<Type>>>>>;

template<typename Type, std::size_t Dime>
Matrix(const MatrixParser<Type, Dime> &init) -> Matrix<Type, Dime>;
template<typename Type, typename... Exts>
Matrix(const Type&, Exts...) -> Matrix<Type, sizeof...(Exts)>;
template<typename Type>
Matrix(const _1Dlist<Type>&) -> Matrix<Type, 1>;
template<typename Type>
Matrix(const _2Dlist<Type>&) -> Matrix<Type, 2>;
template<typename Type>
Matrix(const _3Dlist<Type>&) -> Matrix<Type, 3>;
template<typename Type>
Matrix(const _4Dlist<Type>&) -> Matrix<Type, 4>;
template<typename Type>
Matrix(const _5Dlist<Type>&) -> Matrix<Type, 5>;

template<std::size_t Dime, std::size_t MaxDime = Dime, typename Type>
std::enable_if_t<0 < Dime, std::ostream&> operator<<(std::ostream &os, const Matrix<Type, Dime> &mat)
{
	constexpr bool flag = Dime != 1;
	std::ostream::sentry test(os);
	if (!test)
	{
		os.setstate(std::ios_base::failbit);
		return os;
	}

	os << std::string((MaxDime - Dime) * 2, ' ') << '{';
	if constexpr (flag)
		os << '\n';
	else os << ' ';

	for (std::size_t i = 0, sz = mat.rows(); i < sz; ++i)
	{
		if constexpr (flag)
			operator<< <Dime - 1, MaxDime>(os, mat.row(i));
		else os << mat[i].get();
		if (i != sz - 1)
			os << ", ";
		if constexpr (flag)
			os << '\n';
	}

	if constexpr (flag)
		os << std::string((MaxDime - Dime) * 2, ' '); 
	else os << ' ';
	return os << '}';
}
}

namespace mystd::matrix {
template<typename T>
std::enable_if_t<IsMatrixIter<T>::value, bool> operator==(const T &lhs, const T &rhs) {
	return lhs.get() == rhs.get();
}
template<typename T>
std::enable_if_t<IsMatrixIter<T>::value, bool> operator!=(const T &lhs, const T &rhs) {
	return lhs.get() != rhs.get();
}
template<typename T>
std::enable_if_t<IsMatrixIter<T>::value, bool> operator>=(const T &lhs, const T &rhs) {
	return lhs.get() >= rhs.get();
}
template<typename T>
std::enable_if_t<IsMatrixIter<T>::value, bool> operator<=(const T &lhs, const T &rhs) {
	return lhs.get() <= rhs.get();
}
template<typename T>
std::enable_if_t<IsMatrixIter<T>::value, bool> operator>(const T &lhs, const T &rhs) {
	return lhs.get() > rhs.get();
}
template<typename T>
std::enable_if_t<IsMatrixIter<T>::value, bool> operator<(const T &lhs, const T &rhs) {
	return lhs.get() < rhs.get();
}
}

using namespace mystd::matrix;

int main()
{
	Matrix<double, 4> m {
		{
			{
				{1, 2, 3},
				{4, 5, 6},
				{7, 8, 9}
			},
			{
				{10, 11, 12},
				{13, 14, 15},
				{16, 17, 18},
				{16, 17, 18, 10, 300, 10}
			}
		},
		{
			{
				{19, 20, 21},
				{22, 23, 24},
				{25, 26, 27},
			},
			{
				{28, 29, 30},
				{31, 32, 33},
				{34, 35, 36}
			}
		}
	};
	std::cout << m << std::endl;
}
