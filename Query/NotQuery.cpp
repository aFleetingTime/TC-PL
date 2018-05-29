#include "NotQuery.h"

namespace Chapter15 {
NotQuery::NotQuery(const Query &q) : query(q) { }

NotQuery::operator QueryBase*() const & { return new NotQuery(*this); }
NotQuery::operator QueryBase*() && { return new NotQuery(*this); }

Chapter10::QueryResult NotQuery::eval(const Chapter10::TextQuery &tq) const
{
	Chapter10::QueryResult result = query.eval(tq);
	std::list<line_type> notQueryList(result.getText()->size());
	std::iota(notQueryList.begin(), notQueryList.end(), 0);
	for(const auto &no : result) {
		notQueryList.remove(no);
	}
	std::shared_ptr<std::set<line_type>> lineNo = std::make_shared<std::set<line_type>>(notQueryList.cbegin(), notQueryList.cend());
	return Chapter10::QueryResult(result.getText(), { rep(), lineNo });
}

std::string NotQuery::rep() const {
	return "~(" + query.rep() + ")";
}

Query operator~(const Query &query) {
	return new NotQuery(query);
}
}
