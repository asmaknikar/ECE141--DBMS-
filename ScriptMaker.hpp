//
//  ScriptMaker.hpp
//  RG_PA9
//
//  Created by rick gessner on 6/4/23.
//

#ifndef ScriptMaker_h
#define ScriptMaker_h

#include <algorithm>
#include <random>
#include <stack>
#include "TestSequencer.hpp"
#include "Faked.hpp"

namespace ECE141 {

  struct CreateTableProps {
    Errors  error;
    bool    cmdBegErr;
    bool    cmdEndErr;
    char    cmdOpenParen;
    size_t  fldPrefix;
    size_t  typePrefix;
    size_t  fldDelim;
    char    cmdCloseParen;
  };

  CommandCount addBadTable(const CreateTableProps &aProps, char aChar,std::ostream &anOutput) {
    static char prefixes[]={' ','0'};
    static const char fldDelims[]={',',' '};
    
    static std::map<std::string, std::string> kFields  = {
      {"id", "int NOT NULL auto_increment primary key "},
      {"name", "varchar(50) "},
      {"price", "float "},
    };

    anOutput  << "create " << prefixes[aProps.cmdBegErr] << "TABLE "
              << prefixes[aProps.cmdEndErr] << "table_" << aChar << aProps.cmdOpenParen;
    
    size_t thePos=1;
    for(auto &thePair : kFields) {
      anOutput << prefixes[thePos==aProps.fldPrefix] << thePair.first
        << ' ' << prefixes[thePos==aProps.typePrefix]
        << thePair.second;
      if(thePos<kFields.size()) anOutput << fldDelims[thePos==aProps.fldDelim];
      thePos++;
    }
          
    anOutput << aProps.cmdCloseParen << ";\n";
    
    CommandCount theCommand{Commands::createTable,1,'>',true};
    return theCommand;
  }

  void addUsersTable(std::ostream &anOutput) {
    anOutput << "create table Users (";
    anOutput << " id int NOT NULL auto_increment primary key,";
    anOutput << " first_name varchar(50) NOT NULL,";
    anOutput << " last_name varchar(50),";
    anOutput << " age int,";
    anOutput << " zip int);\n";
  }

  void addPaymentsTable(std::ostream &anOutput) {
    anOutput << "create table Payments (";
    anOutput << " id int NOT NULL auto_increment primary key,";
    anOutput << " user_id int,";
    anOutput << " paydate int,";
    anOutput << " amount int);\n";
  }

  void addAccountsTable(std::ostream &anOutput) {
    anOutput << "create table Accounts (";
    anOutput << " id int NOT NULL auto_increment primary key,";
    anOutput << " account_type varchar(25) NOT NULL,";
    anOutput << " amount int);\n";
  }

  void addBooksTable(std::ostream &anOutput) {
    anOutput << "create table Books (";
    anOutput << " id int NOT NULL auto_increment primary key,";
    anOutput << " title varchar(25) NOT NULL,";
    anOutput << " publisher varchar(50),";
    anOutput << " isbn varchar(10),";
    anOutput << " user_id int);\n";
  }
  //-------------------------------------------------------
          
  void insertUsers(std::ostream &anOut, size_t anOffset, size_t aLimit) {
    static const char* kUsers[]={
      " (\"Terry\",     \"Pratchett\", 70,  92124)",
      " (\"Ian\",       \"Tregellis\", 48,  92123)",
      " (\"Jody\",      \"Taylor\",    50,  92120)",
      " (\"Stephen\",   \"King\",      74,  92125)",
      " (\"Ted\",       \"Chiang\",    56,  92120)",
      " (\"Anthony\",   \"Doerr\",     52,  92122)",
      " (\"J.R.R.\",    \"Tolkien\",   130, 92126)",
      " (\"Aurthur C.\",\"Clarke\",    105, 92127)",
      " (\"Seldon\",    \"Edwards\",   81,  92128)",
      " (\"Neal\",      \"Stephenson\",62,  92121)"
    };

    anOut<<"INSERT INTO Users (first_name, last_name, age, zip)";

    size_t theSize=sizeof(kUsers)/sizeof(char*);
    size_t theLimit=std::min(theSize, anOffset+aLimit);
    const char* thePrefix=" VALUES";
    for(size_t i=anOffset;i<theLimit;i++) {
      anOut << thePrefix << kUsers[i];
      thePrefix=",";
    }
    anOut << ";\n";
  }
     
  void insertFakeUsers(std::ostream &anOut, int aGroupSize,
                   Expected &anExpected, int aGroupCount=1) {
    for(int theCount=0;theCount<aGroupCount;theCount++) {
      anOut<<"INSERT INTO Users (first_name, last_name, age, zip) VALUES ";
      const char* thePrefix="";
      for(int theSize=0;theSize<aGroupSize;theSize++) {
        anOut << thePrefix <<
          '(' << '"' << Fake::People::first_name()
          << "\", \"" << Fake::People::last_name()
          << "\", " << Fake::People::age(20,60)
          << ", " << Fake::Places::zipcode(92130,5000) << ')';
        thePrefix=",";
      }
      anExpected.add({Commands::insert, aGroupSize});
      anOut << ";\n";
    }
  }

  void insertBooks(std::ostream &anOut,
           size_t anOffset, size_t aLimit) {
    static const char* kBooks[]={
      " (\"The Green Mile\",4, \"C123-932L\", \"Harper\")",
      " (\"The Stand\",4, \"RV36-M11B\", \"Harper\")",
      " (\"Misery\",4, \"VI77-21K3\", \"Harper\")",
      " (\"11/22/63\",4, \"PA45-M023\", \"Harper\")",
      " (\"The Institute\",4, \"F94K-916M\", \"Harper\")",
      " (\"Sorcerer\",1, \"E598-B81S\", \"Collins\")",
      " (\"Wintersmith\",1, \"W84S-P70R\", \"Collins\")",
      " (\"Mort\",1, \"KEJ5-27D3\", \"Collins\")",
      " (\"Thud\",1, \"YAL4-J001\", \"Collins\")",
      " (\"Time Police\",3, \"EK50-J001\", \"Prentiss\")",
      " (\"The Mechanical\",2, \"ULRR-1320\", \"Hex & Bolt\")",
      " (\"The Liberation\",2, \"ZK95-9413\", \"Hex & Bolt\")",
      " (\"The Rising\",2, \"ECC7-6BB0\", \"Prentiss\")",
      " (\"Exhalation\",5, \"18MQ-Q414\", \"Prentiss\")",
    };

    anOut<<"INSERT INTO Books (title, user_id, isbn, publisher)";

    size_t theSize=sizeof(kBooks)/sizeof(char*);
    size_t theLimit=std::min(theSize, anOffset+aLimit);
    const char* thePrefix=" VALUES";
    for(size_t i=anOffset;i<theLimit;i++) {
      anOut << thePrefix << kBooks[i];
      thePrefix=",";
    }
    anOut << ";\n";
  }

  std::string getUserSelect(const std::string &aFields,
                            const InitList &aClauses) {
    std::string theResult("SELECT " + aFields + " from Users ");
    if(aClauses.size()) {
      std::vector<std::string> theClauses(aClauses);
      auto rd = std::random_device {};
      auto rng = std::default_random_engine {rd()};
      std::shuffle(theClauses.begin(), theClauses.end(), rng);
      for(auto theClause : theClauses) {
        theResult+=theClause;
      }
    }
    theResult+=";\n";
    return theResult;
  }

  //----------------------------------------------

  bool createFile(const std::string &aPath, const std::string &aName) {
    std::string temp(aPath);
    temp+='/';
    temp+=aName;
    std::fstream output(temp, std::fstream::out | std::fstream::trunc);
    output << "some text\n";
    return true;
  }

  bool deleteFiles(StringList &aFileList) {
    for(auto &theFile : aFileList) {
      fs::remove(theFile);
    }
    return true;
  }

  std::string getRandomDBName(char aChar='A') {
    uint32_t theCount=rand() % 99999;
    return std::string("testdb_"+std::to_string(theCount+1000)+aChar);
  }

  //---------------------------------------------

  class ScriptMaker {
  public:
    ScriptMaker(Expected &anExpected) : expected(anExpected) {}
    
    ScriptMaker& createDBTests(const StringList &aFileList) {
      for(auto &theFile: aFileList) {
        cmds << "CREATE DATABASE "+theFile << ";\n ";
      }      
      cmds << "use "+aFileList.at(0) << ";\n";
      cmds << "shoW databaseS;\n";
      
      expected.clear().add({
        {Commands::createDB,1}, {Commands::createDB,1},
        {Commands::createDB,1}, {Commands::useDB,0}, {Commands::showDBs,2,'>'}
      });
      return *this;
    }
    
    ScriptMaker& dropAndShowTests(const StringList &aFileList) {
      cmds.clear(std::stringstream::goodbit);
      cmds << "DRop dataBASE "+aFileList[1] << ";\n ";
      cmds << "shoW databaseS;\n";
      cmds << "dump database "+aFileList[0] << ";\n ";
      cmds << "drop database "+aFileList[0] << ";\n ";
      cmds << "drop database "+aFileList[2] << ";\n ";
      expected.clear().add({
        {Commands::dropDB,0}, {Commands::showDBs, 1, '>'},
        {Commands::dumpDB, -1,'>'}, {Commands::dropDB,0},
        {Commands::dropDB,0}
      });
      return *this;
    }
    
    operator std::istream&() {return cmds;}
    
    ScriptMaker& filterTest(int aCount=1) {
      std::string theDBName("db_"+std::to_string(rand()%9999));
      cmds << "create database " << theDBName << ";\n";
      cmds << "use " << theDBName << ";\n";

      expected.clear().add({
        {Commands::createDB,1},    {Commands::useDB,0},
        {Commands::createTable,1}, {Commands::insert,5},
        {Commands::createTable,1}, {Commands::insert,14},
      });

      addUsersTable(cmds);
      insertUsers(cmds,0,5);
      addBooksTable(cmds);
      insertBooks(cmds,0,14);
              
      insertFakeUsers(cmds, 50, expected, aCount);

      const int theLCount=rand() % 40;
      std::string theLimit(" limit "+std::to_string(theLCount));
      cmds << "select * from Books;\n";
      cmds << getUserSelect("*", {" order by zip",
                                     " where zip>92122"});
      cmds << getUserSelect("first_name, last_name, age",
                               {" order by zip desc", theLimit});

      expected.add({Commands::select,14});
      expected.add({Commands::select,3+(50*aCount)});
      expected.add({Commands::select,theLCount});

      cmds << "show tables;\n";
      cmds << "dump database " << theDBName << ";\n";
      cmds << "drop database " << theDBName << ";\n";
      
      expected.add({
        {Commands::showTables,2},  {Commands::dumpDB,50*aCount,'>'},
        {Commands::dropDB,0}
      });
      
      cmds << std::endl;
      return *this;
    }
    
    ScriptMaker& indexTest(int aCount=1) {

      std::string theDBName1(getRandomDBName('H'));
      std::string theDBName2(getRandomDBName('H'));
      cmds << "create database " << theDBName2 << ";\n";
      cmds << "create database " << theDBName1 << ";\n";
      cmds << "use " << theDBName1 << ";\n";

      addUsersTable(cmds);
      insertUsers(cmds,0,5);
      cmds << "use " << theDBName2 << ";\n";

      addUsersTable(cmds);
      insertUsers(cmds,0,5);
      cmds << "use " << theDBName1 << ";\n";

      expected.clear().add({
       {Commands::createDB,1},    {Commands::createDB,1},
       {Commands::useDB,0},       {Commands::createTable,1},
       {Commands::insert,5},      {Commands::useDB,0},
       {Commands::createTable,1}, {Commands::insert,5},
       {Commands::useDB,0},
      });
           
      addBooksTable(cmds);
      expected.add({Commands::createTable,1});

      insertBooks(cmds,0,14);
      expected.add({Commands::insert,14});

      insertFakeUsers(cmds, 50, expected, aCount);

      const int theLCount=10+ (rand() % (50*aCount));
      std::string theLimit(" limit "+std::to_string(theLCount));
      cmds << getUserSelect("first_name, last_name, age",
                              {" where id>25", theLimit});
      expected.add({Commands::select,4,'>'});

      cmds << "select * from Books;\n";
      cmds << "select * from Books where id>6;\n";

      expected.add({Commands::select,14});
      expected.add({Commands::select,8});

      cmds << "dump database " << theDBName1 << ";\n";
      cmds << "drop database " << theDBName1 << ";\n";
      cmds << "drop database " << theDBName2 << ";\n";

      expected.add({Commands::dumpDB,70,'>'});
      expected.add({Commands::dropDB,0});
      expected.add({Commands::dropDB,0});

      cmds << std::endl;
      return *this;
    }
    
    ScriptMaker& showIndexScript(int aCount=1) {
      
      std::string theDBName1(getRandomDBName('H'));
      std::string theDBName2(getRandomDBName('H'));
      cmds << "create database " << theDBName1 << ";\n";
      cmds << "create database " << theDBName2 << ";\n";
      cmds << "use " << theDBName1 << ";\n";

      expected.clear().add({
        {Commands::createDB,1},     {Commands::createDB,1},
        {Commands::useDB,0},        {Commands::createTable,1},
      });

      addUsersTable(cmds);
      insertFakeUsers(cmds, 50, expected, aCount);

      cmds << "use " << theDBName2 << ";\n";
      cmds << "use " << theDBName1 << ";\n";

      expected.add({Commands::useDB,0});
      expected.add({Commands::useDB,0});

      cmds << "show indexes;\n";
      cmds << "show index id from Users;\n";

      expected.add({Commands::showIndexes,1});
      expected.add({Commands::showIndex,50});

      cmds << "drop database " << theDBName1 << ";\n";
      cmds << "drop database " << theDBName2 << ";\n";

      expected.add({Commands::dropDB,0});
      expected.add({Commands::dropDB,0});

      cmds << std::endl;
      return *this;
    }
      
    ScriptMaker& joinTest(int aCount=1) {
    
      std::string theDBName1(getRandomDBName('J'));

      cmds << "create database " << theDBName1 << ";\n";
      cmds << "use " << theDBName1 << ";\n";
       
      addUsersTable(cmds);
      insertUsers(cmds,0,6);
      addBooksTable(cmds);
      insertBooks(cmds,0,14);

      cmds << "select * from Users order by last_name;\n";
      cmds << "select * from Books order by title;\n";

      static const std::string theFields[]={"isbn","publisher"};
      cmds << "select first_name, last_name, title, "
         << theFields[rand() % 2]
         << " from Users right join Books on Users.id=Books.user_id "
         << "order by last_name;\n";
      cmds << "select first_name, last_name, title, "
         << theFields[rand() % 2]
         << " from Users left join Books on Users.id=Books.user_id"
         << " order by last_name;\n";
        cmds << "drop database " << theDBName1 << ";\n";
      
      expected.clear().add({
        {Commands::createDB,1},    {Commands::useDB,0},
        {Commands::createTable,1}, {Commands::insert,6},
        {Commands::createTable,1}, {Commands::insert,14},
        {Commands::select,6},      {Commands::select,14},
        {Commands::select,14},     {Commands::select,15},
        {Commands::dropDB,0},
      });
      
      cmds << std::endl;
      return *this;
    }
    
    ScriptMaker& selectTest() {

      std::string theDBName("db_"+std::to_string(rand()%9999));
      cmds << "create database " << theDBName << ";\n";
      cmds << "use " << theDBName << ";\n";

      addBooksTable(cmds);
      insertBooks(cmds,0,14);

      addUsersTable(cmds);
      insertUsers(cmds,0,10);

      cmds << getUserSelect("*", {});//basic
      cmds << getUserSelect("*", {" order by zip"});

      cmds << "show tables;\n";
      cmds << "dump database " << theDBName << ";\n";
      cmds << "drop database " << theDBName << ";\n";
      cmds << std::endl;
      
      expected.clear().add({
        {Commands::createDB,1},    {Commands::useDB,0},
        {Commands::createTable,1}, {Commands::insert,14},
        {Commands::createTable,1}, {Commands::insert,10},
        {Commands::select,10},     {Commands::select,10},
        {Commands::showTables,2},
        {Commands::dumpDB,20,'>'},  {Commands::dropDB,0},
      });
      
      cmds << std::endl;
      return *this;
    }
    
    ScriptMaker& parseTest(Expected &anExpected) {
      std::string theDBName(getRandomDBName('C'));
      
      cmds << "create database " << theDBName << ";\n";
      cmds << "use " << theDBName << ";\n";
        
      addUsersTable(cmds);
            
      expected.clear().add({
        {Commands::createDB,1},{Commands::useDB,0},
        {Commands::createTable,1},
      });

      std::vector<CreateTableProps> kProps {        
        {Errors::syntaxError, false, false, '(', 0,0,0,' '}, //close paren
        {Errors::invalidTableName, false, true, '(', 0,0,0,')'}, //cmdend
        {Errors::syntaxError, false, false, ' ', 0,0,0,')'}, //openparen
        {Errors::illegalIdentifier, false, false, '(', 1,0,0,')'}, //fldprefix
        {Errors::unknownType, false, false, '(', 0,1,0,')'}, //typeprefix
        {Errors::syntaxError, false, false, '(', 0,0,1,')'}, //flddelim
      };
            
      char theChar{'a'};
      for(auto &theProps : kProps) {
        expected.add(addBadTable(theProps,theChar++, cmds));
      }
      
      cmds << "drop database " << theDBName << ";\n";
      expected.add({Commands::dropDB,0});
        
      cmds << std::endl;
      return *this;
    }
    
    ScriptMaker& tablesTest() {
      std::string theDBName(getRandomDBName('B'));
  
      cmds << "create database " << theDBName << ";\n";
      cmds << "use " << theDBName << ";\n";
      
      addUsersTable(cmds);
      addAccountsTable(cmds);
      addPaymentsTable(cmds);

      cmds << "show tables;\n";
      cmds << "describe Accounts;\n";
      cmds << "drop table Accounts;\n";
      cmds << "show tables;\n";
      cmds << "drop database " << theDBName << ";\n";

      expected.clear().add({
        {Commands::createDB,1},    {Commands::useDB,0},
        {Commands::createTable,1}, {Commands::createTable,1},
        {Commands::createTable,1}, {Commands::showTables,3},
        {Commands::describe,3},    {Commands::dropTable,0},
        {Commands::showTables,2},  {Commands::dropDB,0},
      });
      cmds << std::endl;
      return *this;
    }
    
    ScriptMaker& insertTest(int aCount=1) {
      std::string theDBName(getRandomDBName('B'));

      cmds << "create database " << theDBName << ";\n";
      cmds << "use " << theDBName << ";\n";
      
      expected.clear().add({
        {Commands::createDB,1},    {Commands::useDB,0},
        {Commands::createTable,1}, {Commands::insert,5},
      });
            
      addUsersTable(cmds);
      insertUsers(cmds,0,5);
      insertFakeUsers(cmds, 50, expected, aCount);

      cmds << "show tables;\n";
      cmds << "dump database " << theDBName << ";\n";
      cmds << "drop database " << theDBName << ";\n";
      
      expected.add({
        {Commands::showTables,1},
        {Commands::dumpDB,aCount*50,'>'},
        {Commands::dropDB,0},
      });
      cmds << std::endl;
      return *this;
    }
    
    ScriptMaker& deleteTest(int aCount) {
      
      std::string theDBName1(getRandomDBName('F'));
      std::string theDBName2(getRandomDBName('F'));

      cmds << "create database " << theDBName1 << ";\n";
      cmds << "create database " << theDBName2 << ";\n";
      cmds << "use " << theDBName1 << ";\n";
      
      addUsersTable(cmds);
      insertUsers(cmds,0,5);
      
      cmds << "select * from Users;\n";

      cmds << "use " << theDBName2 << ";\n";
      addUsersTable(cmds);
      insertUsers(cmds,6,4);

      cmds << "select * from Users;\n";

      cmds << "use " << theDBName1 << ";\n";

      cmds << "DELETE from Users where zip=92120;\n";
      cmds << "select * from Users;\n";
      cmds << "DELETE from Users where zip<92124;\n";
      cmds << "select * from Users;\n";
      cmds << "DELETE from Users where zip>92124;\n";
      cmds << "select * from Users;\n";
      
      cmds << "drop database " << theDBName1 << ";\n";
      cmds << "drop database " << theDBName2 << ";\n";

      expected.clear().add({
        {Commands::createDB,1},    {Commands::createDB,1},
        {Commands::useDB,0},       {Commands::createTable,1},
        {Commands::insert,5},      {Commands::select,5},
        {Commands::useDB,0},       {Commands::createTable,1},
        {Commands::insert,4},      {Commands::select,4},
        
        {Commands::useDB,0},       {Commands::delet,2},
        {Commands::select,3},      {Commands::delet,1},
        {Commands::select,2},      {Commands::delet,1},
        {Commands::select,1},      {Commands::dropDB,0},
        {Commands::dropDB,0},
      });
      cmds << std::endl;
      return *this;
    }
    
    ScriptMaker& dropTest(int aCount) {

      std::string theDBName1(getRandomDBName('F'));
      
      cmds << "create database " << theDBName1 << ";\n";
      cmds << "use " << theDBName1 << ";\n";
     
      addUsersTable(cmds);
      insertUsers(cmds,0,5);
      cmds << "select * from Users;\n";

      addBooksTable(cmds);
      const size_t theBookCount=13;
      insertBooks(cmds,0,theBookCount);
      cmds << "select * from Books;\n";

      cmds << "show tables;\n";
      cmds << "DROP table Books;\n";
      cmds << "show tables;\n";

      cmds << "drop database " << theDBName1 << ";\n";
      expected.clear().add({
        {Commands::createDB,1},    {Commands::useDB,0},
        {Commands::createTable,1}, {Commands::insert,5},
        {Commands::select,5},      {Commands::createTable,1},
        {Commands::insert,theBookCount}, {Commands::select,theBookCount},
        {Commands::showTables,2},  {Commands::dropTable,theBookCount},
        {Commands::showTables,1},  {Commands::dropDB,0},
      });
      cmds << std::endl;
      return *this;
    }
    
    ScriptMaker& updateTest(int aCount) {

      std::string theDBName1(getRandomDBName('E'));
      std::string theDBName2(getRandomDBName('E'));

      std::stringstream theCmds, theOut;
      cmds << "create database " << theDBName2 << ";\n";
      cmds << "create database " << theDBName1 << ";\n";
      cmds << "use " << theDBName1 << ";\n";

      addUsersTable(cmds);
      insertUsers(cmds,0,5);

      cmds << "use " << theDBName2 << ";\n";
      addUsersTable(cmds);
      insertUsers(cmds,6,4);

      cmds << "use " << theDBName1 << ";\n";
      cmds << "select * from Users;\n";

      std::string theZip(std::to_string(10000+rand()%75000));

      cmds << "update Users set zip=" << theZip
              << " where id=5;\n";

      cmds << "select * from Users;\n";

      cmds << "use " << theDBName2 << ";\n";
      cmds << "select * from Users;\n";

      cmds << "use " << theDBName1 << ";\n";
      cmds << "select * from Users where zip="
              << theZip << ";\n";

      cmds << "drop database " << theDBName1 << ";\n";
      cmds << "drop database " << theDBName2 << ";\n";
      
      expected.clear().add({
        {Commands::createDB,1},     {Commands::createDB,1},
        {Commands::useDB,0},        {Commands::createTable,1},
        {Commands::insert,5},       {Commands::useDB,0},
        {Commands::createTable,1},  {Commands::insert,4},
        {Commands::useDB,0},        {Commands::select,5},
        {Commands::update,1},       {Commands::select,5},
        {Commands::useDB,0},        {Commands::select,4},
        {Commands::useDB,0},        {Commands::select,1},
        {Commands::dropDB,0},       {Commands::dropDB,0}
      });
      cmds << std::endl;
      return *this;
    }
    
  protected:
    std::stringstream cmds;    
    Expected          &expected;
  };

}

#endif /* ScriptMaker_h */
