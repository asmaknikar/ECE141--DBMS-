//
//  Database.cpp
//  PA2
//
//  Created by rick gessner on 2/27/23.
//

#include <string>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <map>
#include <bitset>
#include "Database.hpp"
#include "Indices.cpp"
#include "Config.hpp"

namespace ECE141 {
    Database::Database(): Storable(Database::Header_Char), name(""), changed(false),
        tableBlockIndex(IndIndex(Schemas_Block)), 
        store(std::make_unique<Storage>(name,OpenFile())) {}

    Database::Database(const std::string& aName, AccessMode aMode)
    : Storable(Database::Header_Char),name(aName), changed(true), tableBlockIndex(IndIndex(Schemas_Block)),
        store(std::make_unique<Storage>(name, aMode)) {
        if (std::holds_alternative<CreateFile>(aMode)) store->save(*this, Database::Metadata_Block);
    }

    Database::Database(const Database& aDB): Storable(Database::Header_Char),
        tableBlockIndex(aDB.tableBlockIndex),
        store(std::make_unique<Storage>(name, OpenFile())) {
        name = aDB.name;
        tables = aDB.tables;
    }

    Database& Database::operator=(const Database& aCopy) {
        tableBlockIndex = aCopy.tableBlockIndex;
        store = std::make_unique<Storage>(aCopy.name,OpenFile());
        name = aCopy.name;
        tables = aCopy.tables;
        return *this;
    }

    Database::~Database() {
        if (changed) {
            store->save(*this, 0); // confirm meta-data saved
            saveTables(); // just saves indices
        }
    }

    std::string Database::to_string() const{
        return name;
    }

    StatusResult Database::encode(std::ostream& anOutput) const{
        Block metadata(BlockType::metadata_block, to_string());
        return metadata.write(anOutput);
    }

    StatusResult Database::decode(const Block& aBlock) {
        if (!aBlock.header.isBlockType(blockChar)) return StatusResult(Errors::readError);
        std::stringstream payload(aBlock.payload);
        payload >> name;
        store = std::make_unique<Storage>(name, OpenFile());
        return loadTables();
    }

    bool Database::loadRows(std::string aTableName) {
        if (!tableExists(aTableName)) return false;
        Table_ptr theTable = tables[aTableName];
        if (theTable->rows.size()) return true; // already been loaded
        bool theResult = true;
        ind startBlock = theTable->schema->getStartBlock();
        Block firstBlock;
        if ((theResult = store->readBlock(startBlock, firstBlock))) {
            Row r;
            if (firstBlock.header.type == (char)BlockType::indices_block) {// index of rows exists 
                IndIndex theIndex;
                if ((theResult = loadIndex(theIndex, startBlock))) {
                    theTable->rowIndices = std::make_shared<IndIndex>(theIndex);
                    IndIndex::BasicMap theKeyVals;
                    theIndex.getMap(theKeyVals);
                    theResult = store->indexValueEach(theKeyVals, [&](const Block& aBlock, ind aBlockNum) {
                        bool valid = r.decode(aBlock);
                        theTable->rows.push_back(std::make_shared<Row>(r));
                        return valid;
                        });
                }
            }
            else { // linked list of rows
                store->linkedListEach(startBlock, [&](const Block& aBlock, ind) {
                    bool valid = r.decode(aBlock);
                    theTable->rows.push_back(std::make_shared<Row>(r));
                    return valid;
                    });
            }
        }
        return theResult;
    }

    bool Database::saveRows(std::string aTableName) {
        Table_ptr theTable = getTableByName(aTableName,false);
        bool theResult = true;;
        if ((theResult = store->save(*(theTable->schema), tableBlockIndex.getIndexValue(theTable->schema->getHashedName())))) {
            if (theTable->rowIndices) {
                theResult = saveIndex(*theTable->rowIndices, theTable->getStartBlock());
                for (const ind& index : theTable->changedRows) {
                    store->save(*theTable->rows[index], theTable->rowIndices->getIndexValue(theTable->rows[index]->getID()));
                }
                theTable->changedRows.clear();
            }
            else {
                ind currInd = theTable->getStartBlock();
                for (auto const& row : theTable->rows) {
                    if (currInd < 1) return true;
                    store->save(*row, currInd);
                    currInd = row->getNextBlockNum();
                }
            }
            if (!Config::useCache(CacheType::rows)) { // dont hold onto rows unless we want to use a cache
                theTable->rows.clear();
                if (theTable->rowIndices) theTable->rowIndices = nullptr;
            }
        }
        return theResult;
    }

    bool Database::holdsValidDB() const {
        return getName() != "";
    }

    bool Database::tableExists(std::string aTableName) const {
        return tables.count(aTableName) > 0;
    }

    StatusResult Database::addTable(Table_ptr aTable) {
        StatusResult theResult;
        ind schemaBlockNum = store->getFreeBlock();
        Schema_ptr theSchema = aTable->schema;
        if (tableBlockIndex.addPair(theSchema->getHashedName(), schemaBlockNum)) {
            if (Config::useIndex()) {
                ind rowIndexBlockNum = store->getFreeBlock();
                theSchema->setStartBlock(rowIndexBlockNum);
                aTable->rowIndices = std::make_shared<IndIndex>(theSchema->getHashedName());
                if (!(theResult = store->save(*aTable->rowIndices, rowIndexBlockNum)))
                    store->returnFreeBlock(rowIndexBlockNum);
            }
            if (!(theResult = store->save(*theSchema, schemaBlockNum))) 
                store->returnFreeBlock(schemaBlockNum);
            else {
                tables.insert({ theSchema->getName(),aTable });
                theResult = saveTables();
            }

        }
        else theResult.error = Errors::cantCreateIndex;
        return theResult;
    }

    StatusResult Database::removeTable(std::string aTableName, ind& aRowsAffected) {
        StatusResult theResult;
        if (tableExists(aTableName)) {
            Table_ptr theTable = getTableByName(aTableName);
            // free schema block
            ind schemaBlock = tableBlockIndex.getIndexValue(theTable->schema->getHashedName());
            if (store->markBlockAsFree(schemaBlock)) {
                //remove from schema index
                tableBlockIndex.removeIndex(theTable->schema->getHashedName()); 
                // delete all associated blocks
                IndIndex theDeleted;
                if (loadIndex(theDeleted, theTable->getStartBlock())) { // if it has an index
                    IndIndex::BasicMap theBlocks;
                    theDeleted.getMap(theBlocks);
                    for (auto& [_, block] : theBlocks) {
                        if (!store->markBlockAsFree(block)) {
                            theResult.error = Errors::writeError;
                            break;
                        }
                        aRowsAffected++;
                    }
                }
                else { // no index, uses linked list of rows
                    loadRows(aTableName); //  replace this with block iterator
                    if (theTable->rows.size() > 0 && theTable->getStartBlock() > 1) {
                        ind theBlockNum = theTable->getStartBlock();
                        store->markBlockAsFree(theBlockNum); // first in block LL
                        aRowsAffected++;
                        theTable->each([&](const Row& row) { // rest of LL blocks
                            if ((theBlockNum = row.getNextBlockNum()) > 1) {
                                store->markBlockAsFree(theBlockNum);
                                aRowsAffected++;
                            }
                            return true;
                            });
                    }
                }

                tables.erase(aTableName);
            }
            else theResult.error = Errors::writeError;
            
        }
        else theResult.error = Errors::unknownTable;
        
      return theResult;
    }

    StatusResult Database::addRows(std::string aTableName, const Rows& aRows) {
        StatusResult theResult;
        Table_ptr theTable = getTableByName(aTableName);
        if (theTable) {
            for (auto& row : aRows) {
                ind insertBlock = store->getFreeBlock();
                theTable->addRow(row, insertBlock);
                theResult = store->save(*row, insertBlock);
            }
            if(!saveRows(aTableName)) theResult.error = Errors::writeError;
        }
        else theResult.error = Errors::unknownTable;
        return theResult;
    }

    StringList Database::getTableNames() {
        StringList tableNames;
        for (auto const& [tableName, _] : tables) {
            tableNames.push_back(tableName);
        }
        return tableNames;
    }

    Table_ptr Database::getTableByName(std::string aTableName, bool aLoadRows){
        if (tableExists(aTableName)) {
            bool valid = true;
            if (aLoadRows)  valid = loadRows(aTableName);
            return valid ? tables[aTableName]: nullptr;
        }
        return nullptr;
    }

    StatusResult Database::saveIndex(IndIndex& anIndex, ind aStartBlock) {
        StatusResult theResult;
        IndIndex_ptr currIndices = std::make_shared<IndIndex>(anIndex);
        ind currBlock = aStartBlock; //first indices
        while (theResult && currIndices->hasChild()) {
            ind theBlock;
            if (!(theBlock = currIndices->getNextBlockId())) {
                theBlock = store->getFreeBlock();
            }
            currIndices->setNextBlockId(theBlock);
            if (!(theResult = store->save(*currIndices, currBlock))) {
                // lost all next associated elements
                store->returnFreeBlock(theBlock);
                currIndices->setNextBlockId(currBlock);
            }
            currBlock = theBlock;
            currIndices = currIndices->getNextIndices();
        }
        theResult = store->save(*currIndices, currBlock);
        return theResult;
    }

    StatusResult Database::loadIndex(IndIndex& anIndex, ind aStartBlock) {
        // loads the index stored in a given block if it exists
        Block b;
        StatusResult theResult;
        if ((theResult = store->readBlock(aStartBlock, b))) {
            if (b.header.type == (char)BlockType::indices_block) {
                anIndex.decode(b);
                IndIndex_ptr currIndex = std::make_shared<IndIndex>(anIndex);
                IndIndex_ptr head = currIndex;
                store->linkedListEach(anIndex.getNextBlockId(), [&](const Block& aBlock, ind aBlockNum) {
                    IndIndex nextIndex;
                    if (!nextIndex.decode(aBlock)) {
                        theResult.error = Errors::readError;
                        return false;
                    }
                    if (currIndex->getNextBlockId() == nextIndex.getNextBlockId()) return false;
                    currIndex->setNextIndices(std::make_shared<IndIndex>(nextIndex));
                    currIndex = currIndex->getNextIndices();
                    return true;
                    });
                anIndex = *head;
            }
            else theResult.error = Errors::readError;

        }
        return theResult;
    }

    StatusResult Database::saveTables() {
        // just save the indices of schemas since all else already written
        StatusResult theResult;
        theResult = saveIndex(tableBlockIndex, Schemas_Block);
        for (const std::string& table : getTableNames()) {
            if (!saveRows(table)) {
                theResult.error = Errors::writeError;
                break;
            }
        }
        return theResult;
    }

    StatusResult Database::loadTables() {
        // only loads in the schemas
        IndIndex theTableIndex;
        StatusResult theResult = loadIndex(theTableIndex, Schemas_Block);
        if (theResult) {
            tableBlockIndex = theTableIndex;
            // load the actual schema from their block index
            if (theResult) {
                tables.clear();
                IndIndex::BasicMap theIndices;
                if (!tableBlockIndex.getMap(theIndices)) return StatusResult(Errors::readError);
                for (auto const& [schemaId, blockNum] : theIndices){
                    Schema tempSchema("temp");
                    store->load(tempSchema, blockNum);
                    Table_ptr tempTable = std::make_shared<Table>(tempSchema.getName(), std::make_shared<Schema>(tempSchema));
                    tables.insert({ tempTable->getName(), tempTable });
                }
            }
        }
        else theResult.error = Errors::readError;
        return theResult;
    }
    
    StatusResult leftJoin(Join& aJoin, Table_ptr aFromTable, Table_ptr aJoinTable, Table& anOutTable) {
        // helper method to do joins
        Table_ptr theLeftTable = (aFromTable->getName() == aJoin.onLeft.table) ? aFromTable : aJoinTable;
        Table_ptr theRightTable = (aFromTable->getName() == aJoin.onRight.table) ? aFromTable : aJoinTable;
        StatusResult theResult;
        if (theLeftTable == theRightTable) theResult.error = Errors::unknownTable;
        Schema theJoinedSchema = Schema::joinSchemas(*aFromTable->schema, *aJoinTable->schema);
        anOutTable.schema = std::make_shared<Schema>(theJoinedSchema);
        aFromTable->each([&](const Row& aFromRow) { // use both visitors through each table
            size_t added = 0;
            aJoinTable->each([&](const Row& anOnRow) {
                Row combinedRow = Row::joinRows(aFromRow, anOnRow);
                if (aFromTable == theLeftTable) {
                    if (aFromRow.getValue(aJoin.onLeft.fieldName) == anOnRow.getValue(aJoin.onRight.fieldName)) {
                        anOutTable.addRow(std::make_shared<Row>(combinedRow));
                        added++;
                    }
                }
                else if (aFromRow.getValue(aJoin.onRight.fieldName) == anOnRow.getValue(aJoin.onLeft.fieldName)) {
                        anOutTable.addRow(std::make_shared<Row>(combinedRow));
                        added++;
                }
                return true;
                });
            if (!added && aFromTable->rows.size()) {
                Row combinedRow = Row::joinRows(aFromRow, aJoinTable->rows[0]->nullCopy());
                anOutTable.addRow(std::make_shared<Row>(combinedRow));
            }
            return true;
            });
        return theResult;
    }

    StatusResult Database::selectQuery(const DBQuery& aQuery, Table& anOutTable) {
        std::string theTableName = aQuery.getTableName();
        Table_ptr theQueryTable = getTableByName(theTableName);
        StatusResult theResult;
        Join theJoin;
        if (aQuery.getJoin(theJoin)) {
            Table_ptr theOnTable = getTableByName(theJoin.table);
            if (theJoin.joinType == Keywords::left_kw) {
                theResult = leftJoin(theJoin, theQueryTable, theOnTable, anOutTable);
            }
            else if (theJoin.joinType == Keywords::right_kw) {
                theResult = leftJoin(theJoin, theOnTable, theQueryTable , anOutTable);
            }
        }
        else {
            ind rowIndex = 0;
            anOutTable.schema = theQueryTable->schema;
            theQueryTable->each([&](const Row& aRow) {
                if (aQuery.Matches(aRow)) anOutTable.addRow(theQueryTable->rows[rowIndex]); 
                rowIndex++;
                return true; 
            });
        }
        theResult = aQuery.applyClauses(anOutTable); // order and limit
        theResult = aQuery.selectColumns(anOutTable);
        return theResult;
    }
    
    StatusResult Database::deleteQuery(const DBQuery& aQuery, ind& aCount) {
        StatusResult theResult;
        std::string theTableName = aQuery.getTableName();
        Table_ptr theQueryTable = getTableByName(theTableName);
        IndexList toDelete;
        if (theQueryTable->rowIndices) {
            theQueryTable->each([&](const Row& aRow) {
                if (aQuery.Matches(aRow)) {
                    ind theBlock = theQueryTable->rowIndices->getIndexValue(aRow.getID());
                    toDelete.push_back(aRow.getID());;
                    store->markBlockAsFree(theBlock);
                }
                return true; 
            });
        } else {
            ind currBlockNum = theQueryTable->getStartBlock();
            theQueryTable->each([&](const Row& aRow) {
                if (aQuery.Matches(aRow)) {
                    toDelete.push_back(aRow.getID());;
                    store->markBlockAsFree(currBlockNum);
                }
                currBlockNum = aRow.getNextBlockNum();
                return true; });
        }
        aCount = toDelete.size();
        if (theQueryTable->removeRowsByKeys(toDelete)) {
            if (!theQueryTable->rowIndices) theResult = saveRows(theTableName) ? Errors::noError : Errors::writeError;
        }
        else theResult.error = Errors::unknownIndex;
        return theResult;
    }

    StatusResult Database::updateQuery(const DBQuery& aQuery,ind& aCount, RowKeyValues& aReplacement) {
        std::string theTableName = aQuery.getTableName();
        //if (!loadRows(theTableName)) return StatusResult(Errors::unknownTable);
        Table_ptr theQueryTable = getTableByName(theTableName);
        ind rowIndex = 0;
        if (theQueryTable->rowIndices) {
            theQueryTable->each([&](const Row& aRow) {
                if (aQuery.Matches(aRow)) {
                    theQueryTable->rows[rowIndex]->set(aReplacement);
                    theQueryTable->changedRows.push_back(rowIndex);
                    aCount++;
                }
                rowIndex++;
                return true; });
        }
        else {
            ind currBlockNum = theQueryTable->getStartBlock();
            theQueryTable->each([&](const Row& aRow) {
                if (aQuery.Matches(aRow)) {
                    theQueryTable->rows[rowIndex]->set(aReplacement);
                    aCount++;
                }
                currBlockNum = aRow.getNextBlockNum();
                rowIndex++;
                return true; });
        }
        return saveRows(theTableName) ? StatusResult() : StatusResult(Errors::writeError);
    }

}
