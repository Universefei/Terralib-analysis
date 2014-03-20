#ifndef TEQTGRIDSOURCE_H
#define TEQTGRIDSOURCE_H

#include <TeQtDataSource.h>
#include "../../kernel/TeDatabase.h"

#include <string>

#include <qcolor.h>

class TeAppTheme;

using namespace std;


class TL_DLL TeQtGridSource : public TeQtDataSource
{
	Q_OBJECT

public:
	TeQtGridSource() : theme_(0), portal_(0), nRows_(0), nCols_(0)
	{ }

	~TeQtGridSource() {}

	bool setPortal(TeAppTheme* theme);
	bool reloadPortal(bool updateGrid2PortalColRelation = false);

	TeAppTheme* getTheme() const { return theme_; }

	bool changePortalQuery(const string& orderBy);

	string& getOrderBy()
	{ return orderBy_; }

	void clearPortal();

	QVariant cell(int row, int col);
	void setCell(int row, int col, const QVariant &text);
		
	string getErrorMessage() const { return errorMessage_; }

	int numRows() const { return nRows_; }
	int numCols() const { return nCols_; }

	string getUniqueId(int gridRow);
	string getObjectId(int gridRow);
	QString getPortalInfo(int gridCol);

	vector<QColor>& gridColorsVec() { return gridColorsVec_; }

	map<string, int>& uid2PortalRowMap() { return uid2PortalRowMap_; }
	vector<int>& grid2PortalRowVec() { return grid2PortalRowVec_; }
	vector<int>& portal2GridRowVec() { return portal2GridRowVec_; }
	vector<int>& grid2PortalColVec() { return grid2PortalColVec_; }

	vector<string> getItemsToggled(int begin, int end);
	bool isDateTimeRegistered(int gridCol);
	void sortColumns(const vector<int>& gridColsVec, const string& order);

	int getNextGridRowPointed(int gridRow);
	int getPrevGridRowPointed(int gridRow);
	int getNextGridRowQueried(int gridRow);
	int getPrevGridRowQueried(int gridRow);

signals:
	void setColNamesSignal();

protected slots:
	void setGridColorsVec();

protected:
	TeAppTheme* theme_;
	TeDatabasePortal* portal_;
	mutable string errorMessage_;
	vector<QColor> gridColorsVec_;
	map<string, int> uid2PortalRowMap_;
	vector<int> grid2PortalRowVec_;
	vector<int>	portal2GridRowVec_;
	vector<int> grid2PortalColVec_;
	string orderBy_;

	int nRows_;
	int nCols_;
};


#endif
