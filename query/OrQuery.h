#pragma once
#include "BinaryQuery.h"
#include <iterator>

int main();

namespace Chapter15 {
class OrQuery : public BinaryQuery
{
	friend Query operator|(const Query &, const Query &);
	friend int ::main();

private:
	OrQuery(const Query &, const Query &);
	Chapter10::QueryResult eval(const Chapter10::TextQuery &) const override;
	virtual operator QueryBase*() const & override;
	virtual operator QueryBase*() && override;
};
}
