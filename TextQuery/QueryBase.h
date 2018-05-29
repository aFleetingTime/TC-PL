#pragma once
#include "TextQuery.h"

namespace Chapter15 {
class QueryBase
{
	friend class Query;

protected:
	using line_type = Chapter10::TextQuery::line_type;
	virtual ~QueryBase() = default;
	virtual operator QueryBase*() const & = 0;
	virtual operator QueryBase*() && = 0;

private:
	virtual Chapter10::QueryResult eval(const Chapter10::TextQuery &) const = 0;
	virtual std::string rep() const = 0;
};
}
