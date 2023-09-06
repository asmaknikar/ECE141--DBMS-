//
//  BasicTypes.hpp
//  RGAssignement1
//
//  Created by rick gessner on 3/30/23.
//  Copyright © 2018-2023 rick gessner. All rights reserved.
//

#ifndef BasicTypes_h
#define BasicTypes_h

#include <optional>
#include <string>
#include <vector>
#include <map>
#include <variant>

namespace ECE141 {

  enum class DataTypes {
     no_type='N',  bool_type='B', datetime_type='D',
      float_type='F', int_type='I', varchar_type='V',
  };
  template <typename CRTP>
  class AsSingleton {
  public:
      static CRTP& getInstance() {
          static CRTP theInstance;
          return theInstance;
      }
  };
  using OptStringView = std::optional<std::string_view>;
  using OptString = std::optional<std::string>;
  using StringList = std::vector<std::string>;
  using StringMap = std::map<std::string, std::string>;
  using ind = uint32_t;
  using IndexList = std::vector<ind>;
  using Value = std::variant<bool, int, float, std::string>;
  using ValueOpt = std::optional<Value>;
}
#endif /* BasicTypes_h */
