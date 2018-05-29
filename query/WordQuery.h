#pragma once
#include "QueryBase.h"

namespace Chapter15 {
class WordQuery : public QueryBase
{
	friend class Query;

private:
	WordQuery(const std::string &);
	Chapter10::QueryResult eval(const Chapter10::TextQuery &) const override;
	std::string rep() const override;
	std::string queryWord;
	virtual operator QueryBase*() const & override;
	virtual operator QueryBase*() && override;
};
}
