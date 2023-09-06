//
//  Schema.hpp
//  PA3
//
//  Created by rick gessner on 3/18/23.
//  Copyright © 2023 rick gessner. All rights reserved.
//

#ifndef Schema_hpp
#define Schema_hpp

#include <memory>
#include <string>
#include <unordered_map>

#include "Attribute.hpp"
#include "Errors.hpp"
#include "Storage.hpp"
#include "Indices.hpp"
#include "Row.hpp"
namespace ECE141 {
  class Schema : public Storable {
  public:
                          Schema(const std::string aName);
                          Schema(AttributeList aList, const std::string aName, bool autoKeys = false);
                          Schema(const Schema &aCopy);
    
                          ~Schema();
    std::string           to_string() const;
    Attribute             getAttribute(std::string aAttributeName);
    AttributeList         getAttributes(StringList aAttributeNames);
    const std::string&    getName() const {return name;}
    const ind&            getHashedName() const { return hashedName; }
    const StatusResult    createView(ViewListener &aView, double aDuration);
    bool                  setStartBlock(ind aBlockNum);
    std::vector<uint32_t> getColumnWidths();
    bool                  getHeaderNameAtCol(ind aColumn, std::string& aElement);
    OptString             getAutoIncrementHeader() const;
    ind                   getStartBlock() { return startBlock; }
    // Storable
    virtual StatusResult  encode(std::ostream& anOutput) const;
    virtual StatusResult  decode(const Block& aBlock);

    bool                  checkContains(StringList aRowHeaderList);
    bool                  getRemainingDefaults(StringList aRowHeaderList, RowKeyValues& aKeyValuePair);
    ind                   getNextAutoIncrement() { return next_primary_key++; }
    
    static Schema         joinSchemas(Schema& aSchemaA, Schema& aSchemaB);
    static ind            getHash(std::string aName) { return Helpers::hash(aName); }
    
  protected:
    AttributeList   attributes;
    std::string     name;
    ind hashedName;
    ind startBlock;
    ind next_primary_key = 1;
    //how will you manage creation of primary keys?

  };
  using Schema_ptr = std::shared_ptr<Schema>;
  
}
#endif /* Schema_hpp */
