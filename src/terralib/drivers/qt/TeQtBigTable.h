#ifndef TABLE_H
#define TABLE_H

#include <qtable.h>
#include <qlineedit.h>
#include <string>

#include "../../kernel/TeDefines.h"

class TeQtDataSource;

using namespace std;

class TL_DLL TeQtBigTable : public QTable
{
	Q_OBJECT

public:
	TeQtBigTable(QWidget *parent = 0, const char *name = 0);
	TeQtBigTable(TeQtDataSource *dSource, QWidget *parent = 0, const char *name = 0);

	void setDataSource(TeQtDataSource *dSource);
	TeQtDataSource* getDataSource() { return dataSource_; }

	QString text(int row, int col) const;
	QWidget *createEditor(int row, int col, bool initFromCell) const;
	void setCellContentFromEditor(int row, int col);
	QWidget *cellWidget(int row, int col) const;
	void endEdit(int row, int col, bool accept, bool replace);

	void resizeData(int) {}
	QTableItem *item(int, int) { return 0; }
	void setItem(int, int, QTableItem *) {}
	void clearCell(int, int) {}
	void insertWidget(int, int, QWidget *) {}
	void clearCellWidget(int, int) {}

	void adjustColumn(int col);

public slots:
	void updateContents();

protected:
	TeQtDataSource *dataSource_;
	mutable QLineEdit *editor_;

	virtual void paintCell(QPainter *painter, int row, int col,
				   const QRect &cr, bool selected, const QColorGroup &cg);

};


#endif
