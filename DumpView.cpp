//
// Created by asmak on 4/17/2023.
//

#include "DumpView.hpp"


namespace ECE141 {
    DumpView::DumpView(Storage& aStorage) {
      aStorage.each([&](const Block& theBlock, uint32_t size) {
          dump.add(theBlock);
          return true;
          });
    }

    bool DumpView::show(std::ostream &aStream) {
      std::vector<int> colWidths{6,15,9,35};
      std::vector<std::string> colNames{"Block","Type","TableId","Extra"};
      std::string outString = "";
      // schema header
      std::string divider = "+";
      std::string header = "|";

      for (unsigned int i = 0; i < colWidths.size(); i++) {
        divider += std::string(colWidths[i], '-') + "+";
        std::string element = colNames[i];
        header += element + std::string(colWidths[i] - element.size(), ' ') + "|";
      }
      divider += "\n"; header += "\n";
      outString += divider + header + divider;
      for (unsigned int j = 0; j < dump.dumpDataList.size(); j++) {
        outString += "|";
        std::vector<std::string> fields{std::to_string(j),dump.dumpDataList[j].type,dump.dumpDataList[j].id,dump.dumpDataList[j].extra};
        for (unsigned int i = 0; i < fields.size(); i++) {
          std::string element = fields[i];
          outString += element + std::string(std::max(0,colWidths[i] - int(element.size())), ' ') + "|";
        }
        outString += "\n" + divider;
      }
      aStream << outString;
      return true;
    }

} // ECE141