#pragma once
#include <ostream>
#include <sstream>

class Format;

template<typename T>
struct BoundFormat
{
	const Format &fmts;
	const T &value;
};

class Format
{
	template<typename T>
	friend std::ostream& operator<<(std::ostream &os, const BoundFormat<T> &bf);

public:
	using fmtflags = typename std::ios_base::fmtflags;
	constexpr Format(int p = 6, int w = 0, fmtflags f = fmtflags{}) : fmt(f), width(w), preci(p) { }

	template<typename T>
	BoundFormat<T> operator()(const T &val) const { return { *this, val }; }

private:
	fmtflags fmt;
	int width, preci;
};

template<typename T>
std::ostream& operator<<(std::ostream &os, const BoundFormat<T> &bf)
{
	std::ostringstream form;
	form.setf(bf.fmts.fmt, std::ios_base::floatfield);
	form.width(bf.fmts.width);
	form.precision(bf.fmts.preci);
	form << bf.value;
	return os << form.str();
}
