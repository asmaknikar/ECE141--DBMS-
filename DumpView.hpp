//
// Created by asmak on 4/17/2023.
//

#ifndef SP23DATABASE_DUMPVIEW_HPP
#define SP23DATABASE_DUMPVIEW_HPP

#include "View.hpp"
#include "Dump.hpp"

namespace ECE141 {

    class DumpView: public View {

    public:
        DumpView(Storage& aStorage);

        ~DumpView(){};
        bool show(std::ostream &aStream);
        Dump dump;
    };

} // ECE141

#endif //SP23DATABASE_DUMPVIEW_HPP
