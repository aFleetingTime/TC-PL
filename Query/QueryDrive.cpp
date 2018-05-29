#include "QueryDrive.h"

namespace Chapter15 {

Query QueryDrive::createQuery(const std::string &exp)
{
	instrm.clear();
	instrm.str(exp);
	Query tmp = expr(false);
	history.addInfo(tmp.rep(), tmp);
	return tmp;
}

bool QueryDrive::isOp(int ch)
{
	if(ch == '|' || ch == '&' || ch == '~')
		return true;
	return false;
}

std::string QueryDrive::getNextWord()
{
	if(!instrm)
		return "";
	std::string s;
	int ch = instrm.get();
	while(!isOp(ch) && instrm)
	{
		s += ch;
		ch = instrm.get();
		if(isOp(ch))
			instrm.unget();
	}
	return s;
}

Query QueryDrive::prim(bool get)
{
	if(get) getNextWord();
	switch(instrm.peek())
	{
	case '~':
		instrm.get();
		return ~Query(getNextWord());
	case '(':
		return expr(true);
	}
	return { getNextWord() };
}

Query QueryDrive::term(bool get)
{
	Query left = prim(get);
	while(true)
	{
		if(instrm.peek() == '&')
			left = left & prim(true);
		else
			return std::move(left);
	}
}

Query QueryDrive::expr(bool get)
{
	Query left = term(get);
	while(true)
	{
		if(instrm.peek() == '|')
			left = left | term(true);
		else
			return std::move(left);
	}
}
}
