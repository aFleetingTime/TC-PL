#include <array>
#include <string>
#include <vector>
#include <numeric>
#include <iostream>
#include <algorithm>
#include <functional>
#include <type_traits>

// 13增：slice切片、输出运算格式化。
// 14日新增：矩阵运算、col函数

namespace mystd::matrix::impl {
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

	template<typename Type, std::size_t Dime>
	struct MatrixInitList {
		using init_list_type = std::initializer_list<typename MatrixInitList<Type, Dime - 1>::init_list_type>;
		static constexpr inline std::size_t dime = Dime;
	};
	template<typename Type>
	struct MatrixInitList<Type, 1> {
		using init_list_type = std::initializer_list<Type>;
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
	std::enable_if_t<(Dime == 1), std::size_t> addExtents(OutputIt iter, const typename MatrixInitList<Type, Dime>::init_list_type &init)
	{
		return init.size();
	}
	template<typename Type, std::size_t Dime, typename OutputIt>
	std::enable_if_t<(Dime > 1), std::size_t> addExtents(OutputIt iter, const typename MatrixInitList<Type, Dime>::init_list_type &init)
	{
		std::size_t max = 0;
		for (auto beg = init.begin(); beg != init.end(); ++beg)
			if (std::size_t ret = addExtents<Type, Dime - 1>(iter + 1, *beg); max < ret)
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


template<typename Type, std::size_t Dime>
using MatrixInitList = typename impl::MatrixInitList<Type, Dime>::init_list_type;

template<typename T>
constexpr bool IsReferenceWrp = impl::IsReferenceWrp<T>::value;

class Slice;
template<typename Type, std::size_t Dime>
class Matrix;
template<typename Type, std::size_t Dime>
class MatrixRef;
template<typename Type, std::size_t Dime>
class ConstMatrixRef;

template<typename T>
struct IsSliceOrIntegral : std::bool_constant<std::is_same_v<T, Slice> || std::is_integral_v<T>> { };
template<typename First, typename... Tail>
constexpr bool IsAllSlice = impl::TemplateIsAll<IsSliceOrIntegral, First, Tail...>::value;
template<typename First, typename... Tail>
constexpr bool IsAllIntegral = impl::TemplateIsAll<std::is_integral, First, Tail...>::value;

template<typename T>
struct IsMatrix : std::false_type { };
template<typename T, std::size_t D>
struct IsMatrix<Matrix<T, D>> : std::true_type { };
template<typename T, std::size_t D>
struct IsMatrix<MatrixRef<T, D>> : std::true_type { };
template<typename T, std::size_t D>
struct IsMatrix<ConstMatrixRef<T, D>> : std::true_type { };

// Slice切片访问
struct Slice
{
	static inline constexpr std::size_t npos = std::numeric_limits<std::size_t>::max();
	constexpr explicit Slice(std::size_t s = 0, std::size_t len = npos, std::size_t stri = 1) : start(s), length(len), stride(stri) { }

	constexpr std::size_t operator()(std::size_t n) const noexcept { return stride * n + start; }

	std::size_t start, length, stride;

	static constexpr Slice createSlice(std::size_t start) {
		return Slice(start, 1);
	}
	static constexpr const Slice& createSlice(const Slice &slice) {
		return slice;
	}
};

// MatrixInitList解析器
template<typename Type, std::size_t Dime>
class MatrixParser
{
	static_assert(Dime != 0, "there is no 0 dimension matrix");
public:
	using init_list = MatrixInitList<Type, Dime>;

	MatrixParser(const init_list &init, const std::array<std::size_t, Dime> &exts) : extents(exts), dimeMaxSize(minDimeMaxSize<Dime>(init))
	{
		list.resize(std::accumulate(extents.cbegin(), extents.cend(), std::size_t(1), std::multiplies<std::size_t>()));
		parser<Dime>(init, list.begin());
		list.shrink_to_fit();
	}

	std::vector<Type>&& parser() noexcept {
		return std::move(list);
	}

	constexpr std::size_t dime() const noexcept {
		return Dime;
	}

	void copy(std::vector<Type> &dest) const {
		dest = list;
	}

private:
	template<std::size_t N, typename T>
	std::enable_if_t<N == 1> parser(const MatrixInitList<Type, N> &init, T &&it)
	{
		it = std::copy(init.begin(), init.end(), it);
		if (init.size() < extents.back())
			it += extents.back() - init.size();
	}
	template<std::size_t N, typename T>
	std::enable_if_t<(N > 1)> parser(const MatrixInitList<Type, N> &init, T &&it)
	{
		for (auto beg = init.begin(); beg != init.end(); ++beg)
			parser<N - 1>(*beg, it);
		std::size_t temp = std::accumulate(extents.cbegin() + (Dime - N + 1), extents.cend(), std::size_t(1), std::multiplies<std::size_t>());
		if (init.size() < extents[Dime - N])
			it += (temp / init.size());
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

	std::vector<Type> list;
	const std::array<std::size_t, Dime> extents;
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
	using init_list = std::initializer_list<std::size_t>;
	using array = std::array<std::size_t, Dime>;

	MatrixSlice(std::size_t s, const array &exts)
		: extents(exts), size(initSize()), start(s) { setStride(); }
	MatrixSlice(std::size_t s, const array &exts, const array &strs)
		: extents(exts), strides(strs), size(initSize()), start(s) { }

	MatrixSlice(const array &stas, const array &exts, const MatrixSlice &other) : strides(other.strides), start(other.start)
	{
		initExtsStrs(stas, exts, other.extents, other.strides);
		if (checkArgs(stas, extents, other.extents))
			throw std::out_of_range("MatrixSlice stas exts: out of range");
		size = initSize();
	}

	template<typename... Exts, typename = std::enable_if_t<IsAllIntegral<Exts...>>>
	explicit MatrixSlice(Exts... exts)  : extents{exts...}, size(initSize()), start(0) { setStride(); }

	template<typename... Dimes, typename = std::enable_if_t<IsAllIntegral<Dimes...>>>
	std::size_t operator()(Dimes... dimes) const
	{
		static_assert(sizeof...(Dimes) == Dime, "dimensions do not match");
		std::array<std::size_t, Dime> temp { std::size_t(dimes)... };
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

	void setStride() {
		std::size_t sum = size;
		for (auto beg = extents.cbegin(), dest = strides.begin(); beg != extents.cend(); ++beg)
			*dest++ = sum /= *beg;
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
			if (oldExts[i] < (stas[i] + exts[i]))
				return true;
		return false;
	}

	array extents;
	array strides;
	std::size_t size;
	std::size_t start;
};

// Matrix元素容器
template<typename Type>
class MatrixElements : public std::vector<Type>
{
	template<typename T> friend class MatrixElements;
public:
	using Base = std::vector<Type>;
	using Base::Base;
	using value_type = Type;

	template<std::size_t Dime>
	MatrixElements(MatrixParser<Type, Dime> &&init) : Base(init.parser()) { }
};

// Matrix矩阵公共接口
template<typename Type, std::size_t Dime>
class MatrixBase
{
public:
	static inline constexpr std::size_t DimeReduction = Dime - 1;
	static inline constexpr std::size_t newDime = DimeReduction ? DimeReduction : 1;
	using array = std::array<std::size_t, newDime>;

protected:
	static inline constexpr std::size_t offset = newDime != 1;

	template<typename T>
	MatrixRef<Type, DimeReduction> rowImpl(T &mat, std::size_t n)
	{
		if (n >= mat.slice.rows())
			throw std::out_of_range("row: out of range");
		array extents;
		array strides;
		std::copy(mat.slice.extents.cbegin() + offset, mat.slice.extents.cend(), extents.begin());
		std::copy(mat.slice.strides.cbegin() + offset, mat.slice.strides.cend(), strides.begin());
		return { MatrixSlice<newDime>(mat.slice.start + (n * mat.slice.strides.front()), extents, strides), mat.elems };
	}
	template<typename T>
	MatrixRef<Type, DimeReduction> colImpl(T &mat, std::size_t n)
	{
		if (n >= mat.slice.cols())
			throw std::out_of_range("col: out of range");
		array extents;
		array strides;
		std::copy(mat.slice.extents.cbegin(), mat.slice.extents.cend() - offset, extents.begin());
		std::copy(mat.slice.strides.cbegin(), mat.slice.strides.cend() - offset, strides.begin());
		return { MatrixSlice<newDime>(mat.slice.start + n, extents, strides), mat.elems };
	}
	template<typename T>
	ConstMatrixRef<Type, DimeReduction> rowImpl(const T &mat, std::size_t n) const
	{
		if (n >= mat.slice.rows())
			throw std::out_of_range("row: out of range");
		array extents;
		array strides;
		std::copy(mat.slice.extents.cbegin() + offset, mat.slice.extents.cend(), extents.begin());
		std::copy(mat.slice.strides.cbegin() + offset, mat.slice.strides.cend(), strides.begin());
		return { MatrixSlice<newDime>(mat.slice.start + (n * mat.slice.strides.front()), extents, strides), mat.elems };
	}
	template<typename T>
	ConstMatrixRef<Type, DimeReduction> colImpl(const T &mat, std::size_t n) const
	{
		if (n >= mat.slice.cols())
			throw std::out_of_range("col: out of range");
		array extents;
		array strides;
		std::copy(mat.slice.extents.cbegin(), mat.slice.extents.cend() - offset, extents.begin());
		std::copy(mat.slice.strides.cbegin(), mat.slice.strides.cend() - offset, strides.begin());
		return { MatrixSlice<newDime>(mat.slice.start + n, extents, strides), mat.elems };
	}
};

// Const Matrix引用
template<typename Type, std::size_t Dime>
class ConstMatrixRef : MatrixBase<Type, Dime>
{
	//template<typename T, std::size_t D> friend class Matrix;
	friend class MatrixBase<Type, Dime>;
	using Base = MatrixBase<Type, Dime>;
public:
	using Base::DimeReduction;
	using value_type = Type;
	using size_type = std::size_t;

	ConstMatrixRef(const ConstMatrixRef &) = default;

	//ConstMatrixRef(ConstMatrixRef &&) = delete;
	//ConstMatrixRef& operator=(const ConstMatrixRef &) = delete;
	//ConstMatrixRef& operator=(ConstMatrixRef &&) = delete;

	template<typename T>
	ConstMatrixRef(const Matrix<T, Dime> &matrix) : slice(matrix.slice), elems(matrix.elems) { }
	ConstMatrixRef(const MatrixSlice<Dime> &sle, const MatrixElements<Type> &elms) : slice(sle), elems(elms) { }

	std::size_t size() const noexcept { return slice.size; }
	std::size_t extent(std::size_t n = 0) const noexcept { return slice.extent(n); }
	constexpr static std::size_t dimensions() { return Dime; }
	std::size_t rows() const noexcept { return slice.rows(); }
	std::size_t cols() const noexcept { return slice.cols(); }

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
	std::enable_if_t<IsAllIntegral<Indexs...>, const value_type&> operator()(Indexs... indexs) const {
		return elems[slice(indexs...)];
	}
	template<typename... Slices>
	std::enable_if_t<!IsAllIntegral<Slices...> && IsAllSlice<Slices...>, ConstMatrixRef<Type, Dime>> operator()(const Slices&... slices) const {
		static_assert(sizeof...(Slices) == Dime);
		return { MatrixSlice<Dime>({ Slice::createSlice(slices).start... }, { Slice::createSlice(slices).length... }, slice), elems };
	}

private:
	const MatrixSlice<Dime> slice;
	const MatrixElements<Type> &elems;
};

// Matrix引用
template<typename Type, std::size_t Dime>
class MatrixRef : MatrixBase<Type, Dime>
{
	template<typename T, std::size_t D> friend class Matrix;
	friend class MatrixBase<Type, Dime>;
	using Base = MatrixBase<Type, Dime>;
public:
	using Base::DimeReduction;
	using value_type = Type;
	using size_type = std::size_t;

	MatrixRef(const MatrixRef &) = default;

	MatrixRef(MatrixRef &&) = delete;
	MatrixRef& operator=(const MatrixRef &) = delete;
	MatrixRef& operator=(MatrixRef &&) = delete;

	template<typename T>
	MatrixRef(Matrix<T, Dime> &matrix) : slice(matrix.slice), elems(matrix.elems) { }
	MatrixRef(const MatrixSlice<Dime> &sle, MatrixElements<Type> &elms) : slice(sle), elems(elms) { }

	std::size_t size() const noexcept { return slice.size; }
	std::size_t extent(std::size_t n = 0) const noexcept { return slice.extent(n); }
	constexpr static std::size_t dimensions() { return Dime; }
	std::size_t rows() const noexcept { return slice.rows(); }
	std::size_t cols() const noexcept { return slice.cols(); }

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
	std::conditional_t<Dime == 1, const value_type&, ConstMatrixRef<Type, DimeReduction>> operator[](std::size_t n) const {
		return row(n);
	}
	template<typename... Indexs>
	std::enable_if_t<IsAllIntegral<Indexs...>, value_type&> operator()(Indexs... indexs) {
		return elems[slice(indexs...)];
	}
	template<typename... Indexs>
	std::enable_if_t<IsAllIntegral<Indexs...>, const value_type&> operator()(Indexs... indexs) const {
		return elems[slice(indexs...)];
	}

	template<typename... Slices>
	std::enable_if_t<!IsAllIntegral<Slices...> && IsAllSlice<Slices...>, MatrixRef<Type, Dime>> operator()(const Slices&... slices) {
		static_assert(sizeof...(Slices) == Dime);
		return { MatrixSlice<Dime>({ Slice::createSlice(slices).start... }, { Slice::createSlice(slices).length... }, slice), elems };
	}
	template<typename... Slices>
	std::enable_if_t<!IsAllIntegral<Slices...> && IsAllSlice<Slices...>, ConstMatrixRef<Type, Dime>> operator()(const Slices&... slices) const {
		static_assert(sizeof...(Slices) == Dime);
		return { MatrixSlice<Dime>({ Slice::createSlice(slices).start... }, { Slice::createSlice(slices).length... }, slice), elems };
	}

private:
	MatrixSlice<Dime> slice;
	MatrixElements<Type> &elems;
};
template<typename Type>
class MatrixRef<Type, 0>
{
public:
	using type = Type;
	using reference = Type&;

	template<typename T>
	MatrixRef(T &&v) : p(&v) { }
	MatrixRef(const MatrixSlice<1> &sle, MatrixElements<Type> &elm) : p(elm.data() + sle(0)) { }
	MatrixRef(std::size_t s, MatrixElements<Type> &elm) : p(elm.data() + s) { }

	operator reference() const noexcept { return *p; }
	reference get() const noexcept { return *p; }

private:
	Type *p;
};
template<typename Type>
class ConstMatrixRef<Type, 0>
{
public:
	using type = Type;
	using const_reference = const Type&;

	template<typename T>
	ConstMatrixRef(const T &v) : p(&v) { }
	ConstMatrixRef(const MatrixSlice<1> &sle, const MatrixElements<Type> &elm) : p(elm.data() + sle(0)) { }
	ConstMatrixRef(std::size_t s, const MatrixElements<Type> &elm) : p(elm.data() + s) { }

	operator const_reference() const noexcept { return *p; }
	const_reference get() const noexcept { return *p; }	

private:
	const Type *p;
};


// Matrix矩阵
template<typename Type, std::size_t Dime>
class Matrix : MatrixBase<Type, Dime>
{
	template<typename T, std::size_t D> friend class Matrix;
	friend class MatrixBase<Type, Dime>;
	friend class MatrixRef<Type, Dime>;
	friend class ConstMatrixRef<Type, Dime>;
	using Base = MatrixBase<Type, Dime>;
public:
	using Base::DimeReduction;
	using value_type = Type;
	using size_type = std::size_t;

	Matrix(const Matrix &) = default;
	Matrix& operator=(const Matrix &) = default;
	Matrix(Matrix &&) = default;
	Matrix& operator=(Matrix &&) = default;

	template<typename... Exts, typename = std::enable_if_t<sizeof...(Exts) == Dime && IsAllIntegral<Exts...>>>
	Matrix(Exts... exts) : slice(std::size_t(exts)...), elems(slice.size) { }

	explicit Matrix(const MatrixInitList<Type, Dime> &init) : slice(0, impl::deriveExtents<Type, Dime>(init)), elems(MatrixParser<Type, Dime>(init, slice.getExtents())) { }

	template<typename T>
	Matrix(const Matrix<T, Dime> &rhs) : slice(rhs.slice), elems(rhs.elems.data(), rhs.elems.data() + rhs.elems.size()) { }

	std::size_t size() const noexcept { return slice.size; }
	std::size_t extent(std::size_t n = 0) const noexcept { return slice.extent(n); }
	constexpr static std::size_t dimensions() { return Dime; }
	std::size_t rows() const noexcept { return slice.rows(); }
	std::size_t cols() const noexcept { return slice.cols(); }

	MatrixRef<Type, DimeReduction> row(std::size_t n) {
		return Base::rowImpl(*this, n);
	}
	ConstMatrixRef<Type, DimeReduction> row(std::size_t n) const {
		if constexpr (Dime == 1)
			return elems[slice(n)];
		else return Base::rowImpl(*this, n);
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
	std::enable_if_t<IsAllIntegral<Indexs...>, value_type&> operator()(Indexs... indexs) {
		static_assert(sizeof...(Indexs) == Dime);
		return elems[slice(indexs...)];
	}
	template<typename... Indexs>
	std::enable_if_t<IsAllIntegral<Indexs...>, const value_type&> operator()(Indexs... indexs) const {
		static_assert(sizeof...(Indexs) == Dime);
		return elems[slice(indexs...)];
	}

	template<typename... Slices>
	std::enable_if_t<!IsAllIntegral<Slices...> && IsAllSlice<Slices...>, MatrixRef<Type, Dime>> operator()(const Slices&... slices) {
		static_assert(sizeof...(Slices) == Dime);
		return { MatrixSlice<Dime>({ Slice::createSlice(slices).start... }, { Slice::createSlice(slices).length... }, slice), elems };
	}
	template<typename... Slices>
	std::enable_if_t<!IsAllIntegral<Slices...> && IsAllSlice<Slices...>, ConstMatrixRef<Type, Dime>> operator()(const Slices&... slices) const {
		static_assert(sizeof...(Slices) == Dime);
		return { MatrixSlice<Dime>({ Slice::createSlice(slices).start... }, { Slice::createSlice(slices).length... }, slice), elems };
	}

private:
	MatrixSlice<Dime> slice;
	MatrixElements<Type> elems;
};

template<typename Type>
class Matrix<Type, 0>;

template<std::size_t Dime = 0, typename Type>
std::enable_if_t<IsMatrix<Type>::value, std::ostream&> operator<<(std::ostream &os, const Type &mat)
{
	constexpr bool flag = Type::dimensions() != 1;
	os << std::string(Dime * 2, ' ') << '{';
	if constexpr (flag)
		os << '\n';
	else os << ' ';
	for (std::size_t i = 0, sz = mat.rows(); i < sz; ++i)
	{
		if constexpr (flag)
			operator<<<Dime + 1>(os, mat[i]);
		else os << mat[i].get();
		if (i != sz - 1)
			os << ", ";
		if constexpr (flag)
			os << '\n';
	}
	if constexpr (flag)
		os << std::string(Dime * 2, ' '); 
	else os << ' ';
	return os << '}';
}
}

using namespace mystd::matrix;

int main()
{
	const Matrix<double, 4> m {
		{
			{
				{1, 2, 3},
				{4, 5, 6},
				{7, 8, 9}
			},
			{
				{10, 11, 12},
				{13, 14, 15},
				{16, 17, 18}
			}
		},
		{
			{
				{19, 20, 21},
				{22, 23, 24},
				{25, 26, 27}
				//{222, 223, 224, 225}
			},
			{
				{28, 29, 30},
				{31, 32, 33},
				{34, 35, 36}
			}
		}
	};
	//int &r = 
	//m.col(0);//.col(0).col(0);//.col(0);//[0][0] = 10000;
	//std::cout << m[0][0][0][0];
	//std::cout << m << std::endl;
	//std::cout << m(1,1,0,2) << std::endl;
	std::cout << m[1] << std::endl;
}
