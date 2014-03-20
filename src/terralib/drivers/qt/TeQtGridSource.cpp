#include <TeQtGridSource.h>
#include <TeAppTheme.h>


bool TeQtGridSource::setPortal(TeAppTheme* theme)
{
	if (theme == 0)
	{
		errorMessage_ = "A theme must be passed to the setPortal function";
		return false;
	}

	unsigned int i;
	int k = 0;
	string uid;

	clearPortal();
	uid2PortalRowMap_.clear();
	grid2PortalRowVec_.clear();
	portal2GridRowVec_.clear();
	grid2PortalColVec_.clear();

	theme_ = theme;

	TeDatabase* db = ((TeTheme*)theme_->getTheme())->layer()->database();
	portal_ = db->getPortal();

	// Get the theme tables
	vector<TeTable> tableVec;
	((TeTheme*)theme_->getTheme())->getAttTables(tableVec);

	orderBy_ = " ORDER BY ";
	for (i = 0; i < tableVec.size(); ++i)
	{
		if (i != 0)
			orderBy_ += ",";
		orderBy_ += tableVec[i].name() + "." + tableVec[i].uniqueName();
	}

	string whereClause;
//	string whereClause = ((TeTheme*)theme_->getTheme())->getWhereClause();
	if (whereClause.empty() == false)
		whereClause = " WHERE " + whereClause;
	string q = ((TeTheme*)theme_->getTheme())->sqlJoin() + whereClause + orderBy_;

	if (portal_->query(q) == false)
	{
		errorMessage_ = portal_->getDatabase()->errorMessage();
		delete portal_;
		portal_ = 0;
		return false;
	}

	nRows_ = portal_->numRows();
	nCols_ = portal_->numFields();

	while (portal_->fetchRow())
	{
		uid = portal_->getData(tableVec[0].uniqueName());
		for (i = 1; i < tableVec.size(); ++i)
		{
			string s = portal_->getData(tableVec[i].uniqueName());
			if (s.empty() == true)
				s = " ";

			uid += s;
		}
		uid2PortalRowMap_[uid] = k;

		grid2PortalRowVec_.push_back(k);
		portal2GridRowVec_.push_back(k++);
	}

	// Initialize the relation between the grid columns and the portal columns
	for (k = 0; k < nCols_; ++k)
		grid2PortalColVec_.push_back(k);

	// Set the grid colors vector
	setGridColorsVec();

	emit dataChanged();
	emit setColNamesSignal();
	return true;
}


bool TeQtGridSource::reloadPortal(bool updateGrid2PortalColRelation)
{
	if (portal_ != 0)
		portal_->freeResult();
	else
	{
		portal_ = ((TeTheme*)theme_->getTheme())->layer()->database()->getPortal();
		if (portal_ == 0)
		{
			errorMessage_ = portal_->getDatabase()->errorMessage();
			return false;
		}
	}

	// Reload portal
	string whereClause;
//	string whereClause = ((TeTheme*)theme_->getTheme())->getWhereClause();
	if (whereClause.empty() == false)
		whereClause = " WHERE " + whereClause;

	string q = ((TeTheme*)theme_->getTheme())->sqlJoin() + whereClause + orderBy_;
	if (portal_->query(q) == false)
	{
		errorMessage_ = portal_->getDatabase()->errorMessage();
		delete portal_;
		portal_ = 0;
		return false;
	}

	nRows_ = portal_->numRows();
	nCols_ = portal_->numFields();

	if (updateGrid2PortalColRelation == true)
	{
		grid2PortalColVec_.clear();
		for (int i = 0; i < nCols_; ++i)
			grid2PortalColVec_.push_back(i);
	}

	// Set the grid colors vector
	setGridColorsVec();

	emit dataChanged();
	emit setColNamesSignal();
	return true;
}

void TeQtGridSource::clearPortal()
{
	if (portal_)
	{
		delete portal_;
		portal_ = 0;
	}

	errorMessage_.clear();
	gridColorsVec_.clear();

	nRows_ = nCols_ = 0;
}


bool TeQtGridSource::changePortalQuery(const string& orderBy)
{
	if (orderBy.empty() == true)
	{
		errorMessage_ = "The string that specifies the order must not be empty"; 
		return false;
	}

	orderBy_ = orderBy;

	portal_->freeResult();
	string queryStr = ((TeTheme*)theme_->getTheme())->sqlJoin() + orderBy_;
	if (portal_->query(queryStr) == false)
	{
		errorMessage_ = portal_->getDatabase()->errorMessage();
		delete portal_;
		portal_ = 0;
		return false;
	}

	// Get the theme tables
	vector<TeTable> tableVec;
	((TeTheme*)theme_->getTheme())->getAttTables(tableVec);

	unsigned int i;
	int k = 0;
	string uid;
	while (portal_->fetchRow())
	{
		uid = portal_->getData(tableVec[0].uniqueName());
		for (i = 1; i < tableVec.size(); ++i)
			uid += portal_->getData(tableVec[i].uniqueName());
		uid2PortalRowMap_[uid] = k;

		grid2PortalRowVec_.push_back(k);
		portal2GridRowVec_.push_back(k++);
	}

	// Initialize the relation between the grid columns and the portal columns
	for (k = 0; k < nCols_; ++k)
		grid2PortalColVec_.push_back(k);

	// Set the grid colors vector
	setGridColorsVec();

	emit dataChanged();
	return true;
}

QVariant TeQtGridSource::cell(int row, int col)
{
	QVariant v;
	int portalRow = grid2PortalRowVec_[row];
	int portalCol = grid2PortalColVec_[col];

	if (portal_->fetchRow(portalRow))
		v = portal_->getData(portalCol);

	return v;
}


void TeQtGridSource::setCell(int row, int col, const QVariant &v)
{
	int portalRow = grid2PortalRowVec_[row];
	int portalCol = grid2PortalColVec_[col];

	// Get the name of the attribute that was edited
	string attrNameEdited = ((TeTheme*)theme_->getTheme())->getAttribute(portalCol);

	// Get the name of the theme table whose contents will be changed
	string tableNameToEdit = ((TeTheme*)theme_->getTheme())->getTableName(attrNameEdited);

	string newValue = v.toString().latin1();

	// Find the unique name(primary key) of the table
	TeTable table;
	bool b = ((TeTheme*)theme_->getTheme())->getTable(table, tableNameToEdit);
	if (b == false)
	{
		errorMessage_ = "There is no table with this name";
		return;
	}
	string uniqueName = table.uniqueName();

	// Check if the attribute to be edited is a primary key
	// If positive, do not allow to edit this attribute
	string attr;
	size_t pos = attrNameEdited.find(".");
	if (pos != string::npos)
		attr = attrNameEdited.substr(pos + 1);
	if (attr == uniqueName)
	{
		errorMessage_ = "This attribute cannot be edited because it is a primary key";
		return;
	}

	// Find the name of the register corresponding to the unique name
	string uniqueValue;
	if (portal_->fetchRow(portalRow) == true)
		uniqueValue = portal_->getData(uniqueName);

	// Update the table in the database
	TeDatabase* db = ((TeTheme*)theme_->getTheme())->layer()->database();
	string q = "UPDATE " + tableNameToEdit + " SET " + attrNameEdited + " = '";
	q += newValue + "' WHERE " + uniqueName + " = '" + uniqueValue + "'";
	if (db->execute(q) == false)
	{
		errorMessage_ = db->errorMessage();
		return;
	}

	// Update the portal due the changes in the database
	portal_->freeResult();
	string sqlJoin = ((TeTheme*)theme_->getTheme())->sqlJoin() + orderBy_;
	if (portal_->query(sqlJoin) == false)
	{
		errorMessage_ = db->errorMessage();
		delete portal_;
		return;
	}

	emit dataChanged();
}

string TeQtGridSource::getUniqueId(int gridRow)
{
	string uid;
	unsigned int i;

	vector<TeTable> tableVec;
	((TeTheme*)theme_->getTheme())->getAttTables(tableVec);

	int portalRow = grid2PortalRowVec_[gridRow];
	if (portal_->fetchRow(portalRow))
	{
		for (i = 0; i < tableVec.size(); ++i)
		{
			string s = portal_->getData(tableVec[i].uniqueName());
			if (s.empty() == true)
				s = " ";

			uid += s;
		}
	}
	return uid;
}

string TeQtGridSource::getObjectId(int gridRow)
{
	string objectId;

	vector<TeTable> tableVec;
	((TeTheme*)theme_->getTheme())->getAttTables(tableVec);

	int portalRow = grid2PortalRowVec_[gridRow];
	if (portal_->fetchRow(portalRow))
		objectId = portal_->getData(tableVec[0].uniqueName());

	return objectId;
}

void TeQtGridSource::setGridColorsVec()
{
	if (theme_ == 0)
		return;

	TeColor c;
	QColor qc;

	gridColorsVec_.clear();
	gridColorsVec_.push_back(Qt::white);

	c = ((TeTheme*)theme_->getTheme())->pointingLegend().visual(TePOLYGONS)->color();
	qc = QColor(c.red_,c.green_,c.blue_);
	gridColorsVec_.push_back(qc);

	c = ((TeTheme*)theme_->getTheme())->queryLegend().visual(TePOLYGONS)->color();
	qc = QColor(c.red_,c.green_,c.blue_);
	gridColorsVec_.push_back(qc);

	c = ((TeTheme*)theme_->getTheme())->queryAndPointingLegend().visual(TePOLYGONS)->color();
	qc = QColor(c.red_,c.green_,c.blue_);
	gridColorsVec_.push_back(qc);

//	emit cellColorChanged();
}

QString TeQtGridSource::getPortalInfo(int gridCol)
{
	// Get the portal column relative to the given grid column
	if (gridCol < 0)
		return QString();

	unsigned int portalCol = grid2PortalColVec_[gridCol];

	// Get the attribute name and the table name associated to the portal column
	string attrName = ((TeTheme*)theme_->getTheme())->getAttribute(portalCol);
	string tableName = ((TeTheme*)theme_->getTheme())->getTableName(attrName);

	// Get the representation of a table given an attribute name
	TeTable table;

	if (((TeTheme*)theme_->getTheme())->getTable(table, tableName) == false)
		return QString();

	QString qStr = tr("Table:") + " ";
	qStr += tableName.c_str();
	if(table.tableType() == TeAttrStatic)
		qStr += " - " + tr("Table type: TeAttrStatic");
	else if(table.tableType() == TeAttrExternal)
		qStr += "  " + tr("Table type: TeAttrExternal");
	else if(table.tableType() == TeAttrEvent)
		qStr += " - " + tr("Table type: TeAttrEvent");
	else if(table.tableType() == TeFixedGeomDynAttr)
		qStr += " - " + tr("Table type: TeFixedGeomDynAttr");
	else if(table.tableType() == TeDynGeomDynAttr)
		qStr += " - " + tr("Table type: TeDynGeomDynAttr");

	vector<TeAttribute>& attrVec = portal_->getAttributeList();
	if(attrVec.size() > portalCol)
	{
		qStr += " - " + tr("Column:");
		qStr += " ";
		qStr += attrVec[portalCol].rep_.name_.c_str();

		if(attrVec[portalCol].rep_.type_ == TeSTRING)
		{
			qStr += " - " + tr("Column type: TeSTRING");
			qStr += " - " + tr("Num char:");
			qStr += " ";
			qStr += Te2String(attrVec[portalCol].rep_.numChar_).c_str();
		}
		else if(attrVec[portalCol].rep_.type_ == TeREAL)
			qStr += "  " + tr("Column type: TeREAL");
		else if(attrVec[portalCol].rep_.type_ == TeINT)
			qStr += " - " + tr("Column type: TeINT");
		else if(attrVec[portalCol].rep_.type_ == TeDATETIME)
			qStr += " - " + tr("Column type: TeDATETIME");

		// Get the single name of attrName
		size_t pos = attrName.find(".");
		if (pos != string::npos)
			attrName = attrName.substr(pos + 1);

		// Check if the attribute is editable
		if(table.uniqueName() == attrName)
			qStr += " - " + tr("Column is not editable");
		else
			qStr += " - " + tr("Column is editable");
	}

	return qStr;
}


vector<string> TeQtGridSource::getItemsToggled(int begin, int end)
{
	string item;
	vector<string> itemVec;

	// Get the theme tables
	vector<TeTable> tableVec;
	((TeTheme*)theme_->getTheme())->getAttTables(tableVec);

	for (int i = begin; i <= end; ++i)
	{
		if (portal_->fetchRow(grid2PortalRowVec_[i]))
		{
			item = portal_->getData(tableVec[0].uniqueName());
			for (unsigned j = 1; j < tableVec.size(); ++j)
			{
				string s = portal_->getData(tableVec[j].uniqueName());
				if (s.empty() == true)
					s = " ";

				item += s;
			}
			itemVec.push_back(item);
		}
	}

	return itemVec;
}


bool TeQtGridSource::isDateTimeRegistered(int gridCol)
{
	int portalCol = grid2PortalColVec_[gridCol];
	string fullAttrName = ((TeTheme*)theme_->getTheme())->getAttribute(portalCol);

	size_t pos = fullAttrName.find(".");
	string tableName = fullAttrName.substr(0, pos);
	string attrName = fullAttrName.substr(pos + 1);

	// Get the representation of a table given an attribute name
	TeTable table;
	if (((TeTheme*)theme_->getTheme())->getTable(table, tableName) == false)
		return false;

	if(table.attInitialTime() == attrName || table.attFinalTime() == attrName)
		return true;
	return false;
}


void TeQtGridSource::sortColumns(const vector<int>& gridColsVec, const string& order)
{
	unsigned int i;
	int portalCol;
	string fullAttrName;

	orderBy_ = " ORDER BY ";

	for (i = 0; i < gridColsVec.size(); ++i)
	{
		portalCol = grid2PortalColVec_[gridColsVec[i]];
		fullAttrName = ((TeTheme*)theme_->getTheme())->getAttribute(portalCol);

		if (i != 0)
			orderBy_ += ",";

		orderBy_ += fullAttrName + " " + order;
	}

	reloadPortal();

	// Update the relationships between the grid and portal rows
	for (i = 0; i < grid2PortalRowVec_.size(); ++i)
		grid2PortalRowVec_[i] = i;

	for (i = 0; i < grid2PortalRowVec_.size(); ++i)
		portal2GridRowVec_[i] = i;
}


/*
void TeQtGridSource::sortColumns(const vector<int>& colsVec, const string& order)
{
	unsigned int i;
	int portalCol;
	string fullAttrName;

	string orderBy = " ORDER BY ";

	for (i = 0; i < colsVec.size(); ++i)
	{
		portalCol = grid2PortalColVec_[colsVec[i]];
		fullAttrName = theme_->getAttribute(portalCol);

		if (i != 0)
			orderBy += ",";

		orderBy += fullAttrName + " " + order;
	}

	changePortalQuery(orderBy);
}
*/




int TeQtGridSource::getNextGridRowPointed(int gridRow)
{
	int i;
	unsigned int j;
	string oid, uid;
	map<string, int>& uidStatusMap = ((TeTheme*)theme_->getTheme())->getItemStatusMap();

	// Get the theme tables
	vector<TeTable> tableVec;
	((TeTheme*)theme_->getTheme())->getAttTables(tableVec);

	for (i = gridRow; i < nRows_; ++i)
	{
		if (portal_->fetchRow(grid2PortalRowVec_[i]))
		{
			uid = oid = portal_->getData(tableVec[0].uniqueName());
			for (j = 1; j < tableVec.size(); ++j)
				uid += portal_->getData(tableVec[j].uniqueName());
		}

		int& uidStatus = uidStatusMap[uid];
		if (uidStatus == TePOINTED || uidStatus == TePOINTED_QUERIED)
			break;
		else if (uidStatus == TeDEFAULT)
			uidStatusMap.erase(uid);
	}

	return i;
}


int TeQtGridSource::getPrevGridRowPointed(int gridRow)
{
	int i;
	unsigned int j;
	string oid, uid;
	map<string, int>& uidStatusMap = ((TeTheme*)theme_->getTheme())->getItemStatusMap();

	// Get the theme tables
	vector<TeTable> tableVec;
	((TeTheme*)theme_->getTheme())->getAttTables(tableVec);

	for (i = gridRow; i >= 0; --i)
	{
		if (portal_->fetchRow(grid2PortalRowVec_[i]))
		{
			uid = oid = portal_->getData(tableVec[0].uniqueName());
			for (j = 1; j < tableVec.size(); ++j)
				uid += portal_->getData(tableVec[j].uniqueName());
		}

		int& uidStatus = uidStatusMap[uid];
		if (uidStatus == TePOINTED || uidStatus == TePOINTED_QUERIED)
			break;
		else if (uidStatus == TeDEFAULT)
			uidStatusMap.erase(uid);
	}

	return i;
}


int TeQtGridSource::getNextGridRowQueried(int gridRow)
{
	int i;
	unsigned int j;
	string oid, uid;
	map<string, int>& uidStatusMap = ((TeTheme*)theme_->getTheme())->getItemStatusMap();

	// Get the theme tables
	vector<TeTable> tableVec;
	((TeTheme*)theme_->getTheme())->getAttTables(tableVec);

	for (i = gridRow; i < nRows_; ++i)
	{
		if (portal_->fetchRow(grid2PortalRowVec_[i]))
		{
			uid = oid = portal_->getData(tableVec[0].uniqueName());
			for (j = 1; j < tableVec.size(); ++j)
				uid += portal_->getData(tableVec[j].uniqueName());
		}

		int& uidStatus = uidStatusMap[uid];
		if (uidStatus == TeQUERIED || uidStatus == TePOINTED_QUERIED)
			break;
		else if (uidStatus == TeDEFAULT)
			uidStatusMap.erase(uid);
	}

	return i;
}

int TeQtGridSource::getPrevGridRowQueried(int gridRow)
{
	int i;
	unsigned int j;
	string oid, uid;
	map<string, int>& uidStatusMap = ((TeTheme*)theme_->getTheme())->getItemStatusMap();

	// Get the theme tables
	vector<TeTable> tableVec;
	((TeTheme*)theme_->getTheme())->getAttTables(tableVec);

	for (i = gridRow; i >= 0; --i)
	{
		if (portal_->fetchRow(grid2PortalRowVec_[i]))
		{
			uid = oid = portal_->getData(tableVec[0].uniqueName());
			for (j = 1; j < tableVec.size(); ++j)
				uid += portal_->getData(tableVec[j].uniqueName());
		}

		int& uidStatus = uidStatusMap[uid];
		if (uidStatus == TeQUERIED || uidStatus == TePOINTED_QUERIED)
			break;
		else if (uidStatus == TeDEFAULT)
			uidStatusMap.erase(uid);
	}

	return i;
}
