//
//  OutputValidator.hpp
//  RG_PA9
//
//  Created by rick gessner on 6/4/23.
//

#ifndef OutputValidator_h
#define OutputValidator_h

#include "TestSequencer.hpp"
#include <functional>

namespace ECE141 {

  using Validate    = std::function<bool(const std::string &aBuffer)>;
  using TestCall    = std::function<bool()>;
  using InputLine   = std::pair<std::string_view, size_t>;
  using LineList    = std::initializer_list<InputLine>;
  using InitList    = std::initializer_list<std::string>;
  using TestCalls   = std::map<std::string, TestCall>;

  using CountList = std::vector<int>;

  //------------------------------------

  enum class Commands {unknown, alter, createDB, createTable, describe,
    delet, dropDB, dropTable, dumpDB, error, insert, select, showDBs,
    showTables, showIndex, showIndexes, update, useDB};

  struct CommandCount {
    Commands  command;
    int       count;
    char      cmp='=';
    bool      error=false;
    
    bool compare(const CommandCount &anItem) {
      if(command==anItem.command) {
        if(anItem.error!=error) return false;
        return ('>'==cmp) ? anItem.count>count : anItem.count==count;
      }
      return false;
    }
  };

  using Responses = std::vector<CommandCount>;
  using ExpectList = std::initializer_list<CommandCount>;

  class Expected {
  public:
  
    Expected() {}
    Expected(const ExpectList &aList) : expected(aList) {}

    bool operator==(const Responses &aResponses) {
      size_t theSize=expected.size();
      if(theSize && aResponses.size()==theSize) {
        for(size_t i=0;i<theSize;i++) {
          if(!expected[i].compare(aResponses[i])) {
            return false;
          }
        }
        return true;
      }
      return false;
    }

    Expected& add(const CommandCount &aCommand) {
      expected.push_back(aCommand);
      return *this;
    }
          
    Expected& add(const ExpectList &aCommands) {
      for(auto &theCmd: aCommands) {
        expected.push_back(theCmd);
      }
      return *this;
    }
    
    Expected& clear() {
      expected.clear();
      return *this;
    }
    
  protected:
    Responses expected;
  };

  //-------------------------------

  class OutputValidator {
  public:
    OutputValidator() {}
    
  protected:
  };

  //----------------------------------------------
      
  bool helpIsValid(const std::string &aString) {
    static std::vector<const char*> theCmds{
      "about","version","quit","help",
      "create database","show databases","drop database",
      "use", "dump database",
      "create table", "drop table","describe table",
      "select", "insert", "update", "delete", "show tables"
    };
    
    std::string temp(aString);
    std::transform(temp.begin(), temp.end(), temp.begin(), ::tolower);

    for(auto thePhrase : theCmds) {
      if (temp.find(thePhrase) == std::string::npos) {
        return false;
      }
    }
    return true;
  }

  //----------------------------------------------

  bool matches(const std::string &aBuffer, const LineList &aLines) {
    std::stringstream theInput(aBuffer);
    std::string theLine;
    for(auto &theItem: aLines) {
      if(std::getline(theInput,theLine)) {
        if (theLine.find(theItem.first) != std::string::npos) {
          if(theLine.length()<theItem.second)
            return false;
        }
        else return false;
      }
      else return false;
    }
    return true;
  }


  enum states {wasUnknown, wasAlter, wasCreate, wasDescribe,
      wasDelete, wasDrop, wasDump, wasInsert, wasSelect,
      wasShow, wasUpdate, wasUse};
            
  //validates output of DBCommand test..
  size_t analyzeOutput(std::istream &aStream, Responses &aResults) {
    std::stack<Commands> theStack; //tracking open state...

    static KWList createDB{Keywords::create_kw,Keywords::database_kw};
    static KWList changedDB{Keywords::database_kw,Keywords::changed_kw};
    static KWList showDBs{Keywords::show_kw,Keywords::databases_kw};
    static KWList dumpDB{Keywords::dump_kw,Keywords::database_kw};
    static KWList dropDB{Keywords::drop_kw,Keywords::database_kw};
    static KWList createTable{Keywords::create_kw,Keywords::table_kw};
    static KWList showTables{Keywords::show_kw,Keywords::tables_kw};
    static KWList showIndex{Keywords::show_kw,Keywords::index_kw};
    static KWList showIndexes{Keywords::show_kw,Keywords::indexes_kw};
    static KWList dropTable{Keywords::drop_kw,Keywords::table_kw};
    static KWList insertInto{Keywords::insert_kw,Keywords::into_kw};

    Tokenizer theTokenizer(aStream);
    if(theTokenizer.tokenize()) {
      TestSequencer theSeq(theTokenizer);
      int theValue{0};
      while(theTokenizer.more()) {
        //auto &theToken=theTokenizer.current();
        if(theSeq.clear().nextIs(createDB)) {
          if(theSeq.skipPast(Keywords::query_kw)) {
            theTokenizer.skipTo(TokenType::number);
            theSeq.getNumber(theValue).skipPast(')');
            aResults.push_back({Commands::createDB,theValue});
          }
        }
        else if(theSeq.clear().nextIs(showDBs)) {
          if(theSeq.skip(1)) {
            theTokenizer.skipTo(TokenType::number);
            theSeq.getNumber(theValue).skipPast(')');
            aResults.push_back({Commands::showDBs,theValue});
          }
        }
        else if(theSeq.clear().nextIs({Keywords::use_kw})) {
          if(theSeq.skipPast(';').nextIs(changedDB).skipPast(')')) {
            aResults.push_back({Commands::useDB, 0});
          }
        }
        else if(theSeq.clear().nextIs(dropDB)) {
          if(theTokenizer.skipTo(TokenType::number)) {
            theSeq.getNumber(theValue).skipPast(')');
            aResults.push_back({Commands::dropDB,theValue});
          }
        }
        else if(theSeq.clear().nextIs(dumpDB)) {
          if(theSeq.skipPast(Keywords::query_kw)) {
            if(theTokenizer.skipTo(TokenType::number)) {
              theSeq.getNumber(theValue).skipPast(')');
              aResults.push_back({Commands::dumpDB,theValue});
            }
          }
        }
        else if(theSeq.clear().nextIs(createTable)) {
          if(theSeq.skipPast(';').nextIs({Keywords::query_kw})) {
            theTokenizer.skipTo(TokenType::number);
            theSeq.getNumber(theValue).skipPast(')');
            aResults.push_back({Commands::createTable,theValue});
          }
          else {
            theTokenizer.skipTo(TokenType::number);
            theSeq.getNumber(theValue);
            theTokenizer.skipTo(TokenType::number);
            theTokenizer.next(); //skip number..
            aResults.push_back({Commands::createTable, theValue,'>', true});
          }
        }
        else if(theSeq.clear().nextIs(showTables)) {
          theTokenizer.skipTo(TokenType::number);
          theSeq.getNumber(theValue).skipPast(')');
          aResults.push_back({Commands::showTables,theValue});
        }
        else if(theSeq.clear().nextIs(showIndex)) {
          if(theTokenizer.skipTo(Keywords::rows_kw)) {
            auto theToken=theTokenizer.peek(-1);
            theValue=std::stoi(theToken.data);
            theSeq.skip(7);
          }
          aResults.push_back({Commands::showIndex,theValue});
        }
        else if(theSeq.clear().nextIs(showIndexes)) {
          theTokenizer.skipTo(TokenType::number);
          theSeq.getNumber(theValue).skipPast(')');
          aResults.push_back({Commands::showIndexes,theValue});
        }
        else if(theSeq.clear().nextIs(dropTable)) {
          theTokenizer.skipTo(TokenType::number);
          theSeq.getNumber(theValue).skipPast(')');
          aResults.push_back({Commands::dropTable,theValue});
        }
        else if(theSeq.clear().nextIs(insertInto)) {
          if(theSeq.skipPast(';').nextIs({Keywords::query_kw}).skipPast(',')) {
            theSeq.getNumber(theValue).skipPast(')');
            aResults.push_back({Commands::insert,theValue});
          }
        }
        else if(theTokenizer.skipIf(Keywords::describe_kw)) {
          if(theTokenizer.skipTo(Keywords::rows_kw)) {
            auto theToken=theTokenizer.peek(-1);
            theValue=std::stoi(theToken.data);
            aResults.push_back({Commands::describe,theValue});
            theSeq.clear().skipPast(')');
          }
        }
        else if(theSeq.clear().nextIs({Keywords::select_kw})) {
          if(theTokenizer.skipTo(Keywords::rows_kw)) {
            auto theToken=theTokenizer.peek(-1);
            theValue=std::stoi(theToken.data);
            aResults.push_back({Commands::select,theValue});
            theSeq.clear().skipPast(')');
          }
        }
        else if(theSeq.clear().nextIs({Keywords::update_kw})) {
          if(theTokenizer.skipTo(Keywords::rows_kw)) {
            auto theToken=theTokenizer.peek(-1);
            theValue=std::stoi(theToken.data);
            aResults.push_back({Commands::update,theValue});
            theSeq.clear().skipPast(')');
          }
        }
        else if(theSeq.clear().nextIs({Keywords::delete_kw})) {
          if(theTokenizer.skipTo(Keywords::rows_kw)) {
            auto theToken=theTokenizer.peek(-1);
            theValue=std::stoi(theToken.data);
            aResults.push_back({Commands::delet,theValue});
            theSeq.clear().skipPast(')');
          }
        }
        else theTokenizer.next(); //skip...
      }
    }
    return aResults.size();
  }
      
  size_t countDBFiles() {
    std::string thePath=Config::getStoragePath();
    FolderReader theReader(thePath.c_str());
    size_t theCount{0};
    theReader.each(Config::getDBExtension(),
                   [&](const std::string &aName) {
      theCount++;
      return true;
    });
    return theCount;
  }

}

#endif /* OutputValidator_h */

