#include "QueryDrive.h"

namespace Chapter15 {
QueryDrive::QueryDrive() : opSym { { '|', [](const Query &lhs, const Query &rhs) { return lhs | rhs; } },
								   { '&', [](const Query &lhs, const Query &rhs) { return lhs & rhs; } } } { }

Chapter10::Query QueryDrive::createQuery(const std::string &expr)
{
	outstrm.str(expr);
	return 
	auto tmp = resolve(exp, 0, std::string::npos);
	history.addInfo(tmp.rep(), tmp);
	return tmp;
}

Query QueryDrive::combination(std::size_t a, std::size_t b, char op)
{
	auto q1 = history.find(a), q2 = history.find(b);
	if(op == '|')
		return q1.second | q2.second;
	if(op == '&')
		return q1.second & q2.second;
	throw std::out_of_range("不支持该运算符!");
}

bool QueryDrive::isOp(int ch)
{
	if(ch == '|' && ch == '&' && ch == '~')
		return true;
	return false;
}

std::string getNextWord()
{
	std::string s;
	for(int ch = outstrm.get(); !isOp(ch); ch = outstrm.get())
		s.push_back(ch);
}

Query QueryDrive::prim(bool get)
{
	if(get) getNextWord();
	switch(outstrm.peek())
	{
	case '~':
		return ~Query(getNextWord());
	case '(':
		return expr(true);
	default:
		return { getNextWord() };
	}
}

Query QueryDrive::term(bool get)
{
	Query left = prim(get);
	while(true)
	{
		if(outstrm.peek() == '&')
			left = left & prim(true);
		else
			return left;
	}
}

Query QueryDrive::expr(bool get)
{
	Query left = term(get);
	while(true)
	{
		if(outstrm.peek() == '|')
			left = left | term(true);
		else
			return left;
	}
#if 0
	if(exp[bpos] == '~')
	{
		std::size_t tpos = exp.find_first_of("&|", ++bpos);
		Query q = ~resolve(exp, bpos, epos);
		if(tpos == epos)
			return q;
		return (*opSym.find(exp[tpos])).second(q, resolve(exp, tpos + 1, epos));
	}
	std::size_t tpos = exp.find_first_of("&|", bpos);
	if(tpos != epos)
		return (*opSym.find(exp[tpos])).second(exp.substr(bpos, tpos - bpos), resolve(exp, tpos + 1, epos));
	return Query(exp.substr(bpos, tpos));
}
#endif
}
