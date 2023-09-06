#include "DataBaseView.hpp"
namespace ECE141 {
	DataBaseView::DataBaseView(bool aOK, int aEffected, double aSeconds) :
		ok(aOK),effected(aEffected),seconds(aSeconds) {}

	bool DataBaseView::show(std::ostream& aStream) {
		if (ok) aStream << "Query OK, " << effected << " rows affected (" << seconds << " secs)\n";
		return true;
	}
	
}