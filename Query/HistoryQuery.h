#pragma once
#include "Query.h"
#include "QueryResult.h"
#include <list>

namespace Chapter15 {
class HistoryQuery final
{
	friend std::ostream& operator<<(std::ostream &, const HistoryQuery &);
public:
	void addInfo(const std::string &s, const Query &q)
	{
		info.push_back(std::make_pair(s, q));
	}
	void removeInfo(std::size_t lineNo) 
	{
		if(lineNo >= info.size())
			throw std::out_of_range("没有该条记录!");
		info.erase(info.begin() + lineNo);
	}
	std::pair<std::string, Query> find(std::size_t no) const
	{
		if(no >= info.size())
			throw std::out_of_range("没有该条记录!");
		return info[no];
	}
private:
	std::vector<std::pair<std::string, Query>> info;
};

std::ostream& operator<<(std::ostream &, const HistoryQuery &);
}
