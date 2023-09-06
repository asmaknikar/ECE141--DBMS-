//
// Created by asmak on 4/17/2023.
//

#include "Dump.hpp"
#include <stdio.h>

namespace ECE141 {
    Dump::Dump(DumpDataList aDumpDataList) {
      dumpDataList = aDumpDataList;
    }

    Dump::Dump() {
    }

    void Dump::add(const Block& aBlock) {
        DumpData theData(aBlock);
      dumpDataList.push_back(theData);
    }


} // ECE141