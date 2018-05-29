#pragma once
#include <utility>
#include <iostream>
#include <type_traits>

template<typename T, bool B = false>
class copyow_ptr { };

template<typename T>
class copyow_ptr<T, false>
{
	friend class copyow_ptr<T, true>;
public:
	using element_type = typename std::remove_extent<T>::type;
	explicit copyow_ptr(element_type *p) : ptr(p) { std::cout << "A" << std::endl; }

	copyow_ptr(copyow_ptr &&rhs) : ptr(std::exchange(rhs.ptr, nullptr)) { std::cout << "AD" << std::endl; }

	template<typename... Argv>
	explicit copyow_ptr(Argv&&... argv) : ptr(new T{std::forward<Argv>(argv)...}) { }

	const element_type& operator*() const { return *ptr; }
	copyow_ptr& operator=(element_type &&val)
	{
		*ptr = std::move(val);
		return *this;
	}
	copyow_ptr& operator=(const element_type &val)
	{
		*ptr = val;
		return *this;
	}

	~copyow_ptr() { delete ptr; }

private:
	element_type *ptr;
};

template<typename T>
class copyow_ptr<T, true>
{
public:
	using element_type = typename std::remove_extent<T>::type;

	template<bool B>
	copyow_ptr(const copyow_ptr<T, B> &p) : ptr(p.ptr) { }

	//copyow_ptr(copyow_ptr &&p) noexcept : ptr(std::exchange(p.ptr, nullptr)), isShared(std::exchange(p.isShared, true)) { std::cout << "A" << std::endl; }

	const element_type& operator*() const { return *ptr; }

	copyow_ptr& operator=(element_type &&val)
	{
		checkShared();
		*ptr = std::move(val);
		return *this;
	}
	copyow_ptr& operator=(const element_type &val)
	{
		checkShared();
		*ptr = val;
		return *this;
	}
	template<typename Y, bool B>
	copyow_ptr& operator=(copyow_ptr<Y, B> rhs)
	{
		swap(*this, rhs);
		return *this;
	}
	template<typename Y, bool B>
	void swap(copyow_ptr<Y, B> &rhs)
	{
		if constexpr(B)
		{
			swap(this->ptr, rhs.ptr);
			swap(this->isShared, rhs.isShared);
		}
		else
			swap(this->ptr, rhs.ptr);
	}

	~copyow_ptr()
	{
		if(!isShared)
			delete ptr;
	}

private:
	element_type *ptr;
	void checkShared()
	{
		if(isShared)
		{
			if constexpr(std::is_nothrow_move_constructible<element_type>())
				ptr = new element_type(std::move(*ptr));
			else ptr = new element_type(*ptr);
			isShared = false;
		}
	}
	bool isShared = true;
};

template<typename Y, bool B>
void swap(copyow_ptr<Y, B> &lhs, copyow_ptr<Y, B> &rhs)
{
	lhs.swap(rhs);
}

template<typename T>
copyow_ptr(const copyow_ptr<T, false>&) -> copyow_ptr<T, true>;

template<typename T>
copyow_ptr(copyow_ptr<T, false>&&) -> copyow_ptr<T, false>;

template<typename T>
copyow_ptr(copyow_ptr<T, true>&&) -> copyow_ptr<T, true>;
