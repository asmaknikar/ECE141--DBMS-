//
//  BlockIO.hpp
//  PA2
//
//  Created by rick gessner on 2/27/23.
//

#ifndef BlockIO_hpp
#define BlockIO_hpp

#include <stdio.h>
#include <iostream>
#include <fstream>
#include <variant>
#include <functional>
#include "Errors.hpp"

namespace ECE141 {

  enum class BlockType {
    metadata_block='M',
    data_block='D',
    free_block='F',
    schema_block='S',
    unknown_block='U',
    indices_block='I',
    first_row_block='R',
  };


  //a small header that describes the block...
  struct BlockHeader {
   
    BlockHeader(BlockType aType=BlockType::data_block, int aTableId=0,int aBlockId=0)
      : type(static_cast<char>(aType)), tableId(aTableId), nextBlockId(aBlockId) {}

    BlockHeader(const BlockHeader &aCopy) {
      *this=aCopy;
    }
        
    void empty() {
      type=static_cast<char>(BlockType::free_block);
    }
    static bool isBlockType(char aChar) {
        switch (toupper(aChar)) {
        case 'M': return true;
        case 'D': return true;
        case 'F': return true;
        case 'S': return true;
        case 'I': return true;
        case 'R': return true;
        default:  return false;
        }
    }
    BlockHeader& operator=(const BlockHeader &aCopy) {
      type=aCopy.type;
      tableId = aCopy.tableId;
      nextBlockId = aCopy.nextBlockId;
      return *this;
    }
   
    char    type;     //char version of block type
    ind     tableId;  //which table it belongs to 
    ind     nextBlockId;
    //other properties?
  };

  const size_t kBlockSize = 1024;
  const size_t kBlockHeaderSize = 17;
  const size_t kPayloadSize = kBlockSize - kBlockHeaderSize;
  
  //block .................
  class Block {
  public:
    Block(BlockType aType=BlockType::data_block, std::string aString="");
    Block(const Block &aCopy);
    std::string extra();
    Block& operator=(const Block &aCopy);
   
    StatusResult write(std::ostream &aStream);
    BlockHeader   header;
    char          payload[kPayloadSize];
  };

  //------------------------------

   struct CreateFile {
    operator std::ios_base::openmode() {
      return std::ios::binary | std::ios::in | std::ios::out | std::ios::trunc;
    }
  }; //tags for db-open modes...
  struct OpenFile {
    operator std::ios_base::openmode() {
      return std::ios::binary | std::ios::in | std::ios::out;
    }
  };

  using AccessMode=std::variant<CreateFile, OpenFile>;

  class BlockIO {
  public:
    BlockIO(const std::string &aName, AccessMode aMode);

    uint32_t              getBlockCount();
    
    virtual StatusResult  readBlock(ind aBlockNumber, Block &aBlock);
    virtual StatusResult  writeBlock(ind aBlockNumber, Block &aBlock);
    
  protected:
    std::fstream stream;
    static unsigned int blocks;
  };

}


#endif /* BlockIO_hpp */
