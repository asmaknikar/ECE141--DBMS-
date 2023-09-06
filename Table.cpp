#include "Table.hpp"
#include "Filters.hpp"
namespace ECE141 {
    bool Table::each(const RowVisitor& aVisitor,ind start) {
        for (size_t i = start; i < rows.size();i++){
            if (!aVisitor(*rows[i])) return false;
        }  
        return true;
    }
    ind Table::setAutoIncrementKey(Row_ptr aRow) {
        auto aKey = schema->getAutoIncrementHeader();
        ind theID = aRow->getID();
        if (theID != 0) 
            return theID;
        theID = schema->getNextAutoIncrement();
        if (aKey) {
            aRow->set(aKey.value(), (int)theID); // make it a value
            aRow->setID(theID);
        }
        return theID;
    }
    bool Table::removeRowByKey(ind aKey) {
        ind rowIndex = 0;
        bool notRemoved = each([&](const Row& aRow) {
            if (aRow.getID() == aKey) {
                if (!rowIndices) { // if linked list, need to bridge gap
                    if (rowIndex == 0) {
                        schema->setStartBlock(aRow.getNextBlockNum());
                    }
                    else rows[rowIndex - 1]->setNextBlockNum(aRow.getNextBlockNum());
                }
                rows.erase(rows.begin() + rowIndex);
                return false;
            }
            rowIndex++;
            return true;
            });
        
        return !notRemoved;
    }
    bool Table::removeRowsByKeys(IndexList aKeyList) {
        for (auto& key : aKeyList) {
            if (!removeRowByKey(key)) return false;
        }
        return true;
    }
    bool Table::addRow(Row_ptr aRow, ind aBlockNum) {
        ind key = setAutoIncrementKey(aRow);
        if (aBlockNum > 1) { // wants to be stored
            if (rowIndices) {
                rowIndices->addPair(key,aBlockNum);
                changedRows.push_back(rows.size());
            }
            else if (rows.size() == 0) schema->setStartBlock(aBlockNum);
            else rows.back()->setNextBlockNum(aBlockNum);
        }
        rows.push_back(aRow);
        return true;
    }
    bool Table::sortRows(Order anOrder) {
        auto orderingFunc = anOrder.descending ?
            std::function{ [&](const Row_ptr& a, const Row_ptr& b) -> bool {
           return Expression::greaterThan(a->getData().at(anOrder.orderingColumn.fieldName), b->getData().at(anOrder.orderingColumn.fieldName)); } } :
            std::function{ [&](const Row_ptr& a, const Row_ptr& b) -> bool {
            return Expression::lessThan(a->getData().at(anOrder.orderingColumn.fieldName), b->getData().at(anOrder.orderingColumn.fieldName)); } };
        size_t i, j; 
        for (i = 0; i < rows.size(); i++) 
            for (j = i + 1; j < rows.size(); j++)
                if (orderingFunc(rows[j],rows[i])) std::swap(rows[i], rows[j]);
        return true;
    }
    
}