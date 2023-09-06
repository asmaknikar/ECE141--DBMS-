#pragma once

#ifndef Handler_h
#define Handler_h

#include <vector>
#include <memory>

#include "Tokenizer.hpp"
#include "View.hpp"

namespace ECE141 {
  using Command = std::vector<Token>;
  using CommandCheck = std::function<bool(Command& aCommand)>;
  using TestCommands = std::map<Keywords, CommandCheck>;
  using CommandProcessor = std::map<Keywords,std::function<StatusResult(Command& aCommand, ViewListener& aViewer)>>;

    enum class CommandType {
		appCommand, 
		dbCommand, 
		tableCommand 
	};
	
	class Handler
	{
	public:
		virtual ~Handler() {};
		virtual StatusResult executeCommand(Command& aCommand, ViewListener& aViewer) =0;
		virtual bool canHandle(Command& aCommand)=0;
	};
	using Handler_ptr = std::shared_ptr<Handler>;
}
#endif /*Handler_h*/

