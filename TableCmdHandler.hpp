#pragma once

#include <set>
#include "Handler.hpp"
#include "Database.hpp"
#include "ParseHelper.hpp"

namespace ECE141 {
    class TableCmdHandler : public Handler
	{
	public:
		~TableCmdHandler() {};
		TableCmdHandler(Database* aDatabase);
		StatusResult executeCommand(Command& aCommand, ViewListener& aViewer) override;
		bool canHandle(Command& aCommand) override;
		StatusResult createTable(Command& aCommand, ViewListener& aViewer);
		StatusResult showTables(Command& aCommand, ViewListener& aViewer);
		StatusResult describeTable(Command& aCommand, ViewListener& aViewer);
		StatusResult dropTable(Command& aCommand, ViewListener& aViewer);
		StatusResult deleteRows(Command& aCommand, ViewListener& aViewer);
  private:
		Table table;
		Database* db;
    
		const Keywords identifier = Keywords::table_kw;
		const std::set<Keywords> kwTypes {
			Keywords::create_kw,
			Keywords::drop_kw,
			Keywords::show_kw,
			Keywords::describe_kw,
			Keywords::delete_kw,
			Keywords::insert_kw,
			Keywords::select_kw,
			Keywords::update_kw,
      };

      StatusResult insertRow(Command &vector, ViewListener &function);

      StatusResult executeSelect(Command &aCommand, ViewListener &aViewer);

      StatusResult executeUpdate(Command &aCommand, ViewListener &aViewer);

      StatusResult getTable(TableName aTableName, Table_ptr& aTable);
    };
}

