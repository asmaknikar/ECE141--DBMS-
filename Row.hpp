//
//  Row.hpp
//  PA3
//
//  Created by rick gessner on 4/2/23.
//

#ifndef Row_hpp
#define Row_hpp

#include <stdio.h>
#include <utility>
#include <memory>
#include "Attribute.hpp"
#include "BasicTypes.hpp"
#include "Storage.hpp"

namespace ECE141 {

  //These really shouldn't be here...
  using RowKeyValues = std::map<const std::string, Value>;
  class Row : public Storable {
  public:

    //Row(){};
    Row(ind entityId = 0);
    Row(const Row &aRow);
   // Row(const Attribute &anAttribute); //maybe?
    
    ~Row();
    
    Row& operator=(const Row &aRow);
    bool operator<(const Row& aRow);

    //bool operator==(Row &aCopy) const;
                          
    void                set(const std::string &aKey,
                            const Value &aValue);
    void                set(const RowKeyValues& aPair);
        
    const RowKeyValues& getData() const {return data;}
    Row                 getSelectedDataRow(AttributeList aList) const ;
    std::string         getValueString(const std::string aColumn) const;
    const Value         getValue(const std::string aColumn) const;
    std::string         valueToString(Value aVal) const;
    Value               stringToValue(std::string& aString);
    virtual StatusResult  encode(std::ostream& anOutput) const;
    virtual StatusResult  decode(const Block& aBlock);
    std::string           to_string() const;
    void                setNextBlockNum(ind aBlockNum);
    void                setTable(std::string aName);
    void                setTable(ind aHashInd);
    ind                 getNextBlockNum() const;
    ind                 getTableHash() const;
    ind                 getID()const { return ID; }
    void                setID(ind anID){ ID = anID; }
    Row                 nullCopy();
    static Row      joinRows(const Row& rowA, const Row& rowB);
  protected:
    RowKeyValues        data;
    ind            ID;
    ind            tableHashID;
    ind            nextBlockNumber;
  };
  //-------------------------------------------
  using Row_ptr = std::shared_ptr <Row>;

}
#endif /* Row_hpp */

