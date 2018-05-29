#include <iostream>
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
	Format(int p = 6, int w = 0, fmtflags f = static_cast<fmtflags>(0)) : fmt(f), width(w), preci(p) { }

	template<typename T>
	BoundFormat<T> operator()(const T &val) { return { *this, val }; }

private:
	fmtflags fmt;
	int width, preci;
};

template<typename T>
std::ostream& operator<<(std::ostream &os, const BoundFormat<T> &bf)
{
#if 0
	auto fmtTmp = os.setf(bf.fmts.fmt);
	int widTmp = os.width(bf.fmts.width), preTmp = os.precision(bf.fmts.preci);
	os << bf.value;
	os.flags(fmtTmp);
	os.width(widTmp);
	os.precision(preTmp);
#endif
	std::ostringstream form;
	form.setf(bf.fmts.fmt, std::ios_base::floatfield);
	form.width(bf.fmts.width);
	form.precision(bf.fmts.preci);
	form << bf.value;
	return os << form.str();
}

Format hexFloat(4, 0, std::ios_base::scientific | std::ios_base::fixed);
Format scien(0, 0, std::ios_base::scientific);
Format longfixed(16, 0, std::ios_base::fixed);

int main()
{
	std::cout << hexFloat(100.888) << ' ' << 100.888 << std::endl;
	std::cout << scien(777.2929) << ' ' << 777.2929 << std::endl;
	std::cout << longfixed(777.29299999999999999999) << ' ' << 777.29299999999999999999 << std::endl;
}
