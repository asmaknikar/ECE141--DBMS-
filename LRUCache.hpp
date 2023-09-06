//
//  LRUCache.hpp
//
//  Created by rick gessner on 5/27/23.
//

#ifndef LRUCache_h
#define LRUCache_h

#include <unordered_map>
#include <list>
#include <cstddef>
#include <stdexcept>

namespace ECE141 {

  template<typename KeyT, typename ValueT>
  class LRUCache {
  public:
    using KVPair = std::pair<KeyT, ValueT>;
    using ListIter = typename std::list<KVPair>::iterator;

    LRUCache(size_t aCapacity=200) :
      capacity(aCapacity) {
    }
        
    size_t size() const {return itemsMap.size();}
    
    bool contains(const KeyT& key) const {
      return itemsMap.find(key) != itemsMap.end();
    }

    void evictIf() {
      if (itemsMap.size() > capacity) {
        auto last = itemsList.end();
        last--;
        itemsMap.erase(last->first);
        itemsList.pop_back();
      }
    }
    
    //only call this if you know the key is new
    void add(const KeyT& key, const ValueT& value) {
      itemsList.push_front(KVPair(key, value));
      itemsMap[key] = itemsList.begin();
      evictIf();
    }

    void put(const KeyT& key, const ValueT& value) {
      auto it = itemsMap.find(key);
      itemsList.push_front(KVPair(key, value));
      if (it != itemsMap.end()) {
        itemsList.erase(it->second);
        itemsMap.erase(it);
      }
      itemsMap[key] = itemsList.begin();
      evictIf();
    }
    const ValueT& get(const KeyT& key) {
      auto it = itemsMap.find(key);
      if (it == itemsMap.end()) {
        throw std::range_error("There is no such key in cache");
      }
      
      //move selected item to front of our list...
      itemsList.splice(itemsList.begin(), itemsList, it->second);
      return it->second->second;
    }
    
    void clear() {
       itemsMap.clear();
       itemsList.clear();
    }
    
    bool erase(const KeyT &aKey) {
      return true;
    }

    bool remove(const KeyT& aKey) {
      auto iter = itemsMap.find(aKey);
      if (iter == itemsMap.end()) {
        return false;
      }
      itemsList.erase(iter->second);
      itemsMap.erase(iter);
      return true;
    }

    
  private:
    std::list<KVPair> itemsList;
    std::unordered_map<KeyT, ListIter> itemsMap;
    size_t capacity;
  };
}

#endif /* LRUCache_h */
