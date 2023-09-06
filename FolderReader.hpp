//
//  FolderReader.hpp
//  Database5
//
//  Created by rick gessner on 4/4/20.
//  Copyright © 2020 rick gessner. All rights reserved.
//

#ifndef FolderReader_h
#define FolderReader_h

#include <string>
#include <filesystem>
#include <fstream>
#include "Config.hpp"

namespace fs =  std::filesystem;

namespace ECE141 {
  
  using FileVisitor = std::function<bool(const std::string&)>;

  class FolderReader {
  public:
            FolderReader(const char *aPath) : path(aPath) {}
    virtual ~FolderReader() {}
    
    virtual bool exists(const std::string &aFilename) {
      std::ifstream theStream(aFilename);
      return !theStream ? false : true;
    }
    
    virtual void each(const std::string &anExt,
                      const FileVisitor &aVisitor) const {
        for(auto const& theFile : fs::directory_iterator(Config::getStoragePath())){
            if (fs::is_regular_file(theFile) && theFile.path().extension() == anExt) {
                aVisitor(theFile.path().filename().string());
            }
        }
    };
    
    std::string path;
  };
  
}

#endif /* FolderReader_h */
