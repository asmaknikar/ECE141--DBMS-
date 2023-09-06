//
//  Storage.hpp
//  PA2
//
//  Created by rick gessner on 2/27/23.
//

#ifndef Storage_hpp
#define Storage_hpp

#include <string>
#include <fstream>
#include <iostream>
#include <deque>
#include <stack>
#include <optional>
#include <memory>
#include <functional>
#include "BlockIO.hpp"
#include "Errors.hpp"
#include "LRUCache.hpp"

namespace ECE141 {

  const ind kNewBlock=-1; 

  using ChunkVisitor = std::function<bool(std::string)>;
  
  class Storable {
  public:
    Storable(char aBlockChar):blockChar(aBlockChar){};
    virtual StatusResult  encode(std::ostream &anOutput) const=0;
    virtual StatusResult  decode(const Block&b) =0; 
    inline static const std::string fileDelimiter = ";";
    //virtual void operator>>(Storable aStorable);
    bool eachChunk(std::string aPayload, ChunkVisitor aVisitor);
    char blockChar;
  };

  using BlockVisitor = std::function<bool(const Block&, ind)>;
  using BlockList = std::deque<ind>;

  // USE: Our storage manager class...
  class Storage : public BlockIO {
  public:
        
    Storage(const std::string &aName, AccessMode aMode);
    Storage(const Storage& aCopy);
    //Storage & operator=(const Storage & aCopy);
    ~Storage();

    bool each(const BlockVisitor &aVisitor);
    bool linkedListEach(ind aStartBlock, const BlockVisitor& aVisitor);
    bool indexValueEach(const std::map<ind, ind>& anIndex, const BlockVisitor& aVisitor); 
    
    StatusResult save(const Storable &aStorable, ind aStartPos=kNewBlock);
    StatusResult load(Storable& aStorable, ind aStartBlockNum);

    StatusResult markBlockAsFree(ind aPos);
    ind          getFreeBlock(); //pos of next free (or new)...
    bool         returnFreeBlock(ind aBlockNum);

  protected:
      std::string name;
      friend class Database;
      std::deque<ind> freeBlocks;
      LRUCache<ind,Block> blockCache; // block number -> block
  };
  using Storage_ptr = std::unique_ptr<Storage>;

}


#endif /* Storage_hpp */
