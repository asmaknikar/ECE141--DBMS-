//
//  AppProcessor.hpp
//  Database5
//
//  Created by rick gessner on 3/30/23.
//  Copyright © 2018-2023 rick gessner. All rights reserved.
//

#ifndef AppController_hpp
#define AppController_hpp

#include <stdio.h>
#include "Config.hpp"
#include "Errors.hpp"
#include "View.hpp"
#include "Tokenizer.hpp"
#include "Handler.hpp"

namespace ECE141 {
  class AppController{
  public:
    
    AppController();
    virtual ~AppController();

      //app api...    
    virtual StatusResult  handleInput(std::istream &anInput,
                                      ViewListener aViewer);
            bool          isRunning() const {return running;}

            OptStringView     getError(StatusResult &aResult) const;
            std::optional<Handler_ptr> getHandler(Command& aCommand);
    bool running;
    std::vector<Handler_ptr> handlers={};

  };
  
}

#endif /* AppController_hpp */
