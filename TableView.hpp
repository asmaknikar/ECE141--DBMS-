#include "View.hpp"
#include "Table.hpp"
#include "Timer.hpp"

namespace ECE141 {
	class TableView : public View {
	public:
		//StringView(std::string aString) {str = std::string_view(aString);}
		TableView(Table aTable, double anElapsed = 0) : table(aTable),duration(anElapsed) {}
		~TableView() {}
		bool show(std::ostream& aStream) {
			Timer t;
			std::vector<uint32_t> colWidths = table.schema->getColumnWidths();
			if (colWidths.size() == 0) return false;
			std::string outString = "";
			// schema header
			std::string divider = "+";
			std::string header = "|";
			for (unsigned int i = 0; i < colWidths.size(); i++) {
				divider += std::string(colWidths[i], '-') + "+";
				std::string element;
				bool hasValue = table.schema->getHeaderNameAtCol(i,element);
				if(hasValue) header += element + std::string(std::max(int(colWidths[i] - element.length()),0), ' ') + "|";
			}
			divider += "\n"; header += "\n";
			outString += divider + header + divider;
			for (unsigned int j = 0; j < table.rows.size(); j++) {
				outString += "|";
				for (unsigned int i = 0; i < colWidths.size(); i++) {
					std::string element;
					if (table.schema->getHeaderNameAtCol(i, element)) {
						std::string value = table.rows[j]->getValueString(element).substr(0,colWidths[i]);
						outString += value + std::string(colWidths[i] - value.length(), ' ') + "|";
					}
				}
				outString += "\n";
			}
			outString += divider;
			aStream << outString.substr(0,outString.length()-1);
			if (duration > 0)
				aStream << "\n" << std::to_string(table.rows.size()) << " rows in set (" << std::to_string(duration + t.elapsed()) << " secs)\n";
			return true;
		}
		Table table;
		double duration;
	};
}