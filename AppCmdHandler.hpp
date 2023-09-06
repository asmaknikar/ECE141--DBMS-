#pragma once
#include "Handler.hpp"
namespace ECE141 {
	class AppCmdHandler : public Handler {
	public:
		~AppCmdHandler() {};
		AppCmdHandler() {};
		StatusResult executeCommand(Command& aCommand, ViewListener& aViewer) override;
		bool canHandle(Command& aCommand) override;

	private:
		const Keywords kwTypes[4] = {
			Keywords::about_kw,
			Keywords::quit_kw,
			Keywords::version_kw,
			Keywords::help_kw };
	};
}