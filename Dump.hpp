//
// Created by asmak on 4/17/2023.
//

#ifndef SP23DATABASE_DUMP_HPP
#define SP23DATABASE_DUMP_HPP

#include <vector>
#include "Storage.hpp"

namespace ECE141 {
    struct DumpData{
        DumpData(Block aBlock) :
            type(std::string(1, aBlock.header.type)),
            id(std::to_string(aBlock.header.tableId)),
            extra(aBlock.extra()) {
        }
        std::string     type;
        std::string     id;
        std::string     extra;
    };

    using DumpDataList = std::vector<DumpData> ;
    class Dump {
    public:
        Dump();
        Dump(DumpDataList aDumpDataList);
        void add(const Block& ablock);
        DumpDataList dumpDataList;

    };


} // ECE141

#endif //SP23DATABASE_DUMP_HPP
