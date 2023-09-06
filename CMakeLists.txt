cmake_minimum_required(VERSION 3.10)
project(SP23Database)

set(CMAKE_CXX_STANDARD 17)

include_directories(./)

add_executable(SP23Database
        main.cpp
        AppController.cpp
        AppController.hpp
        BasicTypes.hpp
        BlockIO.hpp
        BlockIO.cpp
        Config.hpp
        Errors.hpp
        Helpers.hpp
        keywords.hpp
        Scanner.hpp
        ScriptRunner.hpp
        TestAutomatic.hpp
        TestManually.hpp
        TestSequencer.hpp
        Timer.hpp
        Tokenizer.cpp
        Tokenizer.hpp
        Row.hpp Row.cpp
        Attribute.hpp Attribute.cpp
        Schema.hpp Schema.cpp
        Storage.cpp Storage.hpp
        TableCmdHandler.cpp
        Handler.hpp
        DatabaseCmdHandler.cpp DatabaseCmdHandler.hpp
        AppCmdHandler.cpp AppCmdHandler.hpp
        View.hpp Database.cpp  Table.hpp
        FolderReader.hpp Database.hpp Dump.cpp Dump.hpp
        OutputValidator.hpp
        Remoting.hpp
        ScriptMaker.hpp
        DumpView.cpp DumpView.hpp DataBaseView.cpp Table.cpp
        Faked.cpp Faked.cpp Indices.cpp Filters.cpp ParseHelper.cpp CommandTokenReader.cpp Clauses.cpp Clauses.hpp Clauses.cpp)