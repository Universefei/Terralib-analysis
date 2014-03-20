#ifndef TEQTLEGENDSOURCE_H
#define TEQTLEGENDSOURCE_H

#include "TeQtDataSource.h"
#include <TeAppTheme.h>
#include "../../utils/TeColorUtils.h"
#include <string>

#include "../../kernel/TeDefines.h"

class TeDatabase;

using namespace std;


class TL_DLL TeQtLegendSource : public TeQtDataSource
{
	Q_OBJECT

	public:
	TeQtLegendSource();
	~TeQtLegendSource() {}

	QVariant cell(int row, int col);
	void setCell(int row, int col, const QVariant &v);

	bool generateLegends(const TeGrouping& groupingParams, double minValue = 0., double maxValue = 0.);
	void putColorOnLegend(string& groupingColors, bool invertColor);
	void putColorOnLegend(vector<ColorBar>& colorVec, vector<ColorBar>& colorBVec);
	void getColorNameVector(string& groupingColors, vector<string>& colorNameVec);

	QPixmap createPixmap(int row);
	void drawPolygonRep(int w, int h, int offset, int row, QPixmap *pixmap);
	void drawLineRep(int offset, int row, QPixmap *pixmap);
	void drawPointRep(int pw, int ph, int offset, int row, QPixmap *pixmap);

	void copyAppThemeContents(const TeAppTheme& appTheme);
	TeAppTheme& getAppTheme() { return appTheme_; }
	vector<map<string, string> >& mapObjValVec() { return mapObjValVec_; }
	void setGroupingColors(const QString& gc)
		{ appTheme_.groupColor(gc.latin1()); }

	void setLegends(const vector<TeLegendEntry>& legVec);
	void setGroupingParams(const TeGrouping& gParams);

	int numRows() const { return nRows_; }
	int numCols() const { return nCols_; }

private:
	TeDatabase* db_;
	TeAppTheme	appTheme_;

    vector<double> dValuesVec_;
    vector<map<string, string> >mapObjValVec_;
    map<TePolyBasicType, Qt::BrushStyle> brushStyleMap_;
    map<TeLnBasicType, Qt::PenStyle> penStyleMap_;

	QString errorMessage_;
	QString groupingColors_;

	int nRows_;
	int nCols_;
};


#endif
