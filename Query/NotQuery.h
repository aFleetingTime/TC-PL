#include "Query.h"
#include <list>
#include <numeric>

namespace Chapter15 {
class NotQuery : public QueryBase
{
	friend class Query;
	friend Query operator~(const Query &);

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
