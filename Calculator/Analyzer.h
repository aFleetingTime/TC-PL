#pragma once
#include <map>
#include <optional>
#include "TokenStream.h"
#include "BadEntry.h"
#include "ReadTokenError.h"
#include "DivideByZeroError.h"

template<typename T>
class Analyzer final
{
public:
	using result_type = std::optional<T>;
	enum class Errnos : short {
		no_error, divide_by_zero, missing_right_parenthesis, primary_expected, 
		lack_of_operational_objects, end_of_transmission, empty_expression, invalid_expression,
		variable_length_is_out_of_range
	};
	explicit Analyzer(TokenStream &ts) : tcin(ts) { }
	result_type calculate() {
		try {
			Kind k = tcin.get().getKind();
			if(k == Kind::print)
				return calculate();
			else if(k == Kind::end)
				return errorInfo("表达式为空", Errnos::empty_expression);
			else if(!tcin)
			{
				errcno = Errnos::end_of_transmission;
				return std::nullopt;
			}
			else
			{
				auto tmp = expr(false);
				if(!tmp && errcno != Errnos::primary_expected)
					tcin.clear();
				return tmp;
			}
		} catch(const ReadTokenError &e) {
			tcin.clear();
			errcno = Errnos::invalid_expression;
			throw;
		}
	}
	void reset(Errnos eno = Errnos::no_error) noexcept {
		errcno = eno;
	}
	Errnos errnos() const noexcept {
		return errcno;
	}
	bool addVariable(const std::pair<std::string, T> &p) noexcept {
		return table.insert(p).second;
	}
	const std::map<std::string, T>& variables() const noexcept {
		return table;
	}
	bool owns = false;
private:
	Errnos errcno;
	result_type prim(bool getflag);
	result_type term(bool getflag);
	result_type expr(bool getflag);
	TokenStream &tcin;
	std::map<std::string, T> table;
	result_type errorInfo(const std::string &info, Errnos eno);
};

template<typename T>
typename Analyzer<T>::result_type Analyzer<T>::errorInfo(const std::string &info, Errnos eno)
{
	errcno = eno;
	std::cerr << "Error[" << static_cast<short>(eno) << "]: " << info << '\n';
	return std::nullopt;
}

template<typename T>
typename Analyzer<T>::result_type Analyzer<T>::prim(bool getflag)
{
	static bool flag = false;
	if(getflag) tcin.get();
	switch(tcin.current().getKind())
	{
	case Kind::number:
	{
		T d = tcin.current().getNumValue();
		if(tcin.get().getKind() == Kind::rp && !flag)
			return errorInfo("')'括号不匹配", Errnos::missing_right_parenthesis);
		return d;
	}
	case Kind::name:
	{
		// New
		auto name = tcin.current().getNameValue();
		if(name.size() > 8)
			return errorInfo("变量名过长(不超过8字符)", Errnos::variable_length_is_out_of_range);
		if(tcin.get().getKind() == Kind::assign)
			return table[name] = expr(true).value_or(0);
		return table[name];
	}
	case Kind::minus:
		return -(prim(true).value_or(0));
	case Kind::lp:
	{
		flag = true;
		result_type d = expr(true);
		flag = false;
		if(!d)
			return d;
		if(tcin.current().getKind() != Kind::rp)
			return errorInfo("'('括号不匹配", Errnos::missing_right_parenthesis);
		tcin.get();
		return d;
	}
	case Kind::rp:
		return errorInfo(flag ? "括号内容为空" : "')'括号不匹配", Errnos::missing_right_parenthesis);
	case Kind::plus:
	case Kind::mul:
	case Kind::div:
	case Kind::mod:
		tcin.get();
		return errorInfo("缺少运算对象", Errnos::lack_of_operational_objects);
	default:
		return errorInfo("与预期primary不符", Errnos::primary_expected);
	}
	return std::nullopt;
}

template<typename T>
typename Analyzer<T>::result_type Analyzer<T>::term(bool getflag)
{
	result_type left = prim(getflag);
	while(left)
	{
		switch(tcin.current().getKind())
		{
		case Kind::mul:
			if(auto tmp = prim(true))
			{
				left.value() *= tmp.value();
				break;
			}
			else return tmp;
		case Kind::div:
			if(auto tmp = prim(true))
			{
				if(T d = tmp.value())
				{
					left.value() /= d;
					break;
				}
				errcno = Errnos::divide_by_zero;
				throw DivideByZeroError("除0错误");
			}
			else return tmp;
		case Kind::mod:
			if(auto tmp = prim(true))
			{
				if(T d = tmp.value())
				{
					left.value() = static_cast<long long>(left.value()) % static_cast<long long>(d);
					break;
				}
				errcno = Errnos::divide_by_zero;
				throw DivideByZeroError("除0错误");
			}
			else return tmp;
		default:
			return left;
		}
	}
	return std::nullopt;
}

template<typename T>
typename Analyzer<T>::result_type Analyzer<T>::expr(bool getflag)
{
	result_type left = term(getflag);
	while(left)
	{
		switch(tcin.current().getKind())
		{
		case Kind::plus:
			if(auto tmp = term(true))
			{
				left.value() += tmp.value();
				break;
			}
			else return tmp;
		case Kind::minus:
			if(auto tmp = term(true))
			{
				left.value() -= tmp.value();
				break;
			}
			else return tmp;
		case Kind::print:
			owns = true;
			return left;
		case Kind::end:
			owns = false;
			return left;
		case Kind::name:
		case Kind::number:
			return std::nullopt;
		default:
			return left;
		}
	}
	return std::nullopt;
}
