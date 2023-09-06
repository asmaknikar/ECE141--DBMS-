//
//  main.cpp
//  Database2
//
//  Created by rick gessner on 3/30/23.
//  Copyright © 2018-2023 rick gessner. All rights reserved.
//


#include <iostream>
#include <sstream>
#include <fstream>
#include <map>
#include <algorithm>
#include <type_traits>
#include <string>
#include <stack>

#include "TestManually.hpp"
#include "TestAutomatic.hpp"
#include "Remoting.hpp"
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
#include "Cout2VisualStudioDebugOutput.h"
#endif


size_t Config::cacheSize[]={0,0,0};
bool   Config::indexing{true};

bool runAllTests(TestAutomatic &aTest) {
  return
    aTest.report("about",   aTest.aboutTest(), false) &&
    aTest.report("cache",   aTest.cacheCommandTest(5), false) &&
    aTest.report("db",      aTest.dbCommandsTest(), false) &&
    aTest.report("drop",    aTest.dropCommandTest(), false) &&
    aTest.report("delete",  aTest.deleteCommandTest(), false) &&
    aTest.report("filter",  aTest.filterCommandTest(5), false) &&
    aTest.report("index",   aTest.indexCommandTest(true,20), false) &&
    aTest.report("insert",  aTest.insertCommandTest(), false) &&
    aTest.report("join",    aTest.joinCommandTest(), false) &&
    aTest.report("noindex", aTest.indexCommandTest(false,20), false) &&
    aTest.report("overfill",aTest.insertCommandTest(20), false) &&
    aTest.report("indexes", aTest.showIndexCommandTest(1), false) &&
    aTest.report("select",  aTest.selectCommandTest(), false) &&
    aTest.report("table",   aTest.tableCommandsTest(), false) &&
    aTest.report("parse",   aTest.parseTest(), false) &&
    aTest.report("update",  aTest.updateCommandTest(), false) &&
    aTest.report("version", aTest.versionTest(), false);
}

int main(int argc, const char * argv[]) {

  srand(static_cast<uint32_t>(time(0)));
  if(argc>1) {
    #if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
          Cout2VisualStudioDebugOutput c2v;
    #endif
        
          ECE141::TestAutomatic theTests;
          static ECE141::TestCalls theCalls{
            {"about",   [&]() {return theTests.aboutTest(); }},
            {"cache",   [&]() {return theTests.cacheCommandTest(); }},
            {"compile", [&]() {return true; }},
            {"delete",  [&]() {return theTests.deleteCommandTest(); }},
            {"db",      [&]() {return theTests.dbCommandsTest(); }},
            {"drop",    [&]() {return theTests.dropCommandTest(); }},
            {"filter",  [&]() {return theTests.filterCommandTest(); }},
            {"help",    [&]() {return theTests.helpTest(); }},
            {"index",   [&]() {return theTests.indexCommandTest(true,20); }},
            {"insert",  [&]() {return theTests.insertCommandTest(); }},
            {"join",    [&]() {return theTests.joinCommandTest(); }},
            {"noindex", [&]() {return theTests.indexCommandTest(false,20); }},
            {"overfill",[&]() {return theTests.insertCommandTest(20); }},
            {"select",  [&]() {return theTests.selectCommandTest(); }},
            {"indexes", [&]() {return theTests.showIndexCommandTest(1); }},
            {"table",   [&]() {return theTests.tableCommandsTest(); }},
            {"parse",   [&]() {return theTests.parseTest(); }},
            {"update",  [&]() {return theTests.updateCommandTest(); }},
            {"quit",    [&]() {return theTests.quitTest(); }},
            {"version", [&]() {return theTests.versionTest(); }},
          };

          std::string theCmd(argv[1]);
          std::transform(theCmd.begin(), theCmd.end(), theCmd.begin(),
              [](unsigned char c) { return std::tolower(c); });

          if (theCalls.count(theCmd)) {
              theTests.report(theCmd, theCalls[theCmd](), theCmd != "all");
          }
          else if (theCmd == "all") {
              theTests.report(theCmd, runAllTests(theTests));
          }
          else if (theCmd == "run") {
              std::fstream theInput(argv[2]);
              ScriptRunner theRunner;
              theRunner.run(theInput, std::cout);
          }
          else std::cout << "Unknown test\n";
  }
  else {
      doManualTesting();
  }
  return 0;
}