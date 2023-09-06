//  TestAutomatic.hpp
//  Created by rick gessner on 3/30/23.
//  Copyright © 2018-2023 rick gessner. All rights reserved.

#ifndef TestAutomatic_h
#define TestAutomatic_h

#include <sstream>
#include <vector>
#include <initializer_list>
#include <functional>
#include "FolderReader.hpp"
#include "ScriptRunner.hpp"
#include "OutputValidator.hpp"
#include "ScriptMaker.hpp"

using namespace ECE141;
using sstream = std::stringstream;

namespace ECE141 {
  const std::string_view theVersion{"Tester(1.95)"};
  const size_t kMaxErrors{100};

  class TestAutomatic {
  public:
    TestAutomatic() : expected(), make(expected) {}
    ~TestAutomatic() {
      std::cout << "---------- " << theVersion << " ----------\n";
    }
    operator sstream&() {return out;}
    
    bool scriptTest(std::istream &anIn, sstream &anOut, Validate aTest) {
      anIn >> std::noskipws;
      ScriptRunner theRunner;
      theRunner.run(anIn,anOut);
      return aTest({anOut.str()});
    }
    
    bool validatedScriptTest(ScriptMaker &aMaker) {
      sstream theOut;
      return scriptTest(aMaker, theOut, [&](const std::string &aBuf) {
        out << theOut.str() << "\n"; //capture!
        Responses theResp;
        return analyzeOutput(theOut,theResp) && expected==theResp;
      });
    }
    
    bool aboutTest() {
      std::stringstream theInput(std::string("about;"));
      return scriptTest(theInput, out, [&](const std::string &aBuffer) {
        return matches(aBuffer, {{"about",4}});
      });
    }
    
    bool helpTest() {
      std::stringstream theInput(std::string("help;"));
      return scriptTest(theInput, out, [&](const std::string &aBuffer) {
        return helpIsValid(aBuffer);
      });
    }
    
    bool quitTest() {
      std::stringstream theInput(std::string("quit;"));
      return scriptTest(theInput, out, [&](const std::string &aBuffer) {
        return matches(aBuffer, {{"quit",4},{"DB::141 is shutting down",24}});
      });
    } //quit
    
    bool versionTest() {
      std::stringstream theInput(std::string("version;"));
      return scriptTest(theInput,out,[&](const std::string &aBuffer) {
        return matches(aBuffer, {{"version",7}, {"Version:",10}});
      });
    }

    bool doReaderTest() {
      std::string thePath=Config::getStoragePath();
      createFile(thePath,"test1.txt");      
      FolderReader theReader(thePath.c_str());
      size_t theCount{0};
      theReader.each(".txt",[&](const std::string &aName) {
        theCount++;
        return true;
      });
      return theCount>0;
    }
                
    bool hasFiles(const StringList &aFilelist) {
      for(auto theFile : aFilelist) {
        std::string thePath=Config::getDBPath(theFile);
        std::ifstream theStream(thePath);
        if(!theStream) return false;
      }
      return true;
    }

    bool dbCommandsTest() {
      std::vector<std::string> theFiles;
      theFiles.push_back(getRandomDBName());
      theFiles.push_back(getRandomDBName());
      theFiles.push_back(getRandomDBName());
      bool theResult{false};
      if(theFiles.size()) {
        Responses theResponses;
        theResult=doCreateDBTests(theFiles, theResponses) &&
                  doDropAndShowTests(theFiles, theResponses[4].count);
        deleteFiles(theFiles);
      }
      return theResult;
    }
    
    bool deleteCommandTest(int aCount=2) {
      return validatedScriptTest(make.deleteTest(aCount));
    }
        
    bool doCreateDBTests(const StringList &aList, Responses &aResponses) {
      sstream theOut;
      make.createDBTests(aList);
      return scriptTest(make,theOut,[&](const std::string &aBuffer) {
        if(hasFiles(aList)) {
          out << theOut.str() << "\n"; //capture!
          auto theCount=analyzeOutput(theOut, aResponses);
          return theCount && expected==aResponses;
        }
        return false;
      });
    }
    
    bool doDropAndShowTests(const StringList &aList, int aPrevDBCount) {
      return validatedScriptTest(make.dropAndShowTests(aList));
    }
        
    bool dropCommandTest(int aCount=2) {
      return validatedScriptTest(make.dropTest(aCount));
    }

    bool filterCommandTest(int aCount=5) {
      return validatedScriptTest(make.filterTest(aCount));
    }
        
    bool cacheCommandTest(int aCount=5) {
      Config::setCacheSize(CacheType::block, 100);
      return filterCommandTest(aCount);
    }

    bool indexCommandTest(bool anIndex, int aCount=1) {
      Config::setUseIndex(anIndex);
      out << "Indexing is " << std::boolalpha << anIndex << "\n";
      return validatedScriptTest(make.indexTest(aCount));
    }
             
    bool insertCommandTest(int aCount=2) {
      return validatedScriptTest(make.insertTest());
    }

    bool joinCommandTest() {
      return validatedScriptTest(make.joinTest(1));
    }

    bool showIndexCommandTest(int aCount=1) {
      Config::setUseIndex(true);
      return validatedScriptTest(make.showIndexScript(aCount));
    }

    bool selectCommandTest() {
      return validatedScriptTest(make.selectTest());
    }
    
    bool parseTest() {
      return validatedScriptTest(make.parseTest(expected));
    }

    bool tableCommandsTest() {
      return validatedScriptTest(make.tablesTest());
    }
    
    bool updateCommandTest(int aCount=2) {
      return validatedScriptTest(make.updateTest(aCount));
    }
    
    bool report(const std::string &aCmd, bool aResult, bool aVerbose=true) {
      static const char* theStatus[]={"FAIL","PASS"};
      std::cout << aCmd << " test " << theStatus[aResult] << "\n";
      if(aVerbose) {
        std::cout << "---------------------------------\n" << out.str() << "\n";
      }
      return aResult;
    }
    
  protected:
    sstream     out;
    Expected    expected;
    ScriptMaker make;
  }; //TestAutomatic...
}
#endif /* TestAutomatic_h */
