//
//  Config.hpp
//
//  Created by rick gessner on 3/30/23.
//  Copyright © 2018-2023 rick gessner. All rights reserved.
//


#ifndef Config_h
#define Config_h
#include <sstream>
#include <filesystem>
#include "Timer.hpp"
//#include "Log.hpp"

namespace fs = std::filesystem;

namespace ECE141 {

  enum class CacheType : int {block=0, rows, views};

  struct Config {

    static size_t cacheSize[3];
    static bool   indexing;

    static const char* getDBExtension() {return ".db";}

    static const std::string getConnectionString() {
        return "localhost";
    }

    static const std::string getTempPath() {
      return fs::temp_directory_path().string();
    }
    
    static const std::string getStoragePath() {
      #if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
        //STUDENT: If you're on windows, use folder on your machine...
        return getTempPath();
      
      #elif __APPLE__ || defined __linux__ || defined __unix__

        return std::string("/tmp");
              
      #endif
    }
    
    static std::string getDBPath(const std::string &aDBName) {
      #if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
        const std::string theSep="\\";
      #elif __APPLE__ || defined __linux__ || defined __unix__
        const std::string theSep="/";
      #endif
            
      std::ostringstream theStream;
      theStream << Config::getStoragePath() << theSep << aDBName << ".db";
      return theStream.str();
    }
      
    static Timer& getTimer() {
      static Timer theTimer;      
      return theTimer;
    }
    
//    static Logger& Log() {
//      return Logger::instance();
//    }
//
    static std::string getAppName() {return "DB::141";}
    static std::string getVersion() {return "Version: 0.9";}
      static std::string_view getMembers() { return "Authors: Shane and Ashish"; }
      static std::string_view getExitMessage() { return "DB::141 is shutting down"; }
      static std::string_view getHelpMessage() {
        std::string_view help{
                "App Commands:\n"
                "about : show members\n"
                "help : show list of commands\n"
                "quit : stop app\n"
                "version : show app version\n"
                "\nDatabase Commands: \n"
                "create database {name}: make a new database in storage folder\n"
                "drop database {name}: delete a known database from storage folder\n"
                "show databases : list databases in storage folder\n"
                "use database {name} or use {name}: load a known database for use\n"
                "dump database {name}: debug database\n"
                "\nTable Commands:\n"
                "create table {table_name}: crete a table in the database\n"
                "drop table {table_name}: delete a known table in the database\n"
                "describe table {table_name}: show fields and attributes of table\n"
                "UPDATE {table_name} set {set clause} (where {clauses}): show fields and attributes of table\n"
                "show tables: list all tables in the database\n"
                "insert into {table_name} (attribute names) values (row1), (row2),... : add row(s) to the table\n"
                "select {attribute names or *} from {table_name} CLAUSE1 CLAUSE 2 ... : select values from the table\n"
                "\t Clause Types: \n"
                "\t\tWHERE expression (e.g. WHERE age < 20 AND name = \"John\")\n"
                "\t\tLIMIT # (e.g. LIMIT 20)\n"
                "\t\tORDER BY field name (e.g. ORDER BY age)\n"
        };
        return help;
      }
      
    //cachetype: block, row, view...
    static size_t getCacheSize(CacheType aType) {
      return cacheSize[(int)aType];
    }

    static void setCacheSize(CacheType aType, size_t aSize) {
      cacheSize[(int)aType]=aSize;
    }
    
    //cachetype: block, row, view...
    static bool useCache(CacheType aType) {
      return cacheSize[(int)aType]>0;
    }

    static bool useIndex() {return indexing;}
    static bool setUseIndex(bool aState) {
      indexing=aState;
      return indexing;
    }
  };

}

#endif /* Config_h */
