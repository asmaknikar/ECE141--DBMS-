#include "CommandTokenReader.hpp"

#include "Helpers.hpp"
#include <exception>
#include <algorithm>

namespace ECE141 {

    CommandTokenReader::CommandTokenReader(Command& aCommand){
        tokens = aCommand;
        index = 0;
    }

    Token& CommandTokenReader::tokenAt(size_t anOffset) {
        if (anOffset >= 0 && anOffset < tokens.size()) {
            return tokens[anOffset];
        }
        throw std::out_of_range("invalid offset");
    }

    bool CommandTokenReader::next(int anOffset) {
        index += anOffset;
        return index < size();
    }

    Token& CommandTokenReader::current() {
        return tokens[index];
    }

    Token& CommandTokenReader::peek(int anOffset) {
        return tokenAt(index + anOffset);
    }

    //skip any token till you find this target...
    bool CommandTokenReader::skipTo(TokenType aTarget) {
        while (more()) {
            Token& theToken = current();
            if (theToken.type == aTarget) {
                return true;
            }
            next();
        }
        return false;
    }

    //skip any token till you find this target...
    bool CommandTokenReader::skipTo(Keywords aTarget) {
        while (more()) {
            Token& theToken = current();
            if (TokenType::keyword == theToken.type && aTarget == theToken.keyword) {
                return true;
            }
            next();
        }
        return false;
    }

    //move ahead IFF the current keyword matches given...
    bool CommandTokenReader::skipIf(Keywords aKeyword) {
        if (more() && (aKeyword == current().keyword)) {
            next(); //eat the target...
            return true;
        }
        return false;
    }

    // USE: skip a regular char, usually punctuation....
    bool CommandTokenReader::skipIf(char aChar) {
        if (more() && (aChar == current().data[0])) {
            next(); //eat the target...
            return true;
        }
        return false;
    }

    //move ahead IFF the current token/op matches given...
    bool CommandTokenReader::skipIf(Operators anOperator) {
        Token& theToken = current();
        if (more() && (theToken.type == TokenType::operators) && (theToken.op == anOperator)) {
            next(); //eat the target...
            return true;
        }
        return false;
    }


    bool CommandTokenReader::each(const TokenVisitor aVisitor) {
        for (auto& theToken : tokens) {
            if (!aVisitor(theToken)) return false;
        }
        return true;
    }

    // USE: ----------------------------------------------

    void CommandTokenReader::dump() {
        for (auto& theToken : tokens) {
            std::cerr << "type ";
            switch (theToken.type) {
            case TokenType::punctuation:
                std::cerr << "punct " << theToken.data << "\n";
                break;

                //case TokenType::operators:
            case TokenType::operators:
                std::cerr << "operator " << theToken.data << "\n";
                break;

            case TokenType::number:
                std::cerr << "number " << theToken.data << "\n";
                break;

            case TokenType::string:
                std::cerr << "string " << theToken.data << "\n";
                break;

            case TokenType::identifier:
                std::cerr << "identifier " << theToken.data << "\n";
                break;

            case TokenType::keyword:
                std::cerr << "keyword " << theToken.data << "\n";
                break;

            default:
                break;
            }
        }
    }


}

