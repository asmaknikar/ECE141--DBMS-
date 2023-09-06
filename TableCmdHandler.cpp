#include "TableCmdHandler.hpp"
#include "Table.hpp"
#include "Timer.hpp"
#include "DataBaseView.hpp"
#include "TableView.hpp"
#include "StringView.hpp"
#include "Filters.hpp"
#include "ParseHelper.hpp"

using namespace std;
namespace ECE141 {

    static TestCommands testTableCalls {
            {Keywords::create_kw, [](Command& aCommand){
                return aCommand.size()>3 && aCommand[1].keyword==Keywords::table_kw;}},
            {Keywords::drop_kw, [](Command& aCommand){
                return aCommand.size()==3 && aCommand[1].keyword==Keywords::table_kw;}},
            {Keywords::show_kw, [](Command& aCommand){
                return aCommand.size()==2 && aCommand[1].keyword==Keywords::tables_kw;}},
            {Keywords::describe_kw, [](Command& aCommand){
                return aCommand.size()==2;}},
            {Keywords::insert_kw, [](Command& aCommand){
                return aCommand.size()>2;}},
            {Keywords::select_kw, [](Command& aCommand){
                return aCommand.size()>2;}},
            {Keywords::delete_kw, [](Command& aCommand) {
                return aCommand.size() > 2; }},
            {Keywords::update_kw, [](Command& aCommand) {
                return aCommand.size() > 4; }},
    };


    TableCmdHandler::TableCmdHandler(Database* aDatabase): table(Table(aDatabase->getName())) {
		db = aDatabase;
	}

	StatusResult TableCmdHandler::executeCommand(Command& aCommand, ViewListener& aViewer) {
        if (!db->holdsValidDB()) return StatusResult(Errors::noDatabaseSpecified);
		if (!testTableCalls[aCommand[0].keyword](aCommand)) return StatusResult(Errors::invalidCommand);
        switch (aCommand[0].keyword) {
          case Keywords::create_kw:
            return TableCmdHandler::createTable(aCommand,aViewer);
          case Keywords::show_kw:
            return  TableCmdHandler::showTables(aCommand, aViewer);
          case Keywords::describe_kw:
            return TableCmdHandler::describeTable(aCommand, aViewer);
          case Keywords::insert_kw:
            return TableCmdHandler::insertRow(aCommand,aViewer);
          case Keywords::drop_kw:
            return TableCmdHandler::dropTable(aCommand, aViewer);
          case Keywords::select_kw:
            return TableCmdHandler::executeSelect(aCommand,aViewer);
          case Keywords::delete_kw:
              return TableCmdHandler::deleteRows(aCommand, aViewer);
          case Keywords::update_kw:
            return TableCmdHandler::executeUpdate(aCommand,aViewer);
          default: return StatusResult(Errors::unknownCommand);
        }
		return StatusResult(Errors::unknownCommand);

	}

    bool TableCmdHandler::canHandle(Command& aCommand) {
        Keywords keyword = aCommand[0].keyword;
        if (kwTypes.find(keyword) != kwTypes.end() &&
            (testTableCalls[aCommand[0].keyword](aCommand))) return true;
        return false;
    }

    StatusResult TableCmdHandler::dropTable(Command& aCommand, ViewListener& aView) {
        Timer t;
        StatusResult theResult;
        std::string theTableName = aCommand[2].data;
        ind rowsAffected = 0;
        theResult = db->removeTable(theTableName,rowsAffected);
        DataBaseView dbView = DataBaseView(true, rowsAffected, t.elapsed());
        aView(dbView);
        return theResult;
    }

	StatusResult TableCmdHandler::createTable(Command& aCommand, ViewListener& aViewer) {
        Timer t;
		std::string theTableName = aCommand[2].data;
		if (db->tableExists(theTableName)) return StatusResult(Errors::tableExists);

        AttributeList theAttributeList;
        Command theAttributeCommand;
        std::string commandString;
        std::vector<Token>::iterator theCommandIterator;
        int open_parens = 0;
        int close_parens = 0;
        for (auto& token : aCommand) {
            if (token.data == "(") open_parens++;
            if (token.data == ")") close_parens++;
        }
        if (!(open_parens == close_parens) || open_parens + close_parens < 2) {
            return StatusResult(Errors::missingParenthesis);
        }
        for(auto theCommandIterator = aCommand.begin()+4; theCommandIterator != aCommand.end()-1; theCommandIterator++ )    {
          if(theCommandIterator->data==",") {
              Attribute a = Attribute(theAttributeCommand);
              if (a.isValid()) theAttributeList.push_back(a);
              else return StatusResult(Errors::invalidAttribute);
              theAttributeCommand.clear();
            }
          else {
              theAttributeCommand.push_back(*theCommandIterator);
              commandString += theCommandIterator->data + " ";
          }
        };
        if (theAttributeCommand.size() > 0) {
            Attribute a = Attribute(theAttributeCommand);
            if (a.isValid()) theAttributeList.push_back(a);
            else {
                return StatusResult(Errors::invalidAttribute);
            }
        }
        Schema_ptr theSchema = std::make_shared<Schema>(theAttributeList,theTableName,true);
        table = Table(theTableName,theSchema);
        StatusResult theResult;
        if( (theResult= db->addTable(std::make_shared<Table>(table))) ){
            DataBaseView dbView = DataBaseView(true, 1, t.elapsed());
            aViewer(dbView);
        };
		return theResult;
	}

    StatusResult TableCmdHandler::showTables(Command &aCommand, ViewListener &aViewListener) {
        Timer t = Timer();
        AttributeList attList;
        std::string tableHeader = "Tables_in_" + db->getName();
        Attribute a = Attribute(tableHeader, DataTypes::varchar_type, 20);
        attList.emplace_back(a);
        Schema_ptr s = std::make_shared<Schema>(attList, "TableView");
        Table theTablesTabularViewTable("TableView", s);
        StringList names = db->getTableNames();
        for (std::string& tableName : names) {
            Row_ptr theRow = std::make_shared<Row>();
            theRow->set(tableHeader, tableName);
            theTablesTabularViewTable.addRow(std::move(theRow));
        };
        TableView theTableView = TableView(theTablesTabularViewTable,t.elapsed());
        aViewListener(theTableView);
        return StatusResult();
    }

    StatusResult TableCmdHandler::describeTable(Command &aCommand, ViewListener &aView) {
      Timer t = Timer();
      std::string theTableName = aCommand[1].data;
      Table theTableDescribeTabularViewTable = Table("TablesView");
      if (!db->tableExists(theTableName)) return StatusResult(Errors::unknownTable);
      Table_ptr theTable = db->getTableByName(theTableName,false);
      theTable->schema->createView(aView,t.elapsed());
      return StatusResult();
    }

    StatusResult TableCmdHandler::insertRow(Command &aCommand, ViewListener &aViewer) {
      Timer t;
      CommandTokenReader theCommandReader(aCommand);
      ParseHelper theHelper(theCommandReader);
      StatusResult theResult;
      if (!theCommandReader.skipIf(Keywords::insert_kw) || !theCommandReader.skipIf(Keywords::into_kw)) return StatusResult(Errors::invalidCommand);
      std::string theTableName;
      if (theResult) theResult = theHelper.parseTableName(theTableName);
      
      Table_ptr theTable;
      if (!(theResult = getTable(theTableName, theTable))) 
          return theResult;

      StringList theRowHeaders;
      theResult = theHelper.parseIdentifierList(theRowHeaders);
      if(!theTable->schema->checkContains(theRowHeaders)) return StatusResult(Errors::invalidAttribute);
      RowKeyValues theSkippedValues;\
      // false
      if (!theTable->schema->getRemainingDefaults(theRowHeaders,theSkippedValues)) return StatusResult(Errors::invalidArguments);
      if(!theCommandReader.skipIf(Keywords::values_kw)) return StatusResult(Errors::invalidCommand);

      int rowsAdded = 0;
      Rows theRows;
      while (theResult && theCommandReader.remaining() > 1) {
        if(!theCommandReader.skipIf(left_paren)) return StatusResult(Errors::invalidArguments);
        Row_ptr theInsertRow = std::make_shared<Row>(theTable->schema->getHashedName());
        for (const std::string& rowHeader: theRowHeaders){
          // if we run out before parsing all the attributes
          if (theCommandReader.remaining()<2 || theCommandReader.skipIf(right_paren)
              || theCommandReader.skipIf(left_paren) || theCommandReader.skipIf(comma)) 
              return StatusResult(Errors::invalidArguments);
          // need to check whether it fits the schema
          Attribute theAttribute = theTable->schema->getAttribute(rowHeader);
          ValueOpt theOptValue = theAttribute.makeValue(theCommandReader.current().data);
          if (!theOptValue) return StatusResult(Errors::invalidAttribute);
          theCommandReader.next();
          Value theValue = theOptValue.value();
          theInsertRow->set(rowHeader, theValue);
          if (theAttribute.isPrimaryKey()) theInsertRow->setID(std::get<int>(theValue));
          // comma or left paren after
          if (!(theCommandReader.skipIf(comma) || theCommandReader.skipIf(right_paren))) return StatusResult(Errors::invalidArguments);
        }
        theInsertRow->set(theSkippedValues);
        rowsAdded++;
        theRows.push_back(theInsertRow);
        theCommandReader.skipIf(comma);
      }
      db->addRows(theTableName, theRows);
      DataBaseView dbView = DataBaseView(true, rowsAdded, t.elapsed());
      aViewer(dbView);
      return StatusResult();
    }

    StatusResult TableCmdHandler::executeSelect(Command &aCommand, ViewListener &aViewer) {
      Timer t;
      StatusResult theResult;
      CommandTokenReader theCommandTokenReader(aCommand);
      ParseHelper theParserHelper(theCommandTokenReader); //todo use and make less clunky
      theCommandTokenReader.next(); // skip "select"
      // get columns to select
      bool allColumnsFlag = false;
      StringList theRowHeaders;
      if (theCommandTokenReader.current().data[0] == star) {
          allColumnsFlag = true;
          theCommandTokenReader.next();
      }
      else theParserHelper.parseIdentifierList(theRowHeaders);
      //check the table
      std::string theTableName;
      theResult = theParserHelper.parseFromTable(theTableName);
      Table_ptr theTable;
      if (theResult && (theResult = getTable(theTableName, theTable))) {
          // parse join
        Table_ptr theTable = db->getTableByName(theTableName);
        Clauses theClauses;
        if (theCommandTokenReader.more() && Helpers::in_array(gJoinTypes, theCommandTokenReader.current().keyword)) {
            Join theJoin;
            if ((theResult = theParserHelper.parseJoin(theJoin))) {
                if (db->tableExists(theJoin.table)) {
                    theClauses.setJoin(theJoin);
                }
                else theResult.error = Errors::unknownTable;
            };
        }
        else if (!allColumnsFlag && !theTable->schema->checkContains(theRowHeaders)) return Errors::invalidAttribute;
        // get the clauses and build the query
        if (theResult && (theResult = theParserHelper.parseClauses(theTable->schema, theClauses))) {
            DBQuery theDbQuery = allColumnsFlag ? DBQuery(theClauses, allColumnsFlag, theTable->schema) :
                DBQuery(theClauses, theRowHeaders, theTable->schema);
            Table theTable("Selected Rows");
            if ((theResult = db->selectQuery(theDbQuery, theTable))) {
                TableView theView = TableView(theTable, t.elapsed());
                aViewer(theView);
                return StatusResult();
            }
        }
      }
      return theResult; // get a better error
      
    }
    StatusResult TableCmdHandler::deleteRows(Command& aCommand, ViewListener& aViewer) {
        Timer t;
        StatusResult theResult;
        CommandTokenReader theCommandTokenReader(aCommand);
        ParseHelper theParserHelper(theCommandTokenReader); //todo use and make less clunky
        if (!theCommandTokenReader.skipIf(Keywords::delete_kw))
            return theResult.error = Errors::invalidCommand;
        //check the table
        std::string theTableName;
        theParserHelper.parseFromTable(theTableName);
        Table_ptr theTable;
        if ((theResult = getTable(theTableName, theTable))) {
            // get the clauses and build the query
            Clauses theClauses;
            if ((theResult = theParserHelper.parseClauses(theTable->schema, theClauses))) {
                DBQuery theDbQuery(theClauses, true, theTable->schema);
                ind deletedCount = 0;
                theResult = db->deleteQuery(theDbQuery, deletedCount);
                DataBaseView theView(theResult, deletedCount, t.elapsed());
                aViewer(theView);
            }
        }
        return theResult;
    }

    StatusResult TableCmdHandler::executeUpdate(Command &aCommand, ViewListener &aViewer) {
      Timer t;
      CommandTokenReader theCommandTokenReader(aCommand);
      ParseHelper theParserHelper(theCommandTokenReader);
      
      if(!theCommandTokenReader.skipIf(Keywords::update_kw)) return Errors::invalidCommand;

      std::string theTableName;
      StatusResult theResult = theParserHelper.parseTableName(theTableName);
      Table_ptr theTable;
      if (!db->tableExists(theTableName)) return Errors::unknownTable;
      theTable = db->getTableByName(theTableName);

      if(!theCommandTokenReader.skipIf(Keywords::set_kw)) return Errors::invalidCommand;
      Expression theSetExpression;
      Schema_ptr theSchema = theTable->schema;
      if ((theResult = theParserHelper.parseExpression(*theSchema, theSetExpression))) {
          if (theSchema->checkContains({ theSetExpression.lhs.name }) &&
              !theSchema->getAttribute(theSetExpression.lhs.name).isAutoIncrement()) { // can't change an autoincrement value
              ValueOpt theVal = theSchema->getAttribute(theSetExpression.lhs.name).makeValue(theSetExpression.rhs.name);
              if (theVal) {
                  RowKeyValues theReplacement = { {theSetExpression.lhs.name,theVal.value()}};
                  Clauses theClauses;
                  if ((theResult = theParserHelper.parseClauses(theSchema, theClauses))) {
                      DBQuery theQuery(theClauses, true, theSchema);
                      ind theCount = 0;
                      theResult = db->updateQuery(theQuery, theCount, theReplacement);
                      DataBaseView theView(true, theCount, t.elapsed());
                      aViewer(theView);
                  }
              }
          }
          else theResult.error = Errors::illegalIdentifier;
      }
      return theResult;
    }

    StatusResult TableCmdHandler::getTable(TableName aTableName, Table_ptr& aTable) {
      if (!db->tableExists(aTableName.table)) 
          return Errors::unknownTable;
      aTable = db->getTableByName(aTableName.table);
      return aTable? StatusResult(): StatusResult(Errors::readError);
    }


}