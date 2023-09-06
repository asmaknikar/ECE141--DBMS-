//
//  Schema.cpp
//  PA3
//
//  Created by rick gessner on 3/2/23.
//

#include <set>
#include "Schema.hpp"
#include "Table.hpp"
#include "TableView.hpp"
#include "Config.hpp"

namespace ECE141 {
    static AttributeList getSchemaViewConfig() {
        AttributeList theViewAttList{
                Attribute("Field", DataTypes::varchar_type, 20),
                Attribute("Type", DataTypes::varchar_type, 20),
                Attribute("Null", DataTypes::varchar_type, 10),
                Attribute("Key", DataTypes::varchar_type, 10),
                Attribute("Default", DataTypes::varchar_type, 10),
                Attribute("Extra", DataTypes::varchar_type, 40),

        };
        return theViewAttList;
    }
    std::map<DataTypes, std::string> theDefaultMap{
              {DataTypes::varchar_type,"NULL"},
              {DataTypes::bool_type,"FALSE"},
              {DataTypes::datetime_type,"NULL"},
              {DataTypes::float_type,"0.0"},
              {DataTypes::int_type,"NULL"},
    };

  Schema::Schema(const std::string aName) : Storable('S'),
      name(aName),hashedName(getHash(aName)),startBlock(0) {}
  Schema::Schema(AttributeList aList, const std::string aName, bool autoKeys) :
      Storable('S'), attributes(aList),name(aName),hashedName(getHash(aName)), startBlock(0) {
      OptString thePrimaryKey = getAutoIncrementHeader();
      if (!thePrimaryKey && autoKeys){
          Attribute basicAutoInc("id", DataTypes::int_type,10);
          basicAutoInc.setAutoIncrement(true);
          attributes.push_back(basicAutoInc);
      }

  }
  Schema::Schema(const Schema &aCopy): Storable('S') {
    name=aCopy.name;
    attributes = aCopy.attributes;
    hashedName = aCopy.hashedName;
    startBlock = aCopy.startBlock;
  }

  Schema::~Schema() {
    //std::cout << "~Schema()\n";
  }
  Attribute Schema::getAttribute(std::string aAttributeName) {
      Attribute theAttribute;
      for (auto& a : attributes) {
          if (a.getName() == aAttributeName) theAttribute = a;
      }
      return theAttribute;
  }

  Schema Schema::joinSchemas(Schema& aSchemaA, Schema& aSchemaB) {
      Schema theNewSchema(aSchemaA.attributes,aSchemaA.getName()+" on "+ aSchemaB.getName());
      theNewSchema.attributes.insert(theNewSchema.attributes.end(), aSchemaB.attributes.begin(), aSchemaB.attributes.end());
      return theNewSchema;
  }
  AttributeList Schema::getAttributes(StringList aAttributeNames) {
      AttributeList theAttributes;
      for (auto const& aName : aAttributeNames) {
          for (auto& a : attributes) {
              if (a.getName() == aName) theAttributes.push_back(a);
          }
      }
      return theAttributes;
  }
  bool Schema::setStartBlock(ind aBlockNum) {
      startBlock = aBlockNum;
      return true;
  }
  std::string Schema::to_string() const {
      std::string out(name + " " + std::to_string(next_primary_key) + " ");
      //out.str(name + fileDelimiter);
      for (const Attribute& a : attributes) {
          out += a.to_string() + fileDelimiter;
      }
      return out; // add map of their row indices
  }
  StatusResult  Schema::encode(std::ostream& anOutput) const{
      Block schemaBlock(BlockType::schema_block, to_string());
      schemaBlock.header.tableId = hashedName;
      schemaBlock.header.nextBlockId = startBlock;
      return schemaBlock.write(anOutput); 
  }

  StatusResult  Schema::decode(const Block& aBlock) {
      StatusResult theResult;
      if (!aBlock.header.isBlockType(blockChar)) return StatusResult(Errors::readError);
      hashedName = aBlock.header.tableId;
      startBlock = aBlock.header.nextBlockId;
      std::stringstream ss(aBlock.payload);
      std::string theAttributes;
      ss >> name;
      ss >> next_primary_key;
      std::getline(ss, theAttributes);
      if (eachChunk(theAttributes, [&](std::string aChunk) {
          std::stringstream ss(aChunk);
          Attribute a = Attribute(ss);
          if (!a.isValid()) return false;
          attributes.push_back(a);
          return true;
          })
          ) theResult = StatusResult(Errors::readError);
          return theResult;
  }

  bool Schema::getHeaderNameAtCol(ind aColumn,std::string& aElement) {
      if (aColumn < attributes.size()) { aElement = attributes[aColumn].getName(); return true; }
      return false;
  }
  OptString Schema::getAutoIncrementHeader() const {
      for (auto& a : attributes) {
          if (a.isAutoIncrement()) return a.getName();
      }
      return std::nullopt;
  }
  std::vector<uint32_t> Schema::getColumnWidths() {
      std::vector<uint32_t> widths;
      for (Attribute a : attributes) {
          uint32_t size{ a.getDataSize() };
          widths.push_back(size);
      }
      return widths;
  }

  const StatusResult Schema::createView(ViewListener &aView,double aDuration) {
    AttributeList theViewAttList = getSchemaViewConfig();
    
    Schema_ptr s = std::make_shared<Schema>(theViewAttList,"SchemaView");
    Table theSchemaTabularViewTable = Table("SchemaView", s);
    for(Attribute attribute:attributes){
      Row_ptr theRow = std::make_shared<Row>();
      std::string theExtra = "";
      if(attribute.isAutoIncrement()) theExtra+="auto_increment ";
      if(attribute.isPrimaryKey()) theExtra+="primary key ";
      if(theExtra.size() > 0) theExtra.pop_back();
      theRow->set("Field", attribute.getName());
      theRow->set("Type", Helpers::dataTypeToString(attribute.getType())+
              ((attribute.getType()==DataTypes::varchar_type)?("("+std::to_string(attribute.getSize())+")"):""));
      theRow->set("Null", attribute.isNullable()?"YES":"NO");
      theRow->set("Key", attribute.isPrimaryKey()?"YES":"");
      theRow->set("Default", theDefaultMap[attribute.getType()]);
      theRow->set("Extra", theExtra);
      theSchemaTabularViewTable.addRow(std::move(theRow));
    };
    TableView theTableView = TableView(theSchemaTabularViewTable,aDuration);
    aView(theTableView);
    return StatusResult(Errors::noError);
    }

  bool Schema::checkContains(StringList aRowHeaderList) {
    std::set<std::string> attributeNameSet;
    std::transform(attributes.begin(),
                   attributes.end(),
                   std::inserter(attributeNameSet,attributeNameSet.begin()),
                   [](Attribute anAttribute){return anAttribute.getName();});
    for(const std::string& rowHeader: aRowHeaderList){
      if(attributeNameSet.find(rowHeader)==attributeNameSet.end()){
        return false;
      }
    }
    return true;
  }
  bool Schema::getRemainingDefaults(StringList aRowHeaderList, RowKeyValues& aKeyValuePair) {
      for (auto& attribute : attributes) {
          if (Helpers::in_array(aRowHeaderList, attribute.getName())) continue;
          else if (attribute.isAutoIncrement()) continue;
          else if (!attribute.isNullable()) return false;
          else aKeyValuePair.insert({ attribute.getName(), attribute.getDefaultValue()});
      }
      return true;
  }

//  bool Schema::validateAndImplementSchema(Rows aRowList) {
//    for(Attribute attribute:attributes){
//      int maxIfAutoIncrement = false?0:-1;//TODO get the autoincrement prev value if autoincrement
//      for(const Row_ptr row: aRowList){
//        Value theValue = row->getValue(attribute.getName());
//        if(attribute.isAutoIncrement()) row->set(attribute.getName(),++maxIfAutoIncrement);
////        if(attribute.getIsDefault() || true ) row.set(attribute.getName(),attribute.getDefaultValue());//TODO check null value
//        if(attribute.isPrimaryKey() || true ) return false;//TODO add condn to check if value is there or not
//      }
//    }
//
//    return true;
//  }
}
