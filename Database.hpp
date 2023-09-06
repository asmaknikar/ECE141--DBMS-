//
//  Database.hpp
//  PA2
//
//  Created by rick gessner on 2/27/23.
//

#ifndef Database_hpp
#define Database_hpp

#include <stdio.h>
#include "Storage.hpp"
#include "Table.hpp"
#include "Indices.hpp"
#include "DBQuery.hpp"

namespace ECE141 {
    class Database : public Storable {
    public:
        // block 0 always metadata, block 1 always index of schemas
        inline static const ind Metadata_Block = 0;
        inline static const ind Schemas_Block = 1;
        inline static const char Header_Char = 'M';

        Database();
        Database(const Database& aDB);
        Database(const std::string& aName, AccessMode aMode);
        Database& operator=(const Database& aCopy);
        virtual ~Database();

        bool holdsValidDB() const;
        bool tableExists(std::string aTableName) const;

        StatusResult addTable(Table_ptr aTable);
        StatusResult saveTables();
        StatusResult loadTables();
        StatusResult addRows(std::string aTableName, const Rows& aRows);
        StatusResult removeTable(std::string aTableName, ind& aRowsAffected);

        StatusResult selectQuery(const DBQuery& aQuery, Table& anOutTable);
        StatusResult deleteQuery(const DBQuery& aQuery, ind& aCount);
        StatusResult updateQuery(const DBQuery& aQuery, ind& aCount, RowKeyValues& aReplacement);
        
        std::string getName() const { return name; }
        StringList  getTableNames();
        Table_ptr   getTableByName(std::string basicString, bool aLoadRows = true);

    protected:
        
        virtual StatusResult  encode(std::ostream& anOutput) const;
        virtual StatusResult  decode(const Block& aBlock);
        std::string           to_string() const;

        // helper functions
        bool loadRows(std::string aTableName);
        bool saveRows(std::string aTableName);
        StatusResult loadIndex(IndIndex& anIndex, ind aStartBlock);
        StatusResult saveIndex(IndIndex& anIndex, ind aStartBlock);

        std::string     name;
        bool            changed;  //might be helpful, or ignore if you prefer.
        IndIndex        tableBlockIndex;
        std::map<std::string,Table_ptr> tables;
        Storage_ptr store;
        friend class Table;
    };


}
#endif /* Database_hpp */
