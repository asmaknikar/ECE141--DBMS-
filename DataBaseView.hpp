#pragma once
#include "View.hpp"
namespace ECE141 {
	class DataBaseView : public View
	{
    public:
        DataBaseView(bool aOK, int aEffected, double aSeconds);

        ~DataBaseView() {};
        bool show(std::ostream& aStream);
        bool ok;
        int effected;
        double seconds;
	};
}


