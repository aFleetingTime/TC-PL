#include "WordQuery.h"
#include "Query.h"
#include "QueryResult.h"
#include "TextQuery.h"

namespace Chapter15 {
WordQuery::WordQuery(const std::string &word) : queryWord(word) { }

WordQuery::operator QueryBase*() const & { return new WordQuery(*this); }
WordQuery::operator QueryBase*() && { return new WordQuery(std::move(*this)); }

Chapter10::QueryResult WordQuery::eval(const Chapter10::TextQuery &tq) const { 
	return tq.query(queryWord); 
}

std::string WordQuery::rep() const { 
	return queryWord; 
}
}
