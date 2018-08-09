#pragma once
#include <string>
#include <iostream>

enum class Kind : char
{
	end, name, number,
	plus = '+', minus = '-', div = '/', mul = '*', mod = '%', print = ';', assign = '=', lp = '(', rp = ')'
};

class Token final
{
	friend std::istream& operator>>(std::istream &istrm, Token &token);
public:
	explicit Token(Kind k) : kind(k) {
		if(k == Kind::name)
			new(&nameValue) std::string;
		else
			numValue = {};
	}
	explicit Token(const long double &num) : kind(Kind::number), numValue(num) { }
	explicit Token(const std::string &name) : kind(Kind::name), nameValue(name) { }
	Token(const Token &rhs);
	~Token() {
		if(kind == Kind::name)
			nameValue.~basic_string();
	}
	const long double getNumValue() const;
	const std::string& getNameValue() const;
	const Kind getKind() const noexcept {
		return kind;
	}

	Token& operator=(const Token &rhs) noexcept;
	Token& operator=(long double d) noexcept;
	Token& operator=(const std::string &s) noexcept;

private:
	Kind kind;
	union {
		long double numValue;
		std::string nameValue;
	};
};

std::istream& operator>>(std::istream &istrm, Token &token);
