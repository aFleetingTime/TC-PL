#include "OrQuery.h"

namespace Chapter15 {
OrQuery::OrQuery(const Query &left, const Query &right) : BinaryQuery(left, right, "|") { }

OrQuery::operator QueryBase*() const & { return new OrQuery(*this); }
OrQuery::operator QueryBase*() && { return new OrQuery(std::move(*this)); }

Chapter10::QueryResult OrQuery::eval(const Chapter10::TextQuery &tq) const
{
	auto left = lhs.eval(tq), right = rhs.eval(tq);
	auto lineNo = std::make_shared<std::set<line_type>>();
	std::set_union(left.begin(), left.end(), right.begin(), right.end(), std::inserter(*lineNo, lineNo->begin()));
	return { left.getText(), { rep(), lineNo } };
}

Query operator|(const Query &left, const Query &right) {
	return new OrQuery(left, right);
}
}
