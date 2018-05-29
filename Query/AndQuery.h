#pragma once
#include "BinaryQuery.h"
#include "TextQuery.h"
#include "QueryResult.h"
#include <algorithm>

namespace Chapter15 {
class AndQuery : public BinaryQuery
{
	friend Query operator&(const Query &, const Query &);

private:
	AndQuery(const Query &, const Query &);
	Chapter10::QueryResult eval(const Chapter10::TextQuery &) const override;
	virtual operator QueryBase*() const & override;
	virtual operator QueryBase*() && override;
};
}
