#pragma once
#include <map>
#include <string>
#include <sstream>
#include "Query.h"
#include "HistoryQuery.h"

namespace Chapter15 {
class QueryDrive final
{
public:
	QueryDrive() { }
	Query createQuery(const std::string &);
	HistoryQuery& getHistory() { return history; }
	Query combination(std::size_t, std::size_t, char);

private:
	Query expr(bool get);
	Query term(bool get);
	Query prim(bool get);
	HistoryQuery history;
	std::istringstream instrm;
	bool isOp(int ch);
	std::string getNextWord();
};
}
