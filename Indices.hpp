#pragma once
#include <map>
#include <memory>
#include "Storage.hpp"
#include "Helpers.hpp"
namespace ECE141 {
    

    template<typename Key, typename Val>
    class Index : public Storable {
    public:
        using IndexVisitor = std::function<bool(Key, Val)>;
        using BasicMap = std::map<Key, Val>;
        using Index_ptr = std::shared_ptr<Index<Key, Val>>;

        Index(ind anId = 0);
        Index(const Index& aCopy);
        Index& operator=(const Index& aCopy){
            headerBlockId = aCopy.headerBlockId;
            nextIndices = aCopy.nextIndices;
            indices = aCopy.indices; 
            stringRepresentation = aCopy.stringRepresentation;
            return *this;
        }
        ~Index(){};
        StatusResult  encode(std::ostream& anOutput) const;
        StatusResult  decode(const Block& aBlock);

        bool          hasChild()const;
        bool          addPair(Key aKey, Val aValue);
        bool          removeIndex(Key aKey);

        Index_ptr     getNextIndices();
        void          setNextBlockId(ind aBlockNum) { nextBlockId = aBlockNum; }
        void          setNextIndices(Index_ptr anIndex) { nextIndices = anIndex; }
        ind           getNextBlockId() { return nextBlockId; }
        ind           getHeaderId() { return headerBlockId; }
        bool          getMap(BasicMap& aIndicesOut) const;
        ind           getIndexValue(Key aKey);
        bool          hasKey(Key aKey);

    private:
        ind            headerBlockId;
        ind            nextBlockId;
        BasicMap       indices;
        Index_ptr      nextIndices;
        std::string    stringRepresentation;

    };
    using IndIndex = Index<ind, ind>;
    using IndIndex_ptr = std::shared_ptr<IndIndex>;
    
}

