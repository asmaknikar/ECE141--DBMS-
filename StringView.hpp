#pragma once
#include "View.hpp"

namespace ECE141 {
    class StringView : public View {
    public:
        //StringView(std::string aString) {str = std::string_view(aString);}
        StringView(std::string_view aStringview) :str(aStringview) {}
        ~StringView() {}
        virtual bool show(std::ostream& aStream) {
            aStream << str;
            return true;
        }
        std::string_view str;
    };

}