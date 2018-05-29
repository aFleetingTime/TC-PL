#pragma once
#include <iostream>
#include <memory>
#include <vector>
#include <set>
#include <string>
#include "StrVec.h"

namespace Chapter10 {
class QueryResult;

std::ostream& operator<<(std::ostream &, const QueryResult &);

class QueryResult
{
	friend std::ostream& operator<<(std::ostream &, const QueryResult &);

public:
	using line_type = StrVec::size_type;
	using WordLine_Type = std::pair<std::string, std::shared_ptr<std::set<line_type>>>;
	using text_ptr = std::shared_ptr<StrVec>;

	QueryResult() = default;
	QueryResult(const text_ptr &, const WordLine_Type &);

	text_ptr getText();
	std::set<QueryResult::line_type>::iterator begin();
	std::set<QueryResult::line_type>::iterator end();

private:
	text_ptr lineText;
	WordLine_Type wordLine;
};
}
