//
//  Attribute.hpp
//  PA3
//
//  Created by rick gessner on 4/18/22.
//  Copyright © 2023 rick gessner. All rights reserved.
//
#include <sstream>
#include "Attribute.hpp"
#include "Helpers.hpp"

namespace ECE141 {


  Attribute::Attribute(DataTypes aType):
      name(""), 
      type(aType), 
      size(10),
      auto_increment(false),
      primary_key(false),
      nullable(true),
      isDefault(false) {}
 
  Attribute::Attribute(std::string aName, DataTypes aType, uint32_t aSize):
      name(aName), type(aType),size(aSize),
      auto_increment(false),
      primary_key(false),
      nullable(true),
      isDefault(false) {
  }
  Attribute::Attribute(std::stringstream& aPayload){
      char typeC;
      aPayload >> name >> typeC >> size >> auto_increment >> primary_key >> nullable;
      type = DataTypes(typeC);
  }

  Attribute::Attribute(Command& anAttributeCommand) {
    type = DataTypes::no_type; nullable = true; auto_increment = false;primary_key= false;
    bool notFlag = false;
    if (anAttributeCommand.size() > 8) return;
    int numVarTypes = 0;
    for (auto& token : anAttributeCommand) {
        if (Helpers::isDatatype(token.keyword)) numVarTypes++;
    }
    if (numVarTypes != 1) return;
    std::vector<Token>::iterator theAttributeCommandIterator = anAttributeCommand.begin();
    if (anAttributeCommand.size() < 2) return;
    name = theAttributeCommandIterator++->data;
    if (Helpers::isDatatype(theAttributeCommandIterator->keyword))
      type = Helpers::keywordToDatatype(theAttributeCommandIterator++->keyword);
    if(type == DataTypes::varchar_type){
      theAttributeCommandIterator++;
      size = stoi(theAttributeCommandIterator++->data);
      theAttributeCommandIterator++;
    }
    size = getDataSize(); // get datasizes for non var char
      for(;theAttributeCommandIterator!=anAttributeCommand.end();theAttributeCommandIterator++){
          switch (theAttributeCommandIterator->keyword) {
            case Keywords::not_kw:
              notFlag = true;
              break;
            case Keywords::null_kw:
              if(notFlag){
                nullable = false;
                notFlag = false;
              } else nullable = true;
              break;
            case Keywords::auto_increment_kw:
              auto_increment = true;
              break;
            case Keywords::primary_kw:
              primary_key = true;
              break;
            default:
                break;
          }
      }

  }
  Attribute::Attribute(const Attribute &aCopy)  {
    name=aCopy.name;
    type = aCopy.type;
    size = aCopy.size;
    auto_increment = aCopy.auto_increment;
    primary_key = aCopy.auto_increment;
    nullable = aCopy.nullable;
  }
 
  Attribute::~Attribute()  {
  }
  bool Attribute::isValid(){
      return name != "" && type != DataTypes::no_type;
  }

  std::string   Attribute::to_string() const {
      std::stringstream sStream;
      sStream << name << " " << char(type) << " " << size << " " << auto_increment << " " << primary_key << " " << nullable;
      return sStream.str();
  }
  uint32_t Attribute::getDataSize() {
      switch (type) {
      case DataTypes::datetime_type: return 15;
      case DataTypes::float_type: return 10;
      case DataTypes::int_type: return 10;
      default: return size;
      }
  }
  ValueOpt Attribute::makeValue(std::string aValueStr) {
      ValueOpt aVal = std::nullopt;
      if (type == DataTypes::varchar_type) {
          return aVal = aValueStr;
      }
      if (type == DataTypes::float_type) {
          return aVal = std::stof(aValueStr);
      }
      if (type == DataTypes::int_type) {
          return aVal = std::stoi(aValueStr);
      }
      if (type == DataTypes::float_type) {
          return aVal = (aValueStr == "true" ? true : false);
      }
      return aVal;
  }
//    Value Attribute::getDefaultValue() const {
//      return defaultValue;
//    }


}
