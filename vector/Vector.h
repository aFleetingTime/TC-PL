#pragma once
#include <memory>
#include <stdexcept>
#include <functional>
#include <type_traits>
#include <iterator>
#include <iostream>
#include "RandomAccessIterator.h"

template<typename T, typename A = std::allocator<T>>//typename std::remove_reference<T>::type>>
class Vector
{
public:	
	using size_type = std::size_t;
	using value_type = typename std::remove_reference<T>::type;
	using reference = value_type&;
	using const_reference = const value_type&;
	using allocator_type = A;
	using difference_type = std::ptrdiff_t;
	using pointer = typename std::allocator_traits<A>::pointer;
	using const_pointer = typename std::allocator_traits<A>::const_pointer;
	using iterator = RandomAccessIterator<pointer>;
	using const_iterator = const RandomAccessIterator<pointer>;

	Vector() = default;
	~Vector()
	{
		std::destroy(elem, space);
		alloc.deallocate(elem, last - elem);
	}

	Vector(Vector &&rhs) noexcept : alloc(std::move(rhs.alloc)), elem(std::exchange(rhs.elem, nullptr)),
									space(std::exchange(rhs.space, nullptr)), last(std::exchange(rhs.last, nullptr)) { }

	Vector(const Vector &rhs) : alloc(rhs.alloc)
	{
		elem = alloc.allocate(rhs.space - rhs.elem);
		initMemory(rhs.size(), static_cast<pointer(*)(pointer, pointer, pointer)>(std::uninitialized_copy), rhs.elem, rhs.space, elem);
	}

	template<typename InputIt>
	Vector(std::enable_if_t<std::is_base_of_v<std::input_iterator_tag, typename std::iterator_traits<InputIt>::iterator_category>, InputIt> fst, InputIt lst, const allocator_type &a = allocator_type()) : alloc(a)
	{
		std::size_t diff = 0;
		if constexpr (std::is_same_v<typename std::iterator_traits<InputIt>::iterator_category, std::bidirectional_iterator_tag>)
			for (auto tempIt = fst; tempIt != lst; ++diff, ++tempIt);
		else
			diff = lst - fst;
		elem = alloc.allocate(diff);
		initMemory(diff, static_cast<pointer(*)(InputIt, InputIt, pointer)>(std::uninitialized_copy), fst, lst, elem);
	}

	Vector(std::initializer_list<value_type> init, const allocator_type &a = allocator_type()) : alloc(a)
	{
		elem = alloc.allocate(init.size());
		using InitType = std::initializer_list<value_type>;
		initMemory(init.size(), static_cast<pointer(*)(typename InitType::iterator, typename InitType::iterator, pointer)>(std::uninitialized_copy), init.begin(), init.end(), elem);
	}
	explicit Vector(size_type n, const allocator_type &a = allocator_type()) : alloc(a)
	{
		elem = alloc.allocate(n);
		initMemory(n, static_cast<pointer(*)(pointer, size_type)>(std::uninitialized_value_construct_n), elem, n);
	}
	Vector(size_type n, const_reference value, const allocator_type &a = allocator_type()) : alloc(a)
	{
		elem = alloc.allocate(n);
		initMemory(n, static_cast<pointer(*)(pointer, size_type, const value_type&)>(std::uninitialized_fill_n), elem, n, value);
	}

	void swap(Vector &rhs) noexcept
	{
		std::swap(elem, rhs.elem);
		std::swap(last, rhs.last);
		std::swap(space, rhs.space);
		std::swap(alloc, rhs.alloc);
	}

	Vector& operator=(Vector &&rhs) noexcept
	{
		Vector temp(std::move(rhs));
		this->swap(temp);
		return *this;
	}

	Vector& operator=(const Vector &rhs)
	{
		if(capacity() < rhs.size())
		{
			Vector temp(rhs.cbegin(), rhs.cend());
			this->swap(temp);
			return *this;
		}
		size_type minSize = std::min(size(), rhs.size());
		std::uninitialized_copy
				(rhs.cbegin() + minSize, rhs.cend(), std::copy_n(rhs.cbegin(), minSize, begin())).base();
		if(minSize == rhs.size())
			std::destroy(elem + minSize, space);
		space = elem + rhs.size();
		return *this;
	}

	reference operator[](size_type n) { return elem[n]; }
	const_reference operator[](size_type n) const { return elem[n]; }
	reference at(size_type n) { return (n < size()) ? elem[n] : throw std::out_of_range("out of range"); }
	const_reference at(size_type n) const { return (n < size()) ? elem[n] : throw std::out_of_range("out of range"); }
	const_pointer data() const noexcept { return elem; }
	allocator_type get_allocator() const { return alloc; }

	size_type size() const noexcept { return space - elem; }
	bool empty() const noexcept { return size() == 0; }
	size_type capacity() const noexcept { return last - elem; }
	void shrink_to_fit() { *this = Vector(*this); }
	reference back() { return *(space - 1); }
	reference front() { return *elem; }
	const_reference back() const { return *(space - 1); }
	const_reference front() const { return *elem; }
	reference at(difference_type n) { return elem[n]; }
	const_reference at(difference_type n) const { return elem[n]; }

	iterator begin() noexcept { return iterator(elem); }
	iterator end() noexcept { return iterator(space); }
	const_iterator begin() const noexcept { return const_iterator(elem); }
	const_iterator end() const noexcept { return const_iterator(space); }
	const_iterator cbegin() const noexcept { return const_iterator(elem); }
	const_iterator cend() const noexcept { return const_iterator(space); }

	void reserve(size_type newSize)
	{
		if(newSize <= capacity())
			return;
		reMemory(newSize);
	}
	void resize(size_type newSize, const value_type &val)
	{
		reserve(newSize);
		if(size() < newSize)
			std::uninitialized_fill(elem + size(), elem + newSize, val);
		else
			destroy(elem + newSize, elem + size());
		space = elem + newSize;
	}
	void resize(size_type newSize)
	{
		reserve(newSize);
		if(size() < newSize)
			std::uninitialized_value_construct(elem + size(), elem + newSize);
		else
			destroy(elem + newSize, elem + size());
		space = elem + newSize;
	}

	void clear() noexcept
	{
		destroy(elem, space);
		space = elem;
	}

	void push_back(const value_type &val = {})
	{
		checkMemory();
		constructCurrent(val);
	}
	void push_back(value_type &&val)
	{
		checkMemory();
		constructCurrent(std::move(val));
	}

	template<typename... Argv>
	void emplace_back(Argv&&... argv)
	{
		checkMemory();
		alloc.construct(space, std::forward<Argv>(argv)...);
		++space;
	}

	void pop_back() noexcept
	{
		destroyCurrent();
	}

#ifndef DELETE_PUSH_FRONT
	void push_front(const value_type &val = {})
	{
		checkMemory();
		if(empty())
		{
			constructCurrent(val);
			return;
		}
		alloc.construct(space, *(space - 1));
		++space;
		std::move(std::reverse_iterator<iterator>(begin() + size() - 2), std::reverse_iterator<iterator>(begin()), std::reverse_iterator<iterator>(begin() + size() - 1));
		alloc.destroy(elem);
		alloc.construct(elem, val);
	}

	void push_front(value_type &&val)
	{
		checkMemory();
		if(empty())
		{
			constructCurrent(std::move(val));
			return;
		}
		alloc.construct(space, std::move(*(space - 1)));
		++space;
		std::move(std::reverse_iterator<iterator>(begin() + size() - 2), std::reverse_iterator<iterator>(begin()), std::reverse_iterator<iterator>(begin() + size() - 1));
		alloc.destroy(elem);
		alloc.construct(elem, std::move(val));
	}
#endif

private:
	template<typename V>
	void constructCurrent(V &&val)
	{
		alloc.construct(space, std::forward<T>(val));
		++space;
	}

	template<typename V>
	void destroyCurrent(V &&val)
	{
		alloc.destroy(space);
		--space;
	}

	template<typename F, typename... Argv>
	void initMemory(size_type n, F func, Argv&&... argv)
	{
		try {
			last = space = func(std::forward<Argv>(argv)...);
		}
		catch(...) {
			alloc.deallocate(elem, n);
			elem = space = last = nullptr;
			throw;
		}
	}

	void reMemory(size_type newSize)
	{
		pointer temp = alloc.allocate(newSize);
		try {
			std::uninitialized_move(elem, space, temp);
		}
		catch(...) {
			alloc.deallocate(temp, newSize);
			throw;
		}
		std::destroy(elem, space);
		alloc.deallocate(elem, capacity());
		space = temp + size();
		last = (elem = temp) + newSize;
	}

	void checkMemory()
	{
		if(size() == capacity())
			reMemory((size() + 1) * 2);
	}

	A alloc;
	pointer elem = nullptr, space = nullptr, last = nullptr;
};

namespace std {
template<typename T, typename A>
void swap(Vector<T, A> &lhs, Vector<T, A> &rhs) { lhs.swap(rhs); }
}

template<typename T, typename A>
Vector(std::initializer_list<T>, const A&)
	-> Vector<T, A>;

template<typename N, typename Input, typename A>
Vector(N, const Input&, const A&)
	-> Vector<Input, A>;

template<typename InputIt, typename A>
Vector(InputIt, InputIt, A)
	-> Vector<typename std::iterator_traits<InputIt>::value_type, A>;

template<typename InputIt>
Vector(InputIt, InputIt)
	-> Vector<typename std::iterator_traits<InputIt>::value_type, std::allocator<typename std::iterator_traits<InputIt>::value_type>>;

template<typename Input, typename A>
Vector(Input, A)
	-> Vector<typename std::iterator_traits<Input>::value_type, A>;

template<typename T>
class Vector<T&> final : public Vector<T> { static_assert(std::is_reference<T>(), "error"); };

template<typename T>
class Vector<T&&> final : public Vector<T> { static_assert(std::is_reference<T>(), "error"); };

template<typename T, typename A>
class Vector<T&, A> final : public Vector<T, A> { static_assert(std::is_reference<T>(), "error"); };

template<typename T, typename A>
class Vector<T&&, A> final : public Vector<T, A> { static_assert(std::is_reference<T>(), "error"); };
