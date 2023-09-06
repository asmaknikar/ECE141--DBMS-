#include "StringView.hpp"
#include "Config.hpp"
#include "AppCmdHandler.hpp"

namespace ECE141 {
		 StatusResult AppCmdHandler::executeCommand(Command& aCommand, ViewListener& aViewer){
			 switch (aCommand[0].keyword) {
				 case Keywords::about_kw: {
					 StringView theStringView = StringView(Config::getMembers());
					 aViewer(theStringView);
					 return Errors::noError;
				 }
				 case Keywords::quit_kw: {
					 StringView theStringView = StringView(Config::getExitMessage());
					 aViewer(theStringView);
					 return Errors::userTerminated;
				 }
				 case Keywords::version_kw: {
					 StringView theStringView = StringView(Config::getVersion());
					 aViewer(theStringView);
					 return Errors::noError;
				 }
				 case Keywords::help_kw: {
					 StringView theStringView = StringView(Config::getHelpMessage());
					 aViewer(theStringView);
					 return Errors::noError;
				 }
				 default:
					 return StatusResult(Errors::unknownCommand);
				 }
			}

		 bool AppCmdHandler::canHandle(Command& aCommand) {
			 if (aCommand.size() == 1) {
				 Keywords keyword = aCommand[0].keyword;
				 for (auto k : kwTypes) {
					 if (keyword == k) return true;
				 }
			 }
			 return false;
		 }

}