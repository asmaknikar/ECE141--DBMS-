//
// Created by asmak on 5/15/2023.
//

#include "Clauses.hpp"
#include "Filters.hpp"
namespace ECE141 {
	bool Clauses::matches(const Row& aRow) const{ 
		bool matches = true;
		if (Helpers::in_array(clauseTypes, ClauseType::where)) matches = filters.matches(aRow.getData());
		return matches;
	}
	StatusResult executeLimitClause(Table& aTable, ind limit) {
		if(limit < aTable.rows.size()) aTable.rows.erase(aTable.rows.begin() + limit, aTable.rows.end());
		return StatusResult();
	}
	StatusResult executeOrderClause(Table& aTable, Order anOrder) {
		aTable.sortRows(anOrder);
		return StatusResult();
	}
	void Clauses::setFilters(Filters& aFilters) {
		filters = aFilters;
		clauseTypes.push_back(ClauseType::where);
	}
	void Clauses::setLimit(ind aLimit) {
		limit = aLimit;
		clauseTypes.push_back(ClauseType::limit);
	}
	void Clauses::setJoin(Join& aJoin) {
		join = aJoin;
		clauseTypes.push_back(ClauseType::join);
	}
	void Clauses::setOrderingVar(Order& anOrder) {
		order = anOrder;
		clauseTypes.push_back(ClauseType::orderBy);
	}
	bool Clauses::getJoin(Join& aJoin) const{
		if (Helpers::in_array(clauseTypes, ClauseType::join)) {
			aJoin = join; 
			return true;
		}
		else return false;
	}
    StatusResult Clauses::execute(Table& aTable) const{
		StatusResult theResult;
		//if (Helpers::in_array(clauseTypes, ClauseType::where)) theResult = executeWhereClause(aTable, filters);
		if (Helpers::in_array(clauseTypes, ClauseType::orderBy)) theResult = executeOrderClause(aTable, order);
		if (Helpers::in_array(clauseTypes, ClauseType::limit)) theResult = executeLimitClause(aTable, limit);
		return theResult;
  }
	/*StatusResult Clauses::eachMatchingBlock(Table& aTable, MatchingIterator& aMatchIterator) const {
		StatusResult theResult;
		if (Helpers::in_array(clauseTypes, ClauseType::where)) {
			theResult = executeWhereClause(aTable, filters, [&](ind aBlockNum) {
				aMatchIterator(aBlockNum);
				return true;
				});
		}
		if (Helpers::in_array(clauseTypes, ClauseType::limit)) theResult = executeLimitClause(aTable, limit);
		return theResult;
	}*/


} // ECE141