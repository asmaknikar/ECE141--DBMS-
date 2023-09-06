#include <set>
#include "Handler.hpp"
#include "DatabaseCmdHandler.hpp"
#include "Timer.hpp"
#include "FolderReader.hpp"
#include "Config.hpp"
#include "TableView.hpp"
#include "DumpView.hpp"
#include "DataBaseView.hpp"
#include "StringView.hpp"
#include "ParseHelper.hpp"

namespace ECE141 {
    static TestCommands testDatabaseLengthCalls {
            {Keywords::create_kw, [](Command& aCommand){
                return aCommand.size()==3;}},
            {Keywords::drop_kw, [](Command& aCommand){
                return aCommand.size()==3;}},
            {Keywords::show_kw, [](Command& aCommand){
                return aCommand.size()==2 ||
                    (aCommand.size() > 4 && 
                    aCommand[1].keyword == Keywords::index_kw);}},
            {Keywords::use_kw, [](Command& aCommand){
                return aCommand.size()==2 || aCommand.size() == 3;}},
            {Keywords::dump_kw, [](Command& aCommand){
                return aCommand.size()==3;}},
    };

    static TestCommands testDatabaseCommandSanctity {
            {Keywords::create_kw, [](Command& aCommand){
                return aCommand[1].keyword==Keywords::database_kw;}},
            {Keywords::drop_kw, [](Command& aCommand){
                return aCommand[1].keyword==Keywords::database_kw;}},
            {Keywords::show_kw, [](Command& aCommand){
                return aCommand[1].keyword==Keywords::databases_kw ||
                    aCommand[1].keyword == Keywords::indexes_kw ||
                    aCommand[1].keyword == Keywords::index_kw;
                }},
            {Keywords::use_kw, [](Command& aCommand){
                if (aCommand.size() == 3 && aCommand[1].keyword == Keywords::database_kw)
                    aCommand.erase(aCommand.begin() + 1);
                return true;}},
            {Keywords::dump_kw, [](Command& aCommand){
                return aCommand[1].keyword==Keywords::database_kw;}},
    };

	StatusResult DatabaseCmdHandler::executeCommand(Command& aCommand, ViewListener& aViewer) {
      if(!testDatabaseLengthCalls[aCommand[0].keyword](aCommand)) return StatusResult(Errors::invalidCommand);
      if(!testDatabaseCommandSanctity[aCommand[0].keyword](aCommand)) return StatusResult(Errors::invalidCommand);
      if (currentDB.holdsValidDB()) currentDB.saveTables();
      switch (aCommand[0].keyword) {
            case Keywords::create_kw:
              return DatabaseCmdHandler::createDatabase(aCommand, aViewer);
            case Keywords::drop_kw:
              return DatabaseCmdHandler::dropDatabase(aCommand, aViewer);
            case Keywords::show_kw: 
                if (aCommand[1].keyword == Keywords::databases_kw)
                    return DatabaseCmdHandler::getDatabases(aViewer);
                else if (aCommand[1].keyword == Keywords::indexes_kw)
                    return DatabaseCmdHandler::getIndexes(aViewer);
                else return DatabaseCmdHandler::showIndex(aCommand,aViewer);
            case Keywords::use_kw:
                return DatabaseCmdHandler::useDatabase(aCommand, aViewer);
            case Keywords::dump_kw:
                return DatabaseCmdHandler::dump(aCommand, aViewer);
                
            default:
                return StatusResult(Errors::unknownCommand);
            }
			return StatusResult(Errors::unknownCommand);
		}

		bool DatabaseCmdHandler::canHandle(Command& aCommand){
            Keywords keyword = aCommand[0].keyword;
            if (kwTypes.find(keyword)!= kwTypes.end()   &&
            (testDatabaseCommandSanctity[aCommand[0].keyword](aCommand)) ) return true;
            return false;
		}

        StatusResult DatabaseCmdHandler::getDatabases(ViewListener& aView) {
            Timer t = Timer();
            std::string thePath = Config::getStoragePath();
            FolderReader theReader(thePath.c_str());
            AttributeList attList;
            Attribute a = Attribute("Databases", DataTypes::varchar_type, 20);
            attList.emplace_back(a);
            Schema_ptr s = std::make_shared <Schema>(attList, "Databases");
            Table dbNameTable = Table("Databases", s);
            theReader.each(Config::getDBExtension(), [&](const std::string& aName) {
                Row_ptr r = std::make_shared<Row>();
                r->set("Databases", aName.substr(0, aName.find_last_of(".")));
                dbNameTable.addRow(r);
                return true;
                });
            TableView theTableView = TableView(dbNameTable);
            aView(theTableView);
            DataBaseView dbView = DataBaseView(true, dbNameTable.rows.size(), t.elapsed());
            aView(dbView);
            return StatusResult();

        }
        StatusResult DatabaseCmdHandler::getIndexes(ViewListener& aView) {
            Timer t = Timer();
            StatusResult theResult = currentDB.loadTables();
            if (theResult) {
                StringList tableNames = currentDB.getTableNames();
                Table theOutTable("Indexes");
                AttributeList theHeaders{ Attribute("table",DataTypes::varchar_type,15),Attribute("field(s)", DataTypes::varchar_type,15) };
                theOutTable.schema = std::make_shared<Schema>(theHeaders, "Indexes");
                for (const std::string& name : tableNames) {
                    Table_ptr theTable = currentDB.getTableByName(name,false);
                    if (theTable) {
                        Row r;
                        r.set("table", theTable->getName());
                        if (theTable->primaryKey) r.set("field(s)", (theTable->primaryKey).value());
                        else r.set("field(s)", "");
                        theOutTable.addRow(std::make_shared<Row>(r));
                    }
                }
                TableView theTableView(theOutTable,t.elapsed());
                aView(theTableView);
            }
            return theResult;
        }

        StatusResult DatabaseCmdHandler::showIndex(Command& aCommand, ViewListener& aViewer) {
            Timer t = Timer();
            StatusResult theResult;
            CommandTokenReader theCommandReader(aCommand);
            theCommandReader.skipIf(Keywords::show_kw);
            theCommandReader.skipIf(Keywords::index_kw);
            ParseHelper theHelper(theCommandReader);
            StringList fields;
            if ((theResult = theHelper.parseIdentifierList(fields))) {
                std::string tableName;
                if ((theResult = theHelper.parseFromTable(tableName))) {
                    theResult = currentDB.loadTables();
                    Table_ptr theTable = currentDB.getTableByName(tableName, true);
                    if (theResult && theTable) {
                        Table theOutTable("Index");
                        AttributeList theHeaders{ Attribute("key",DataTypes::varchar_type,15),Attribute("block#", DataTypes::int_type) };
                        theOutTable.schema = std::make_shared<Schema>(theHeaders, "Indexes");
                        IndIndex::BasicMap theContents;
                        theTable->rowIndices->getMap(theContents);
                        for (auto const& [key,value]: theContents){
                            Row r;
                            r.set("key", std::to_string(key));
                            r.set("block#", int(value));
                            theOutTable.addRow(std::make_shared<Row>(r));
                        }
                        TableView theTableView(theOutTable, t.elapsed());
                        aViewer(theTableView);
                    }
                    else theResult.error = Errors::unknownTable;
                }
            }
            return theResult;
        }

        StatusResult DatabaseCmdHandler::createDatabase(Command& aCommand, ViewListener& aViewer) {
            Timer t = Timer();
            std::string theDBName = aCommand[2].data;
            if (theDBName.length() > maxDBNameLength) return StatusResult(Errors::nameLengthLimitExceeded);
            if (exists(theDBName)) return StatusResult(Errors::databaseExists);
            currentDB = Database(theDBName, CreateFile());
            DataBaseView dbView = DataBaseView(true, 1, t.elapsed());
            aViewer(dbView);
            return StatusResult();
        }

        StatusResult DatabaseCmdHandler::dropDatabase(Command& aCommand, ViewListener& aViewListener) {
            Timer t = Timer();
            std::string theDBName = aCommand[2].data;
            if (!exists(theDBName)) return StatusResult(Errors::unknownDatabase);
            if (theDBName == currentDB.getName()) currentDB = Database();
            std::string thePath = Config::getDBPath(theDBName);
            std::remove(thePath.c_str());
            std::string temp = "Database " + theDBName + " dropped " + " (" + std::to_string(t.elapsed()) + " sec)";
            StringView sView = StringView(temp);
            aViewListener(sView);
            return StatusResult();
        }

        StatusResult DatabaseCmdHandler::dump(Command& aCommand, ViewListener& aViewListener) {
            Timer t = Timer();
            std::string theDBName = aCommand[2].data;
            if (!exists(theDBName)) return StatusResult(Errors::unknownDatabase);
            Storage theFile(theDBName, OpenFile());
            DumpView theDumpView = DumpView(theFile);
            aViewListener(theDumpView);
            std::string out = "Query OK, " + std::to_string(theDumpView.dump.dumpDataList.size()) + " rows in set (" + std::to_string(t.elapsed()) + "secs)";
            StringView sView = StringView(out);
            aViewListener(sView);
            return StatusResult();
        }

        StatusResult DatabaseCmdHandler::useDatabase(Command& aCommand, ViewListener& aViewListener) {
            Timer t = Timer();
            std::string theDBName = aCommand[1].data;
            if (!exists(theDBName)) return StatusResult(Errors::unknownDatabase);
            StatusResult theResult(Storage(theDBName, OpenFile()).load(currentDB, 0));
            std::string response = "Database changed: " + theDBName + " (" + std::to_string(t.elapsed()) + " sec)";
            StringView dbView = StringView(response);
            aViewListener(dbView);
            return theResult;
        }

        bool DatabaseCmdHandler::exists(std::string aName) {
            std::string thePath = Config::getDBPath(aName);
            FolderReader theReader(thePath.c_str());
            if (theReader.exists(thePath.c_str())) {
                return true;
            }
            return false;
        }
	
}