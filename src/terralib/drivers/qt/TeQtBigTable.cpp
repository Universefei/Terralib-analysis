#include <TeQtBigTable.h>
#include <TeQtDataSource.h>
#include <qpainter.h>
#include <qapplication.h>



TeQtBigTable::TeQtBigTable(QWidget *parent, const char *name)
        : QTable(parent, name)
{
}

TeQtBigTable::TeQtBigTable(TeQtDataSource *dSource, QWidget *parent, const char *name)
        : QTable(dSource->numRows(), dSource->numCols(), parent, name),
          dataSource_(dSource), editor_(0)
{
	connect(dataSource_, SIGNAL(dataChanged()), this, SLOT(updateContents()));
}

void TeQtBigTable::setDataSource(TeQtDataSource *dSource)
{
	dataSource_ = dSource;

	connect(dataSource_, SIGNAL(dataChanged()), this, SLOT(updateContents()));
}

void TeQtBigTable::updateContents()
{
	if (numRows() == 0 || numCols() == 0)
	{
		setNumRows(dataSource_->numRows());
		setNumCols(dataSource_->numCols());
	}
	QTable::updateContents();
}

QString TeQtBigTable::text(int row, int col) const
{
	QVariant v(dataSource_->cell(row, col));
	return v.toString();
}
    

void TeQtBigTable::paintCell(QPainter *painter, int row, int col,
                          const QRect &cr, bool selected, const QColorGroup &cg)
{
	QRect rect(0, 0, cr.width(), cr.height());
	if (selected)
	{
		painter->fillRect(rect, cg.highlight());
		painter->setPen(cg.highlightedText());
	}
	else
	{
		painter->fillRect(rect, cg.base());
		painter->setPen(cg.text());
	}

	QTable::paintCell(painter, row, col, cr, selected, cg);

	QVariant v(dataSource_->cell(row, col));
	if (v.type() == QVariant::Pixmap)
	{
		QPixmap p = v.toPixmap();
		painter->drawPixmap(0, 0, p);
	}
	else if (v.type() == QVariant::String || v.type() == QVariant::CString)
	{
		QString qs = v.toString();
		bool ok;
		qs.toDouble(&ok);
		if (ok)
			painter->drawText(0, 0, cr.width()-10, cr.height(), Qt::AlignRight | Qt::SingleLine, v.toString());
		else
			painter->drawText(0, 0, cr.width()-10, cr.height(), Qt::AlignLeft | Qt::SingleLine, v.toString());

	}
}

QWidget* TeQtBigTable::createEditor(int row, int col, bool initFromCell) const
{
	QVariant v(dataSource_->cell(row, col));
	if (v.type() == QVariant::Pixmap)
		return 0;

	editor_ = new QLineEdit(viewport());
	if (initFromCell)
		editor_->setText(text(row, col));
	return editor_;
}


QWidget* TeQtBigTable::cellWidget(int row, int col) const
{
	if (row < 0 || col < 0)
		return 0;
	
	if (row == currEditRow() && col == currEditCol())
			return editor_;
	else
		return 0;
}


void TeQtBigTable::endEdit(int row, int col, bool /* accept */, bool replace)
{
	QTable::endEdit(row, col, false, replace);
	if (editor_)
	{
		// Update the data that was edited in the table in the data source
		dataSource_->setCell(row, col, editor_->text());
	}
	delete editor_;
	editor_ = 0;
}


void TeQtBigTable::setCellContentFromEditor(int row, int col)
{
	if (editor_)
		dataSource_->setCell(row, col, editor_->text());
}

void TeQtBigTable::adjustColumn(int col)
{
	int i, w, oldw;
	QString txt;
	QFontMetrics fm = fontMetrics();

	txt = horizontalHeader()->label(col);
	oldw = fm.width(txt) + 10;
	oldw = QMAX(oldw, 30);

	QVariant v(dataSource_->cell(0, col));
	if (v.type() == QVariant::Pixmap)
	{
		QPixmap p = v.toPixmap();
		w = p.width() + 10;
		w = QMAX(w, oldw);
		oldw = w;
	}
	else
	{
		for (i = 0; i < numRows(); ++i )
		{
			QString txt = text(i,col);
			w = fm.width(txt) + 15;
			w = QMAX(w, oldw);
			oldw = w;
		}
	}
	w = QMAX( w, QApplication::globalStrut().width() );
	setColumnWidth(col, w);
}




