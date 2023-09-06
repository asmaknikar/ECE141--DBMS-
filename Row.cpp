//
//  Row.cpp
//  PA3
//
//  Created by rick gessner on 4/2/22.
//
#include <sstream>
#include "Row.hpp"
#include "Helpers.hpp"

namespace ECE141 {
  
  Row::Row(ind schemaId): Storable('D'), ID(0), tableHashID(schemaId), nextBlockNumber(0) {}
  Row::Row(const Row &aRow) : Storable('D') {
      data = aRow.data;
      ID = aRow.ID;
      nextBlockNumber = aRow.nextBlockNumber;
      tableHashID = aRow.tableHashID;
      /**this=aRow;*/
  }
  Row Row::joinRows(const Row& rowA, const Row& rowB) {
      Row theOut;
      theOut.set(rowA.getData());
      // check here if they have overlap
      theOut.set(rowB.getData());
      return theOut;
  }
  Row Row::nullCopy() {
      Row aCopy;
      std::string nullString = "NULL";
      for (auto& [key, value] : data) {
          aCopy.set(key, nullString);
      }
      return aCopy;
  }
  Row::~Row() {}
  Row& Row::operator=(const Row &aRow) {
      data = aRow.data;
      ID = aRow.ID;
      nextBlockNumber = aRow.nextBlockNumber;
      tableHashID = aRow.tableHashID;
      return *this;
  }
  Row Row::getSelectedDataRow(AttributeList aList) const {
      Row outRow;
      for (auto & att : aList) {
          outRow.set(att.getName(), getValue(att.getName()));
      }
      return outRow;
  }
  void Row::setNextBlockNum(ind aBlockNum) {nextBlockNumber = aBlockNum;}
  ind  Row::getNextBlockNum() const{ return nextBlockNumber; }
  ind  Row::getTableHash() const{ return tableHashID; }
  void Row::setTable(std::string aName) { tableHashID = Helpers::hash(aName); }
  void Row::setTable(ind aHashInd) { tableHashID = aHashInd; }
  //bool Row::operator==(Row &aCopy) const {return false;}
  
  StatusResult  Row::encode(std::ostream& anOutput) const {
      Block rowBlock = Block(BlockType::data_block, to_string());
      rowBlock.header.tableId = tableHashID;
      rowBlock.header.nextBlockId = nextBlockNumber;
      return rowBlock.write(anOutput);
  }

  StatusResult  Row::decode(const Block& aBlock) {
      if (!aBlock.header.isBlockType(blockChar)) return StatusResult(Errors::readError);
      nextBlockNumber = aBlock.header.nextBlockId;
      tableHashID = aBlock.header.tableId;
      std::stringstream ss(aBlock.payload);
      ss >> ID;
      std::string thePayload;
      std::getline(ss,thePayload);
      eachChunk(thePayload, [&](std::string aChunk) {
          std::stringstream ss(aChunk);
          std::string theKey, theValueString;
          ss >> theKey;
          ss >> theValueString;
          Value theValue = stringToValue(theValueString);
          data[theKey] = theValue;
          return true; 
          }
      );
      return StatusResult();
  }

  std::string Row::valueToString(Value aVal) const{
      if (std::holds_alternative<std::string>(aVal)) {
          return "S:"+std::get<std::string>(aVal);
      }
      if (std::holds_alternative<int>(aVal)) {
          return "I:" + std::to_string(std::get<int>(aVal));
      }
      if (std::holds_alternative<float>(aVal)) {
          return "F:" + std::to_string(std::get<float>(aVal));
      }
      if (std::holds_alternative<bool>(aVal)) {
          return std::get<bool>(aVal) ? "B:true" : "B:false";
      }
      return "";
  }
  Value Row::stringToValue(std::string& aString) { // format: "Type_char:value"
      if (aString.length() < 2) return "";
      std::string valString = aString.substr(2, aString.length() - 2);
      if (aString[0] == 'S') {
          return valString;
      }
      if (aString[0] == 'I') {
          return std::stoi(valString);
      }
      if (aString[0] == 'F') {
          return std::stof(valString);
      }
      if (aString[0] == 'B') {
          return valString == "true" ? true : false;
      }
      return "";
  }

  std::string Row::to_string() const{
      std::string out = std::to_string(ID)+" ";
      for (auto const& [key, value] : data) {
          out += key + " " + valueToString(value) + fileDelimiter;
      }
      return out;
  }

  //STUDENT: What other methods do you require?
  std::string Row::getValueString(std::string aColumn) const {
      Value val = data.at(aColumn);
      if (std::holds_alternative<std::string>(val)) return std::get<std::string>(val);
      if (std::holds_alternative<int>(val)) return std::to_string(std::get<int>(val));
      if (std::holds_alternative<bool>(val)) return std::to_string(std::get<bool>(val));
      else return std::to_string(std::get<float>(val));
  }
  const Value  Row::getValue(const std::string aColumn) const{
      return data.at(aColumn);
  }
  void Row::set(const std::string &aKey,const Value &aValue) {
    data[aKey] = aValue;
  }
  void Row::set(const RowKeyValues& aPair) {
      for (auto const& [key, value] : aPair) {
          data[key] = value;
      }
  }
  
    
}
