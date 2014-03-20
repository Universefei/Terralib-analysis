#ifndef TEQTDATASOURCE_H
#define TEQTDATASOURCE_H

#include <qobject.h>
#include <qvariant.h>

#include "../../kernel/TeDefines.h"

class TL_DLL TeQtDataSource : public QObject
{
	Q_OBJECT
public:
	TeQtDataSource() {}
	virtual ~TeQtDataSource() {}

	virtual QVariant cell(int row, int col) = 0;
	virtual void setCell(int row, int col, const QVariant &text) = 0;

	virtual int numRows() const = 0;
	virtual int numCols() const = 0;

signals:
	void dataChanged();
};

#endif
