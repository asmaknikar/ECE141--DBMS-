//
//  Storage.cpp
//  PA2
//
//  Created by rick gessner on 2/27/23.
//


#include <sstream>
#include <cmath>
#include <cstdlib>
#include <optional>
#include <cstring>
#include <iostream>
#include "Storage.hpp"
#include "Config.hpp"

using namespace std;
namespace ECE141 {
    bool Storable::eachChunk(std::string aPayload, ChunkVisitor aVisitor) {
        size_t firstDelim = 0;
        size_t secondDelim = firstDelim + 1;
        while ((secondDelim = aPayload.find(fileDelimiter, firstDelim + 1)) > firstDelim
            && secondDelim != std::string::npos) {
            std::string chunk = aPayload.substr(firstDelim, secondDelim - firstDelim);
            if (!aVisitor(chunk)) return false;
            firstDelim = secondDelim + 1;
        }
        return true;
    }
  // USE: ctor ---------------------------------------

  Storage::Storage(const std::string &aName, AccessMode aMode)
    : BlockIO(aName, aMode), name(aName) {
  }
  Storage::Storage(const Storage& aCopy): 
      BlockIO(aCopy.name, OpenFile()),name(aCopy.name) {
  }
  // USE: dtor ---------------------------------------
  Storage::~Storage() {
  }
  StatusResult Storage::save(const Storable& aStorable, ind aStartPos) {
      if (Config::useCache(CacheType::block)) blockCache.remove(aStartPos);
      stream.clear();
      StatusResult theResult;
      stream.seekp(aStartPos * kBlockSize, ios::beg);
      theResult = aStorable.encode(stream);
      return theResult;
  }

  StatusResult Storage::load(Storable& aStorable, ind aStartBlockNum) {
      StatusResult theResult;
      Block b;
      if (Config::useCache(CacheType::block) && blockCache.contains(aStartBlockNum)) {
          b = blockCache.get(aStartBlockNum);
      }
      else {
          stream.seekp(aStartBlockNum* kBlockSize + sizeof(BlockHeader));
          if (stream.eof()) theResult.error = Errors::eofError;
          if(theResult) theResult = readBlock(aStartBlockNum, b);
          if (Config::useCache(CacheType::block)) blockCache.add(aStartBlockNum, b);
      }
      if (theResult) aStorable.decode(b);
      return theResult;
  }

  bool Storage::each(const BlockVisitor &aVisitor) {
      stream.clear(); stream.seekg(0);
      int numBlocks = getBlockCount();
      Block b;
      for (int i = 0; i <numBlocks ;i++) {
          if (Config::useCache(CacheType::block) && blockCache.contains(i)) {
              if (!aVisitor(blockCache.get(i), i)) return false;
          }
          else if (readBlock(i, b)) {
              if (!aVisitor(b, i)) return false;
          }
          else return false;
      }
    return true;
  }
  bool Storage::linkedListEach(ind aStartBlock, const BlockVisitor& aVisitor) {
      bool theResult = aStartBlock>0;// cant be the database block
      Block b;
      ind currBlock = aStartBlock;
      while(theResult  && currBlock > 0) {
          if (Config::useCache(CacheType::block) && blockCache.contains(currBlock)) {
              theResult = aVisitor(blockCache.get(currBlock), currBlock);
          }
          else if (readBlock(currBlock, b)) theResult = aVisitor(b, currBlock);
          else theResult = false;
          currBlock = b.header.nextBlockId;
      }
      return theResult;
  }
  bool Storage::indexValueEach(const std::map<ind, ind>& anIndex, const BlockVisitor& aVisitor) {
      bool theResult = true;
      if (theResult) {
          Block b;
          for (const auto& [_, blockNum] : anIndex) {
              if (Config::useCache(CacheType::block) && blockCache.contains(blockNum)) {
                  if (!(theResult = aVisitor(blockCache.get(blockNum), blockNum))) break;
              }
              else if (readBlock(blockNum, b)) {
                  if (!(theResult = aVisitor(b, blockNum))) break;
              }
              else theResult = false;
          }
      }
      return theResult;
  }
  
  ind Storage::getFreeBlock() {
      ind theBlockNum = 2; // leave first two blocks reserved
      if (freeBlocks.size() == 0) { // if we dont have the free block buffer,load it
          stream.clear();
          stream.seekg(theBlockNum * kBlockSize);
          BlockHeader header;
          while (stream >> header.type)
          {
              if (header.type == char(BlockType::free_block)) freeBlocks.push_back(theBlockNum);
              theBlockNum++;
              stream.seekg(theBlockNum * kBlockSize);
          }
          freeBlocks.push_back(theBlockNum);
      }
      // if its block at the end of the file, choose the next block as free too
      // last element should always be end of file block
      if (freeBlocks.size() == 1) freeBlocks.push_back(freeBlocks.back() + 1);
      theBlockNum = freeBlocks.front();
      freeBlocks.pop_front();
      
      return theBlockNum;
      
  }
  bool Storage::returnFreeBlock(ind aBlockNum) {
      if (aBlockNum < freeBlocks.back()) {
          stream.seekg(aBlockNum);
          BlockHeader header;
          if (stream >> header.type) {// if its a written block
              if (!(header.type == (char)BlockType::free_block))
                  return false;
          } 
          freeBlocks.push_front(aBlockNum);
          return true;
      }
      return false;
  }
  StatusResult Storage::markBlockAsFree(ind aPos) {
      if (Config::useCache(CacheType::block) && blockCache.contains(aPos)) blockCache.remove(aPos);
      if (aPos >= getBlockCount()) return StatusResult(Errors::seekError);
      Block b(BlockType::free_block);
      StatusResult theResult;
      if ((theResult = writeBlock(aPos, b))) freeBlocks.push_front(aPos);
      return theResult;
  }
 
}

