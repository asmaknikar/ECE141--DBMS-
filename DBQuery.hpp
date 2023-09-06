//
//  DBQuery.hpp
//  PA5
//
//  Created by rick gessner on 4/7/23.
//

#ifndef DBQuery_h
#define DBQuery_h

#include "Schema.hpp"
#include "Filters.hpp"
#include "Clauses.hpp"

namespace ECE141 {

  // State held from a fancy select statement

  struct Property {
    Property(std::string aName, uint32_t aTableId=0) : name(aName), tableId(aTableId), desc(true) {}
    std::string     name;
    uint32_t        tableId;
    bool            desc;
  };

  using PropertyList = std::vector<Property>;
  //--------------------------
  
  class DBQuery {
  public:
      DBQuery(Clauses allClauses, bool allFields=true, Schema_ptr aSchema=nullptr)
      : fromTable(aSchema), all(allFields),attributeNames(), clauses(allClauses) {}

      DBQuery(const DBQuery &aQuery) : fromTable(aQuery.fromTable), all(aQuery.all),
          attributeNames(aQuery.attributeNames), clauses(aQuery.clauses) {}

      DBQuery(Clauses allClauses,StringList allAttributeNames, Schema_ptr aSchema=nullptr )
              : fromTable(aSchema), all(false), attributeNames(allAttributeNames), clauses(allClauses) {}

      DBQuery& operator=(const DBQuery aCopy) {
          fromTable = aCopy.fromTable;
          all = aCopy.all;
          attributeNames = aCopy.attributeNames;
          clauses = aCopy.clauses;
          return *this;
      }

    bool Matches(const Row& aRow) const {
      if (clauses.matches(aRow)) return true;
      return false;
    }
    StatusResult applyClauses(Table& aTable) const {
        return clauses.execute(aTable);
    };

    bool hasClauses() const { return clauses.size() > 0; }
    bool getJoin(Join& aJoin) const {
        return clauses.getJoin(aJoin);
    }
    StatusResult selectColumns(Table& aTable) const{
        if (all) return StatusResult();
        AttributeList theAttList = aTable.schema->getAttributes(attributeNames);
        if (!aTable.schema->checkContains(attributeNames)) return StatusResult(Errors::invalidAttribute);
        Schema_ptr theSchema = std::make_shared<Schema>(theAttList, fromTable->getName());
        Table outTable(aTable.getName(),theSchema);
        aTable.each([&](const Row& row) {
            outTable.addRow(std::make_shared<Row>(row.getSelectedDataRow(theAttList)));
            return true;
            });
        aTable = outTable;
        return StatusResult();
    }
    std::string getTableName() const {
        if (fromTable) return fromTable->getName();
        return "";
    }
  protected:
    Schema_ptr  fromTable;
    bool     all;
    StringList attributeNames;
    Clauses clauses;
  };

}

#endif /* DBQuery_h */


