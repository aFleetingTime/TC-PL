#pragma once
#include "Token.h"

class TokenStream
{
	friend TokenStream& operator>>(TokenStream &ts, Token &token);
public:
	explicit TokenStream(std::istream *istrm) : streamp(istrm), owns(true) { }
	explicit TokenStream(std::istream &istrm) : streamp(&istrm), owns(false) { }
	~TokenStream() { close(); }

	const Token& get();
	const Token& current() const noexcept {
		return curToken;
	}
	const Token& operator*() const noexcept {
		return curToken;
	}

	operator bool() const noexcept {
		return bool(*streamp);
	}

	void setInput(std::istream *istrm) noexcept;
	void setInput(std::istream &istrm) noexcept;

	void clear() noexcept {
		streamp->clear();
		streamp->unget();
		while(*streamp && streamp->get() != '\n');
	}
	void close() noexcept {
		if(owns)
			delete streamp;
	}

private:
	std::istream *streamp;
	bool owns;
	Token curToken{Kind::end};
};

TokenStream& operator>>(TokenStream &ts, Token &token);
