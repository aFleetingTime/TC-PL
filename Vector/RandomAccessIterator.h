#pragma once
#include <iterator>

template<typename T>
class RandomAccessIterator
{
public:
	using iterator_type = T;
	using iterator_category = typename std::iterator_traits<T>::iterator_category;
	using value_type = typename std::iterator_traits<T>::value_type;
	using difference_type = typename std::iterator_traits<T>::difference_type;
	using pointer = typename std::iterator_traits<T>::pointer;
	using reference = typename std::iterator_traits<T>::reference;
	using const_reference = const typename std::iterator_traits<T>::reference;

	constexpr RandomAccessIterator() = default;
	constexpr explicit RandomAccessIterator(T iter) : current(iter) { }
	constexpr RandomAccessIterator(const RandomAccessIterator &iter) : current(iter.current) { }
	template<typename U>
	constexpr RandomAccessIterator(const RandomAccessIterator<U*> &iter) : current(iter.current) { }

	constexpr T base() const { return current; }

	constexpr RandomAccessIterator& operator=(const RandomAccessIterator &rhs) { current = rhs.current; return *this; }

	constexpr RandomAccessIterator& operator++()
	{
		++current;
		return *this;
	}
	constexpr RandomAccessIterator operator++(int) { return RandomAccessIterator(current++); }

	constexpr RandomAccessIterator& operator--()
	{
		--current;
		return *this;
	}
	constexpr RandomAccessIterator operator--(int) { return RandomAccessIterator(current--); }

	constexpr RandomAccessIterator& operator-=(difference_type n)
	{
		current -= n;
		return *this;
	}
	constexpr RandomAccessIterator& operator+=(difference_type n)
	{
		current += n;
		return *this; 
	}

	constexpr reference operator*() { return *current; }
	constexpr const T& operator*() const { return *current; }
	constexpr RandomAccessIterator& operator[](difference_type n) { return current[n]; }

private:
	T current;
};

template<typename It>
constexpr RandomAccessIterator<It> operator-(typename RandomAccessIterator<It>::difference_type n, const RandomAccessIterator<It> &it)
{ return RandomAccessIterator<It>(it) -= n; }
template<typename It>
constexpr RandomAccessIterator<It> operator+(typename RandomAccessIterator<It>::difference_type n, const RandomAccessIterator<It> &it)
{ return RandomAccessIterator<It>(it) += n; }
template<typename It>
constexpr RandomAccessIterator<It> operator-(const RandomAccessIterator<It> &it, typename RandomAccessIterator<It>::difference_type n)
{ return RandomAccessIterator<It>(it) -= n; }
template<typename It>
constexpr RandomAccessIterator<It> operator+(const RandomAccessIterator<It> &it, typename RandomAccessIterator<It>::difference_type n)
{ return RandomAccessIterator<It>(it) += n; }
template<typename It>
constexpr typename RandomAccessIterator<It>::difference_type operator-(const RandomAccessIterator<It> &lhs, const RandomAccessIterator<It> rhs)
{ return lhs.base() - rhs.base(); }
template<typename It>
constexpr typename RandomAccessIterator<It>::difference_type operator+(const RandomAccessIterator<It> &lhs, const RandomAccessIterator<It> rhs)
{ return lhs.base() + rhs.base(); }


template<typename It1, typename It2>
constexpr bool operator<(const RandomAccessIterator<It1> &lhs, const RandomAccessIterator<It2> &rhs)
{
	return lhs.base() < rhs.base();
}

template<typename It1, typename It2>
constexpr bool operator>(const RandomAccessIterator<It1> &lhs, const RandomAccessIterator<It2> &rhs)
{
	return lhs.base() > rhs.base();
}

template<typename It1, typename It2>
constexpr bool operator<=(const RandomAccessIterator<It1> &lhs, const RandomAccessIterator<It2> &rhs)
{
	return lhs.base() <= rhs.base();
}

template<typename It1, typename It2>
constexpr bool operator>=(const RandomAccessIterator<It1> &lhs, const RandomAccessIterator<It2> &rhs)
{
	return lhs.base() >= rhs.base();
}

template<typename It1, typename It2>
constexpr bool operator==(const RandomAccessIterator<It1> &lhs, const RandomAccessIterator<It2> &rhs)
{
	return lhs.base() == rhs.base();
}

template<typename It1, typename It2>
constexpr bool operator!=(const RandomAccessIterator<It1> &lhs, const RandomAccessIterator<It2> &rhs)
{
	return lhs.base() != rhs.base();
}

#if 0
template<typename T>
RandomAccessIterator(typename T::pointer) -> RandomAccessIterator<typename T::pointer>;
#endif
