#include "TokenStream.h"

const Token& TokenStream::get()
{
	*streamp >> curToken;
	return curToken;
}

TokenStream& operator>>(TokenStream &ts, Token &token)
{
	token = ts.get();
	return ts;
}

void TokenStream::setInput(std::istream *istrm) noexcept {
	close();
	streamp = istrm;
	owns = true;
}
void TokenStream::setInput(std::istream &istrm) noexcept {
	close();
	streamp = &istrm;
	owns = false;
}
