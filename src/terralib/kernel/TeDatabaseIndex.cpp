#include "TeDatabaseIndex.h"

TeDatabaseIndex::TeDatabaseIndex()
{
}

TeDatabaseIndex::~TeDatabaseIndex()
{
}

void TeDatabaseIndex::setIndexName(const std::string& name)
{
	_indexName = name;
}

std::string TeDatabaseIndex::getIndexName()
{
	return _indexName;
}

void TeDatabaseIndex::setColumn(const std::string& column)
{
	_indexColumns.clear();
	_indexColumns.push_back(column);
}

void TeDatabaseIndex::setColumns(const std::vector<std::string>& columns)
{
	_indexColumns = columns;
}

std::vector<std::string> TeDatabaseIndex::getColumns()
{
	return _indexColumns;
}

std::string TeDatabaseIndex::getCSVColumns()
{
	std::string csvList = "";

	if(_indexColumns.empty())
	{
		return csvList;
	}

	csvList = _indexColumns[0];
	for(unsigned int i = 1; i < _indexColumns.size(); ++i)
	{
		csvList += "," + _indexColumns[i];
	}

	return csvList;
}

void TeDatabaseIndex::setIsPrimaryKey(bool isPk)
{
	_isPrimaryKey = isPk;
}

bool TeDatabaseIndex::isPrimaryKey()
{
	return _isPrimaryKey;
}
