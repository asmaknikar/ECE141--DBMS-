//
//  CommandProcessor.cpp
//  ECEDatabase
//
//  Created by rick gessner on 3/30/23.
//  Copyright © 2018-2023 rick gessner. All rights reserved.
//

#include <iostream>
#include <memory>
#include <vector>
#include "AppController.hpp"
#include "Tokenizer.hpp"
#include "Helpers.hpp"
#include "AppCmdHandler.hpp"
#include "DatabaseCmdHandler.hpp"
#include "TableCmdHandler.hpp"
#include "StringView.hpp"

namespace ECE141 {
  
  AppController::AppController() : running{ true } {
      Handler_ptr appCmd; Handler_ptr databaseCmd; Handler_ptr tableCmd;
      appCmd = std::make_shared<AppCmdHandler>(AppCmdHandler());
      handlers.push_back(appCmd);
      databaseCmd = std::make_shared<DatabaseCmdHandler>(DatabaseCmdHandler());
      handlers.push_back(databaseCmd);
      tableCmd = std::make_shared<TableCmdHandler>(
          TableCmdHandler(&std::static_pointer_cast<DatabaseCmdHandler>(databaseCmd)->currentDB));
      handlers.push_back(tableCmd);
  }
  AppController::~AppController() {}
  
  // USE: -----------------------------------------------------
  
  //build a tokenizer, tokenize input, ask processors to handle...
  StatusResult AppController::handleInput(std::istream &anInput,
                                        ViewListener aViewer){
    Tokenizer theTokenizer(anInput);
    StatusResult theResult=theTokenizer.tokenize();
    Command currCommand;
    theResult.value = uint32_t(Errors::noCommandGiven);
    while (theResult && theTokenizer.more()) {      
      Token& token = theTokenizer.current();
      if(token.data[0] == semicolon) { // end of one command
          auto capableHandler = getHandler(currCommand);
          if (capableHandler) theResult = capableHandler.value()->executeCommand(currCommand, aViewer);
          else theResult = StatusResult(Errors::unknownCommand);
          currCommand.clear();
      }
      else currCommand.push_back(token);
      theTokenizer.next();
    }
    if (theResult.error == Errors::userTerminated) running = false;
    if (theResult.value == uint32_t(Errors::noCommandGiven)) theResult = StatusResult(Errors::noCommandGiven);
    if (auto errorMsg = getError(theResult)) {
        StringView errorView = StringView(errorMsg.value());
        aViewer(errorView);
    }
    return theResult;
  }

  std::optional<Handler_ptr> AppController::getHandler(Command& aCommand) {
      for (Handler_ptr handler: handlers) {
          if (aCommand.size() > 0 && handler->canHandle(aCommand))
              return handler;
      }
      return std::nullopt;
  }

  OptStringView AppController::getError(StatusResult &aResult) const {
    if (aResult) return std::nullopt; // no error
    static std::map<ECE141::Errors, std::string_view> theMessages = {
      {Errors::illegalIdentifier, "Illegal identifier"},
      {Errors::unknownIdentifier, "Unknown identifier"},
      {Errors::databaseExists, "Database exists"},
      {Errors::tableExists, "Table Exists"},
      {Errors::syntaxError, "Syntax Error"},
      {Errors::unknownCommand, "Unknown command"},
      {Errors::unknownDatabase,"Unknown database"},
      {Errors::unknownTable,   "Unknown table"},
      {Errors::unknownError,   "Unknown error"},
      {Errors::invalidCommand,   "Invalid command"},
      {Errors::noDatabaseSpecified,   "No Database Specified"},
      {Errors::readError,   "Unable to read from file"},
      {Errors::writeError,   "Unable to write to file"},
      {Errors::invalidAttribute,   "Error 402: Attribute is un-parsable at line 1"},
      {Errors::missingParenthesis,   "Error 114: Input missing a parenthesis at line 1"},
      {Errors::noCommandGiven, "Forgot End Semicolon ...;"}

    };
    std::string_view theMessage="Unknown Error";
    if (theMessages.count(aResult.error)) {
        theMessage = theMessages[aResult.error];
    }
    return theMessage;
  }




}
