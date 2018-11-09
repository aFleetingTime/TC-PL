#include "Token.h"
#include "BadEntry.h"
#include "ReadTokenError.h"

Token::Token(const Token &rhs) : kind(rhs.kind) 
{
	if(rhs.kind == Kind::name) new (&nameValue) std::string(rhs.nameValue);
	else if(rhs.kind == Kind::number) numValue = rhs.numValue;
}

const long double Token::getNumValue() const
{
	if(kind != Kind::number)
		throw BadEntry("当前类型不是double");
	return numValue;
}

const std::string& Token::getNameValue() const
{
	if(kind != Kind::name)
		throw BadEntry("当前类型不是string");
	return nameValue;
}

Token& Token::operator=(const Token &rhs) noexcept
{
	if(rhs.kind == Kind::name)
		*this = rhs.nameValue;
	else if(rhs.kind == Kind::number)
		*this = rhs.numValue;
	else
		kind = rhs.kind;
	return *this;
}

Token& Token::operator=(const std::string &s) noexcept
{
	if(kind == Kind::name) nameValue = s;
	else new (&nameValue) std::string(s);
	kind = Kind::name;
	return *this;
}

Token& Token::operator=(long double d) noexcept
{
	this->~Token();
	numValue = d;
	kind = Kind::number;
	return *this;
}

std::istream& operator>>(std::istream &istrm, Token &token)
{
	Kind ch;
	while(std::isspace(istrm.peek()))
		if(istrm.get() == '\n')
		{
			token.kind = Kind::end;
			return istrm;
		}
	ch = static_cast<Kind>(istrm.get());
	if(!istrm)
	{
		token.kind = Kind::end;
		return istrm;
	}
#if 0
	do {
		ch = static_cast<Kind>(istrm.get());
		if(!istrm)
		{
			token.kind = Kind::end;
			return istrm;
		}
	} while(static_cast<char>(ch) != '\n' && std::isspace(static_cast<char>(ch)));
#endif
	switch(ch)
	{
	case Kind::print:
		if(istrm.peek() == '\n')
		{
			istrm.get();
			token.kind = Kind::end;
		}
		else
			token.kind = Kind::print;
		break;
	case Kind::plus:
	case Kind::minus:
	case Kind::mul:
	case Kind::div:
	case Kind::mod:
	case Kind::lp:
	case Kind::rp:
	case Kind::assign:
		token.kind = ch;
		break;
	default:
		char tmp = static_cast<char>(ch);
		if(!tmp)
			token.kind = Kind::end;
		else if((tmp >= '0' && tmp <= '9') || (tmp == '.'))
		{
			long double num;
			istrm.unget() >> num;
			token = num;
			if(std::isalpha(istrm.peek()))
				throw ReadTokenError("变量名不能以数字开头");
		}
		else if(std::isalpha(tmp))
		{
			token = std::string(1, tmp);
			while(istrm.get(tmp) && std::isalnum(tmp))
				token.nameValue += tmp;
			istrm.unget();
		}
		else
			throw ReadTokenError("表达式无效");
	}
	return istrm;
}
