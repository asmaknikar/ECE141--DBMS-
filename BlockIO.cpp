//
//  BlockIO.cpp
//  PA2
//
//  Created by rick gessner on 2/27/23.
//

#include <cstring>
#include <bitset>
#include "BlockIO.hpp"
#include "Config.hpp"
#include "Helpers.hpp"
#include  <iomanip>

namespace ECE141 {
  Block::Block(BlockType aType, std::string aString) {
        std::memcpy(payload, aString.c_str(), aString.size()+1);
        header = BlockHeader(aType);
  }

  Block::Block(const Block &aCopy) {
    *this=aCopy;
  }

  Block& Block::operator=(const Block &aCopy) {
    std::memcpy(payload, aCopy.payload, kPayloadSize);
    header=aCopy.header;
    return *this;
  }
  std::string Block::extra() {
      switch (header.type) {
          case int(BlockType::metadata_block) : {
              std::stringstream thePayload(payload);
              std::string name;
              thePayload >> name;
              return "DB: " + name;
          }
          case int(BlockType::free_block) : return "Empty";
          case int(BlockType::data_block) : {
              if (header.nextBlockId != 0) {
                  return "Next Block: " + std::to_string(header.nextBlockId);
              }
              return "";
          }
          case int(BlockType::indices_block) : {
              std::string indices = std::string(payload);
              return std::to_string(std::count(indices.begin(), indices.end(), ';'))+ " Items";
          }
          case int(BlockType::schema_block) : {
              std::string row = std::string(payload);
              return std::to_string(std::count(row.begin(), row.end(), ';')) + " Attributes, First Block: " + 
                  std::to_string(header.nextBlockId);
          }
          default: return "";
      }
  }
  StatusResult Block::write(std::ostream &aStream) {
     /* std::stringstream buffer;
      aStream.clear();
      aStream.seekp(std::ios::end);
      auto x = aStream.tellp();
      aStream.seekp(0);
      buffer << aStream.rdbuf();*/
      if (aStream << header.type << std::setfill('0') << std::right << std::setw(8) << Helpers::indToHexChars(header.nextBlockId)
          << std::setw(8) << Helpers::indToHexChars(header.tableId) << std::setfill(' ')
          << std::left << std::setw(kPayloadSize) << payload)
          return StatusResult();
      return StatusResult(Errors::writeError);
  }

  //---------------------------------------------------
    struct modeToInt {
        std::ios_base::openmode operator()(CreateFile &aVal) const {return static_cast<std::ios_base::openmode>(aVal);}
        std::ios_base::openmode operator()(OpenFile &aVal) const {return static_cast<std::ios_base::openmode>(aVal);}
    };

    BlockIO::BlockIO(const std::string &aName, AccessMode aMode) {
        std::string thePath = Config::getDBPath(aName);
        auto  theMode=std::visit(modeToInt(), aMode);
        stream.clear(); // Clear flag just-in-case...
        stream.open(thePath.c_str(), theMode); //force truncate if...
        stream.close();
        stream.open(thePath.c_str(), theMode);
    }

  // USE: write data to file given a block (after seek) ---------------------------------------
  StatusResult BlockIO::writeBlock(ind aBlockNum, Block &aBlock) {
    stream.clear();
    stream.seekp(aBlockNum*kBlockSize,std::ios::beg);
    if (stream.eof()) {
        stream.clear();
        stream.seekp(0, std::ios::end);
    }
    aBlock.header.nextBlockId = aBlockNum;
    return aBlock.write(stream);
  }

  // USE: write data to a given block (after seek) ---------------------------------------
  StatusResult BlockIO::readBlock(ind aBlockNumber, Block &aBlock) {
    stream.clear();
    stream.seekg(aBlockNumber*kBlockSize,std::ios::beg);
    char buffer[kBlockSize];
    BlockHeader header;
    StatusResult theResult;
    if(!stream.read(buffer, kBlockSize)) return StatusResult(Errors::readError);
    aBlock.header.type = buffer[0];
    if (!BlockHeader::isBlockType(header.type)) return StatusResult(Errors::readError);
    char temp[9]={0};
    memcpy(temp, buffer + 1, 8);
    temp[8] = '\0';
    std::string s1(temp);
    aBlock.header.nextBlockId = Helpers::hexCharsToInd(s1);
    memcpy(temp, buffer + 9, 8);
    temp[8] = '\0';
    std::string s2(temp);
    aBlock.header.tableId = Helpers::hexCharsToInd(s2);
    memcpy(aBlock.payload, buffer + kBlockHeaderSize, kPayloadSize);
    return StatusResult();
  }

  // USE: count blocks in file ---------------------------------------
  uint32_t BlockIO::getBlockCount()  {
      stream.clear();
      stream.seekg(0, std::ios::beg);
      int i = 0;
      BlockHeader header(BlockType::unknown_block);
      while (stream >> header.type)
      {
          if (!BlockHeader::isBlockType(header.type)) break;
          i++;
          stream.seekg(i * kBlockSize, std::ios::beg);
          header.type = char(BlockType::unknown_block);
      }
      
    return i;
  }

}
