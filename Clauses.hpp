//
// Created by asmak on 5/15/2023.
//

#ifndef SP23DATABASE_CLAUSE_HPP
#define SP23DATABASE_CLAUSE_HPP

#include "Filters.hpp"
#include "Table.hpp"

namespace ECE141 {
    enum class ClauseType{
        orderBy,
        limit,
        where,
        join,
        no_type
    };
    struct Join {
        Join() {};
        Join(const std::string& aTable, Keywords aType, const std::string& aLHS, const std::string& aRHS)
            : joinType(aType), table(aTable),  onLeft(aLHS), onRight(aRHS) {}

        Keywords    joinType;
        std::string table;
        TableField  onLeft;
        TableField  onRight;
    };
    using JoinList = std::vector<Join>;
    using ClauseTypes = std::vector<ClauseType>;
    class Clauses {
    public:
        Clauses() :limit(UINT32_MAX) {};
        bool matches(const Row& aRow) const;
        StatusResult execute(Table& aTable) const;
        //StatusResult eachMatchingBlock(Table& aTable, MatchingIterator& aMatchIterator) const;
        void setFilters(Filters& aFilters);
        void setLimit(ind aLimit);
        void setOrderingVar(Order& anOrder);
        void setJoin(Join& aJoin);
        bool getJoin(Join& aJoin) const;
        size_t size() const{ return clauseTypes.size(); }
    protected:
        ClauseTypes clauseTypes;
        Filters filters;
        ind limit;
        Order order;
        Join join;


    };


    using ClauseHandlers = std::map<Keywords,std::function<StatusResult(CommandTokenReader& aCommandTokenReader,Schema aSchema, Clauses& aClauses)>>;



} // ECE141

#endif //SP23DATABASE_CLAUSE_HPP
