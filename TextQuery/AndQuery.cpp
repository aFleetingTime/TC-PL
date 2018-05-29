#include "AndQuery.h"

namespace Chapter15 {
AndQuery::AndQuery(const Query &left, const Query &right) : BinaryQuery(left, right, "&") { }

AndQuery::operator QueryBase*() const & { return new AndQuery(*this); }
AndQuery::operator QueryBase*() && { return new AndQuery(*this); }

Chapter10::QueryResult AndQuery::eval(const Chapter10::TextQuery &tq) const
{
	auto leftLine = lhs.eval(tq), rightLine = rhs.eval(tq);
	auto lineNo = std::make_shared<std::set<line_type>>();
	std::set_intersection(leftLine.begin(), leftLine.end(), rightLine.begin(), rightLine.end(), std::inserter(*lineNo, lineNo->begin()));
	return { leftLine.getText(), { rep(), lineNo } };
}

Query operator&(const Query &lhs, const Query &rhs)
{
	return new AndQuery(lhs, rhs);
}
}
