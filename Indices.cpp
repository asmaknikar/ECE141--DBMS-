#include "Indices.hpp"
#include "Scanner.hpp"

namespace ECE141 {
    template<typename Key, typename Val>
    Index<Key,Val>::Index(ind anId): Storable('I'), 
        headerBlockId(anId),nextBlockId(0), nextIndices(nullptr), stringRepresentation("") {}
    template<typename Key, typename Val>
    Index<Key,Val>::Index(const Index& aCopy): Storable('I') {
        headerBlockId = aCopy.headerBlockId;
        nextBlockId = aCopy.nextBlockId;
        nextIndices = aCopy.nextIndices;
        stringRepresentation = aCopy.stringRepresentation;
        indices = aCopy.indices;
    }
    template<typename Key, typename Val>
    StatusResult  Index<Key, Val>::encode(std::ostream& anOutput) const {
        Block metadata(BlockType::indices_block, stringRepresentation);
        metadata.header = BlockHeader(BlockType::indices_block, headerBlockId, nextBlockId);
        StatusResult theResult(metadata.write(anOutput));
        return theResult;
    }
    template<typename Key, typename Val>
    StatusResult  Index<Key, Val>::decode(const Block& aBlock) {
        indices.clear();
        StatusResult theResult;
        if (!aBlock.header.isBlockType(blockChar)) return StatusResult(Errors::readError);
        stringRepresentation = "";
        headerBlockId = aBlock.header.tableId;
        nextBlockId = aBlock.header.nextBlockId;
        std::string thePayLoad(aBlock.payload);
        // go over each chunk of form "hashIndex:blockIndex"
        if (!eachChunk(thePayLoad, [&](std::string aPair) {
            // for some reason on linux, the filter test gets a singular blank value
            // at value 190. Seems to correspond to when key="ab" when run on windows
            ind key = Helpers::hexCharsToInd(aPair.substr(0, aPair.find(colon)));
            ind value = Helpers::hexCharsToInd(aPair.substr(aPair.find(colon) + 1));
            if (!addPair(key, value)) return false;
            return true;
            })) theResult = StatusResult(Errors::readError);
            return theResult;
    }
    template<typename Key, typename Val>
    typename Index<Key,Val>::Index_ptr Index<Key,Val>::getNextIndices(){
        return nextIndices;
    }
    template<typename Key, typename Val>
    bool Index<Key,Val>::addPair(Key aKey, Val aValue) {
        if (hasKey(aKey)) { 
            return false; 
        }
        std::string keyValPair = Helpers::indToHexChars(aKey) + colon + Helpers::indToHexChars(aValue) + fileDelimiter;
        if (stringRepresentation.length() + keyValPair.length() > kPayloadSize) {
            if(!nextIndices) nextIndices = std::make_shared<Index>(headerBlockId);
            return nextIndices->addPair(aKey, aValue);
        }
        indices.insert({aKey,aValue});
        stringRepresentation += keyValPair;
        return true;
    }
    template<typename Key, typename Val>
    bool Index<Key,Val>::hasKey(Key aKey) {
        ind count = indices.count(aKey);
        if (count == 0 && nextIndices) return nextIndices->hasKey(aKey);
        return count > 0;
    }
    template<typename Key, typename Val>
    bool Index<Key,Val>::removeIndex(Key aKey) {
        if (indices.count(aKey) == 0) {
            if (hasChild()) return nextIndices->removeIndex(aKey);
            return false;
        }
        if (indices.erase(aKey) < 0) return false;
        stringRepresentation = "";
        for (const auto& [key, val] : indices) {
            stringRepresentation += Helpers::indToHexChars(key) + colon + Helpers::indToHexChars(val) + fileDelimiter;
        }
        return true;
    }
    template<typename Key, typename Val>
    ind Index<Key,Val>::getIndexValue(Key aKey) {
        if (indices.count(aKey) == 0) {
            if (hasChild()) return nextIndices->getIndexValue(aKey);
            return -1;
        }
        return indices[aKey];
    }
    template<typename Key, typename Val>
    bool Index<Key,Val>::hasChild() const {
        if (nextIndices == nullptr) return false;
        return true;
    }
    template<typename Key, typename Val>
    bool Index<Key,Val>::getMap(BasicMap& aIndicesOut) const {
        aIndicesOut.insert(indices.begin(), indices.end());
        while (hasChild()) {
            return nextIndices->getMap(aIndicesOut);
        }
        return true;
    }
}
