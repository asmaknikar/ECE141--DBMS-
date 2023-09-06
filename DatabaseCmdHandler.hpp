#pragma once

#include <set>
#include "Handler.hpp"
#include "Database.hpp"
namespace ECE141 {
	class DatabaseCmdHandler : public Handler {
	public:
		~DatabaseCmdHandler() {};
		DatabaseCmdHandler() {};
		StatusResult executeCommand(Command& aCommand, ViewListener& aViewer) override;
		bool canHandle(Command& aCommand) override;

		StatusResult createDatabase(Command& aCommand, ViewListener& aViewer);
		StatusResult getDatabases(ViewListener& aViewer);
		StatusResult getIndexes(ViewListener& aViewer);
		StatusResult showIndex(Command& aCommand, ViewListener& aViewer);
		StatusResult dropDatabase(Command& aCommand, ViewListener& aViewer);
		StatusResult useDatabase(Command& aCommand, ViewListener& aViewer);
		StatusResult dump(Command& aCommand, ViewListener& aViewer);
		bool exists(std::string aName);
	private:
		friend class AppController;
        Database currentDB;
		static const uint32_t maxDBNameLength = 100;
		const Keywords identifier = Keywords::database_kw;
		const std::set<Keywords> kwTypes {
			Keywords::create_kw,
			Keywords::drop_kw,
			Keywords::show_kw,
			Keywords::use_kw,
			Keywords::dump_kw,
		};
	};
}