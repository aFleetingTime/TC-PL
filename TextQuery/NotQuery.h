#include "Query.h"
#include <list>
#include <numeric>

int main();

namespace Chapter15 {
class NotQuery : public QueryBase
{
	friend class Query;
	friend Query operator~(const Query &);
	friend int ::main();

private:
	NotQuery(const Query &);
	Chapter10::QueryResult eval(const Chapter10::TextQuery &) const override;
	std::string rep() const override;
	Query query;
	std::shared_ptr<QueryBase> p;
	virtual operator QueryBase*() const & override;
	virtual operator QueryBase*() && override;
};
}
