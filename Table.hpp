#pragma once
#include <vector>
#include <map>
#include <memory>
#include <sstream>

#include "BasicTypes.hpp"
#include "Helpers.hpp"
#include "Handler.hpp"
#include "Row.hpp"
#include "Schema.hpp"

namespace ECE141 {
	using Rows = std::vector<Row_ptr>;
	using RowVisitor = std::function<bool(const Row& row)>;
	struct TableField {
      TableField(){};
      TableField(std::string aFieldName) :fieldName(aFieldName){}
		std::string fieldName;
		std::string table;
	};
	struct Order {
		TableField orderingColumn;
		bool descending = false;
	};
	class Table
	{
	public:
		//Table() {};
		Table(std::string aName):name(aName), schema(std::make_shared<Schema>(aName)),rows() {};

		Table(std::string aName, Schema_ptr aSchema) : name(aName), schema(aSchema) {
			if (aSchema) {
				primaryKey = aSchema->getAutoIncrementHeader();
			}
				
		};
		Table(const Table& aCopy) : name(aCopy.name), schema(std::make_shared<Schema>(*aCopy.schema)), 
			rows(aCopy.rows),primaryKey(aCopy.primaryKey) {
			if (aCopy.rowIndices) rowIndices = std::make_shared<IndIndex>(*aCopy.rowIndices);
		};
		bool each(const RowVisitor& aVisitor, ind start = 0);
		std::string getName() { return name; };
		ind getStartBlock() { return schema->getStartBlock(); }
		bool addRow(Row_ptr aRow, ind aRowBlock = 0);
		bool removeRowsByKeys(IndexList aKeyList);
		bool removeRowByKey(ind aKey);
		ind setAutoIncrementKey(Row_ptr aRow);
		bool sortRows(Order anOrder);
		IndexList getAllBlockNums();

		std::string name;
		Schema_ptr schema;
		Rows rows;
		std::shared_ptr<IndIndex> rowIndices; // row primary key -> block number
		IndexList changedRows;
		OptString primaryKey;
	};
	using Table_ptr = std::shared_ptr<Table>;

}

