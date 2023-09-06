/* A copied class of the tokenizer that is lighter weight and only does 
operations on a tokenized vector*/
#pragma once
#ifndef CommandTokenReader_hpp
#define CommandTokenReader_hpp

#include "Tokenizer.hpp"
#include "Handler.hpp"

namespace ECE141 {
    class CommandTokenReader
    {
    public:
        CommandTokenReader(Command& aCommand);

        Token&        tokenAt(size_t anOffset);

        Token&        current();
        bool          more() { return index < size(); }
        bool          next(int anOffset = 1);
        Token&        peek(int anOffset = 1);
        void          restart() { index = 0; }
        size_t        size() { return tokens.size(); }
        size_t        pos() { return index; }
        size_t        remaining() { return index < size() ? size() - index : 0; }

        //these might consume a token...
        bool          skipTo(Keywords aKeyword);
        bool          skipTo(TokenType aTokenType);

        bool          skipIf(Keywords aKeyword);
        bool          skipIf(Operators anOperator);
        //bool          skipIf(TokenType aTokenType);
        bool          skipIf(char aChar);

        bool          each(const TokenVisitor aVisitor);
        void          dump(); //utility

    protected:

        std::vector<Token>    tokens;
        size_t                index;
    };
   

}

#endif /* CommandTokenReader_hpp */





