#include "Query.h"

namespace Chapter15 {
Query::Query(const std::string &word) : queryType(new WordQuery(word)) { }

Query::Query(const Query &q) : queryType(*q.queryType) { }

Query::Query(Query &&q) noexcept : queryType(q.queryType) { q.queryType = nullptr; }

Query::Query(const QueryBase *p) : queryType(p) { }

Query::~Query() { delete queryType; }

Query& Query::operator=(const Query &rhs)
{
	QueryBase *temp = *rhs.queryType;
	delete queryType;
	queryType = temp;
	return *this;
}

Query& Query::operator=(Query &&rhs) noexcept
{
	if(&rhs != this)
	{
		delete queryType;
		queryType = rhs.queryType;
	}
	return *this;
}

Chapter10::QueryResult Query::eval(const Chapter10::TextQuery &tq) const { return queryType->eval(tq); }

std::string Query::rep() const { return queryType->rep(); }

void swap(Query &lhs, Query &rhs) {
	using std::swap;
	swap(lhs.queryType, rhs.queryType);
}

void Query::swap(Query &rhs) {
	using std::swap;
	swap(*this, rhs);
}
}
