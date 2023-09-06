//
//  Attribute.hpp
//  PA3
//
//  Created by rick gessner on 4/18/22.
//  Copyright © 2023 rick gessner. All rights reserved.
//

#ifndef Attribute_hpp
#define Attribute_hpp

#include <stdio.h>
#include <string>
#include <vector>
#include <optional>
#include "keywords.hpp"
#include "BasicTypes.hpp"
#include "Tokenizer.hpp"
#include "Handler.hpp"

namespace ECE141 {
    using KeywordList = std::vector<Keywords>;

    class Attribute {
      protected:
        std::string   name;
        DataTypes     type;
        uint32_t size = 15;
        bool auto_increment=false;
        bool primary_key=false;
        bool nullable=true;
        bool isDefault=false;
        Value defaultValue = "";
        //what other properties do we need?

      public:
          
            Attribute(DataTypes aType=DataTypes::no_type);
            Attribute(Command& aCommand);
            Attribute(std::stringstream& aPayload);
            Attribute(std::string aName, DataTypes aType, uint32_t aSize=0);
            Attribute(const Attribute &aCopy);
            ~Attribute();
        
            const std::string&  getName() const {return name;}
            DataTypes           getType() const {return type;}
            uint32_t            getSize() const {return size;}
            Value               getDefaultValue() const { return defaultValue;}
            bool                getIsDefault() const { return isDefault; }
            bool                isAutoIncrement() const { return auto_increment; }
            bool                isPrimaryKey() const { return primary_key; }
            bool                isNullable() const { return nullable; }
            

            void setAutoIncrement(bool aBool) { auto_increment = aBool; }
            void setPrimaryKey(bool aBool) { primary_key = aBool; }
            void setNullable(bool aBool){ nullable = aBool;}
            void setDataType(DataTypes aDatatype){ type = aDatatype; }
            void setSize(int aSize) { size = aSize; }

            ValueOpt makeValue(std::string aValue);
            std::string         to_string() const;

    
        bool                isValid(); //is this  valid?
        uint32_t            getDataSize();
      };
  
  using AttributeOpt = std::optional<Attribute>;
  typedef  std::vector<Attribute> AttributeList;

}


#endif /* Attribute_hpp */
