#pragma once
#include <type_traits>
#include <vector>

class ObjectBase
{
public:
	//template<typename T>
	virtual std::enable_if<true, int> enable() const { }
	virtual ~ObjectBase() = 0;
};

template<typename T>
class Object : ObjectBase
{
public:
	using type = T;
	Object(const type &val) : value(val) { }
	Object(type &&val) : value(std::move(val)) { }
	operator type&() { return value; }
	operator const type&() const { return value; }

private:
	type value;
};

class ObjectVec
{
	std::vector<int> v;
};

int main()
{
}
