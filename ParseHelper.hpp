//
//  ParseHelper.hpp
//  RGAssignment4
//
//  Created by rick gessner on 4/18/21.
//

#ifndef ParseHelper_hpp
#define ParseHelper_hpp

#include <stdio.h>
#include "keywords.hpp"
#include "BasicTypes.hpp"
#include "CommandTokenReader.hpp"
#include "Attribute.hpp"
#include "Filters.hpp"
#include "Table.hpp"
#include "DBQuery.hpp"

namespace ECE141 {

  //-------------------------------------------------
    struct TableName {
        TableName(const std::string& aTableName, const std::string& anAlias = "")
            : table(aTableName), alias(anAlias) {}
        TableName(const TableName& aCopy) : table(aCopy.table), alias(aCopy.alias) {}
        TableName& operator=(const std::string& aName) {
            table = aName;
            return *this;
        }
        operator const std::string() { return table; }
        std::string table;
        std::string alias;
    };

  class Entity;
  struct Expression;
  
  struct ParseHelper {
            
    ParseHelper(CommandTokenReader &aTokenReader) : tokenReader(aTokenReader) {}
        
    StatusResult parseTableName(std::string &aTableName);
    StatusResult parseTableField(TableField& aField);
    StatusResult parseAttributeOptions(Attribute &anAttribute);

    StatusResult parseAttribute(Attribute &anAttribute);

    StatusResult parseIdentifierList(StringList &aList);
    StatusResult parseJoin(Join& aJoin);
    StatusResult parseAssignments(Expressions &aList, Schema&);
    StatusResult parseFromTable(std::string& aTableName);
    StatusResult parseValueList(StringList &aList);
    StatusResult parseOrder(Order& anOrder);
    StatusResult parseOperator(Operators &anOp);
    StatusResult parseOperand(Schema&, Operand&);
    StatusResult parseExpression(Schema&, Expression&);
    StatusResult parseLogical(Logical& aLogic);
    StatusResult parseQuery(DBQuery& aQuery);
    StatusResult parseClauses(Schema_ptr aSchema, Clauses& aClauses);
    CommandTokenReader &tokenReader;
  };

}

#endif /* ParseHelper_hpp */

