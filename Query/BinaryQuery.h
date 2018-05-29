#pragma once
#include "Query.h"

namespace Chapter15 {

class BinaryQuery : public QueryBase
{
protected:
	BinaryQuery(const Query &, const Query &, const std::string &);
	virtual std::string rep() const override final;
	Query lhs, rhs;
	std::string opSym;
};

inline BinaryQuery::BinaryQuery(const Query &left, const Query &right, const std::string &op) : lhs(left), rhs(right), opSym(op) { }

inline std::string BinaryQuery::rep() const {
	return "(" + lhs.rep() + " " + opSym + " " + rhs.rep() + ")";
}

}
