#include "Query.h"
#include "AndQuery.h"
#include "NotQuery.h"
#include "OrQuery.h"

using namespace Chapter15;

void check(AndQuery *p)
{
	if(p) std::cout << "Success";
	else  std::cout << "Failure";
	std::cout << std::endl;
}
void checkRef(QueryBase &r)
{
	try {
		AndQuery &a = dynamic_cast<AndQuery&>(r);
		std::cout << "Success";
	} catch(std::bad_cast &e) {
		std::cout << "Failure";
	}
	std::cout << std::endl;
}
bool compare(QueryBase &b1, QueryBase &b2)
{
	return typeid(b1) == typeid(b2);	
}
template <typename T>
bool compare(QueryBase &b)
{
	return typeid(b) == typeid(T);	
}

int main()
{
	QueryBase *bpa = new AndQuery(Query("A"), Query("B"));
	QueryBase *bpn = new NotQuery(Query("A"));
	QueryBase *bpo = new OrQuery(Query("A"), Query("B"));
	check(dynamic_cast<AndQuery*>(bpa));
	check(dynamic_cast<AndQuery*>(bpn));
	check(dynamic_cast<AndQuery*>(bpo));
	checkRef(*bpa);
	checkRef(*bpn);
	checkRef(*bpo);
	std::cout << std::boolalpha << compare(*bpa, *bpn) << ' ' << compare(*bpa, *bpo) << std::endl;
	std::cout << compare<AndQuery>(*bpa) << ' ' << compare<OrQuery>(*bpa) << ' ' << compare<NotQuery>(*bpa) << std::noboolalpha << std::endl;
}
