#include "QueryResult.h"

namespace Chapter10 {
std::ostream& operator<<(std::ostream &out, const QueryResult &result)
{
	if(!result.wordLine.second)
	{
		std::cout << "文本不包含单词: " << result.wordLine.first << '\n';
		return out;
	}
	out << "字符串: " << result.wordLine.first << "	包含该字符串的行数: " << result.wordLine.second->size() << std::endl;
	for(const auto &lineNo : *result.wordLine.second)
		out << "\t(line " << lineNo + 1 << ") " << result.lineText->at(lineNo) << '\n';
	return out;
}

QueryResult::QueryResult(const text_ptr &text, const WordLine_Type &pair) : lineText(text), wordLine(pair) {}

std::set<QueryResult::line_type>::iterator QueryResult::begin()
{
	return wordLine.second->begin();
}

std::set<QueryResult::line_type>::iterator QueryResult::end()
{
	return wordLine.second->end();
}

QueryResult::text_ptr QueryResult::getText()
{
	return lineText;
}
}
