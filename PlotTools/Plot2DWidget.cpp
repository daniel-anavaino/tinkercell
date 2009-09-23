/****************************************************************************
 Copyright (c) 2008 Deepak Chandran
 Contact: Deepak Chandran (dchandran1@gmail.com)
 see COPYRIGHT.TXT

 This tool displays a plot based on the DataTable contained.
 
****************************************************************************/

#include <math.h>
#include <QSettings>
#include <QColorDialog>
#include <QPushButton>
#include <QGroupBox>
#include <QPrinter>
#include <QListWidget>
#include <QTableWidget>
#include "qwt_scale_engine.h"
#include "GraphicsScene.h"
#include "MainWindow.h"
#include "ConsoleWindow.h"
#include "PlotTool.h"
#include "PlotTextWidget.h"
#include "Plot2DWidget.h"

namespace Tinkercell
{
	/**********************
	Data Column
	**********************/
	DataColumn::DataColumn(DataTable<qreal>* dataPtr, int xindex, int yindex, int delta)
	{
		dataTable = dataPtr;
		column = yindex;
		xaxis = xindex;
		dt = delta;
		if (dt < 1 || dt > dataTable->rows()/2) dt = 1;
	}
	
	QwtData * DataColumn::copy() const
	{
		return new DataColumn(dataTable,xaxis, column);
	}
	
	size_t DataColumn::size() const
	{
		if (!dataTable) return 0;
		return (int)(dataTable->rows()/dt);
	}
	
	double DataColumn::x(size_t index) const
	{
		if (!dataTable) return 0;
		if (xaxis < 0) return (int)index;
		return dataTable->at((int)index,xaxis);
	}
	
	double DataColumn::y(size_t index) const
	{
		if (!dataTable) return 0;
		return dataTable->at((int)index*dt,column);
	}

	/****************************
		Data Plot
	****************************/

	QList<QPen> DataPlot::penList = QList<QPen>();

	DataPlot::DataPlot(QWidget * parent) : QwtPlot(parent)
	{
		xcolumn = 0;
		delta = 1;
		zoomer = new QwtPlotZoomer(xBottom,yLeft,QwtPicker::DragSelection,QwtPicker::AlwaysOff,canvas());
		zoomer->setRubberBandPen(QPen(Qt::black));
		setCanvasBackground(Qt::white);
		plotLayout()->setAlignCanvasToScales(true);
		plotLayout()->setCanvasMargin(0);
		//setAxisAutoScale(xBottom);
		//setAxisAutoScale(yLeft);
	}

	QSize DataPlot::minimumSizeHint() const
	{
		return QSize(100,50);
	}

	QSize DataPlot::sizeHint() const
	{
		return QSize(160,80);
	}
	
	void DataPlot::itemChecked(QwtPlotItem * plotItem,bool on)
	{
		if (plotItem)
		{
			on = !on;
			plotItem->setVisible(on);
			const QwtPlotItemList& list = itemList();
			for (int i=0; i < dataTable.cols() && i < list.size(); ++i)
				if (list.at(i) == plotItem)
				{
					if (on && hideList.contains(dataTable.colName(i)))
					{
						hideList.removeAll(dataTable.colName(i));
					}
					else
					if (!on && !hideList.contains(dataTable.colName(i)))
					{
						visibleDataTable.removeCol(dataTable.colName(i));
						hideList += (dataTable.colName(i));
					}
				}
			this->replot();
		}
	}
	
	void DataPlot::setXAxis(int x)
	{
		if (x >= 0 && x < dataTable.cols())
		{
			int dt = 1;
			if (dataTable.rows() > 100)
			{
				dt = (int)(dataTable.rows() / 100) + 1;
			}
			plot(dataTable,x,title().text(),dt);
		}
	}
	
	void DataPlot::plot(const DataTable<qreal>& dat, int x, const QString& title, int dt)
	{
		delta = dt;
		xcolumn = x;
		if (!this->isVisible())
		{
			if (this->parentWidget() && !this->parentWidget()->isVisible())
				this->parentWidget()->show();
			else
				this->show();
		}
		setAutoReplot(false);
		this->dataTable = dat;
		this->visibleDataTable = dat;
		
		QRegExp regex(tr("\\_(?!_)"));
		for (int i=0; i < dataTable.rows(); ++i)
		{
			this->dataTable.rowName(i).replace(regex,tr("."));
		}

		this->clear();
		insertLegend(new QwtLegend(this), QwtPlot::RightLegend,0.2);
		legend()->setItemMode(QwtLegend::CheckableItem);
		
		connect(this,SIGNAL(legendChecked(QwtPlotItem*,bool)),this,SLOT(itemChecked(QwtPlotItem*,bool)));
		
		QList<QwtPlotCurve*> curves;
		for (int i=0, c = 0, t = 0; i < dataTable.cols(); ++i)
		{
			if (i != x && dataTable.colName(i).toLower() != tr("time"))
			{
				QwtPlotCurve * curve = new QwtPlotCurve(dataTable.colName(i));
				curve->setRenderHint(QwtPlotItem::RenderAntialiased);
				
				if (c >= penList.size())
				{
					c = 0;
				}
				
				curve->setPen(penList[c]);
				curve->setData( DataColumn(&dataTable,x,i,dt) );
				curve->attach(this);
				curve->updateLegend(legend());
				
				++c;
			}
		}
		if (dataTable.cols() > x)
			setAxisTitle(xBottom, dataTable.colName(x));
		else
			if (x < 0)
				setAxisTitle(xBottom, "Index");
			else
				setAxisTitle(xBottom, "Time");
		
		QString ylabel = axisTitle(QwtPlot::yLeft).text();
		if (ylabel.isEmpty() || ylabel.isNull())
			ylabel = tr("Values");
		setAxisTitle(yLeft, ylabel);
		setTitle(title);

		setAxisAutoScale(xBottom);
		setAxisAutoScale(yLeft);
		setAutoReplot(true);
		replot();
		if (zoomer)
		{
			zoomer->setZoomBase();
		}
		
		replotUsingHideList();
	}
	
	void DataPlot::replotUsingHideList()
	{
		const QwtPlotItemList& list = itemList();
		QwtLegend * leg = legend();
		for (int i=0; i < dataTable.cols() && i < list.size(); ++i)
			if (hideList.contains(dataTable.colName(i)))
			{
				list[i]->setVisible(false);
				visibleDataTable.removeCol(dataTable.colName(i));
				QWidget * w = leg->find(list[i]);
				if ( w && w->inherits( "QwtLegendItem" ) )
					((QwtLegendItem *)w)->setChecked( true );
			}
		replot();
	}
	
	void DataPlot::setLogX(bool b)
	{
		if (b)
		{
			setAxisMaxMajor(QwtPlot::xBottom, 6);
			setAxisMaxMinor(QwtPlot::xBottom, 10);
			QwtLog10ScaleEngine * engine = new QwtLog10ScaleEngine;
			double d1,d2,d3;
			engine->autoScale(1,d1,d2,d3);
			setAxisScaleEngine(QwtPlot::xBottom, engine);
		}
		else
		{
			setAxisScaleEngine(QwtPlot::xBottom, new QwtLinearScaleEngine);
		}
		replot();
	}
	
	void DataPlot::setLogY(bool b)
	{
		if (b)
		{
			setAxisMaxMajor(QwtPlot::yLeft, 6);
			setAxisMaxMinor(QwtPlot::yLeft, 10);
			QwtLog10ScaleEngine * engine = new QwtLog10ScaleEngine;
			double d1,d2,d3;
			engine->autoScale(1,d1,d2,d3);
			setAxisScaleEngine(QwtPlot::yLeft, engine);
		}
		else
		{
			setAxisScaleEngine(QwtPlot::yLeft, new QwtLinearScaleEngine);
		}
		replot();
	}
	
	/*********************************
		Plot Widget
	*********************************/
	
	
	Plot2DWidget::Plot2DWidget(PlotTool * parent) : PlotWidget(parent), buttonsGroup(this)
	{
		if (DataPlot::penList.isEmpty())
		{
			QCoreApplication::setOrganizationName(Tinkercell::ORGANIZATIONNAME);
			QCoreApplication::setOrganizationDomain(Tinkercell::PROJECTWEBSITE);
			QCoreApplication::setApplicationName(Tinkercell::ORGANIZATIONNAME);

			QSettings settings(Tinkercell::ORGANIZATIONNAME, Tinkercell::ORGANIZATIONNAME);
			
			settings.beginGroup("Plot2DWidget");
			
			QStringList colors, penWidths, penStyles;
			
			colors = settings.value(tr("colors"),QStringList()).toStringList();
			penWidths = settings.value(tr("widths"),QStringList()).toStringList();
			penStyles = settings.value(tr("styles"),QStringList()).toStringList();
		
			settings.endGroup();
			
			for (int i=0; i < colors.size() && i < penWidths.size() && i < penStyles.size(); ++i)
			{
				bool ok;
				double w = penWidths[i].toDouble(&ok);
				if (!ok) w = 2.0;
				
				int k = penStyles[i].toInt(&ok);
				if (!ok) k = 0;
				
				DataPlot::penList << QPen(QColor(colors[i]),w,Qt::PenStyle(k));
			}
		}
		
		dataPlot = new DataPlot();
		
		axisNames = new QComboBox();
		connect(axisNames,SIGNAL(currentIndexChanged(int)),dataPlot,SLOT(setXAxis(int)));
		
		dataPlot->setTitle(tr("Plot"));
		
		QVBoxLayout * layout = new QVBoxLayout;
		layout->addWidget(dataPlot);
		setPalette(QPalette(QColor(255,255,255,255)));
		setAutoFillBackground(true);
		setLayout(layout);
		
		QToolButton * setLabels = new QToolButton(this);
		setLabels->setIcon(QIcon(":/images/text.png"));
		setLabels->setText(tr("Labels"));
		setLabels->setToolTip(tr("Change title and axis labels"));
		QMenu * labelsMenu = new QMenu(tr("Set labels"),setLabels);
		labelsMenu->addAction(tr("Title"),this,SLOT(setTitle()));
		labelsMenu->addAction(tr("x label"),this,SLOT(setXLabel()));
		labelsMenu->addAction(tr("y label"),this,SLOT(setYLabel()));
		setLabels->setMenu(labelsMenu);
		setLabels->setPopupMode ( QToolButton::MenuButtonPopup );
		setLabels->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
		
		QToolButton * logScale = new QToolButton(this);
		logScale->setIcon(QIcon(":/images/log.png"));
		logScale->setText(tr("Log scale"));
		logScale->setToolTip(tr("Change to and from log scale"));
		QMenu * logMenu = new QMenu(tr("Set axis labels"),logScale);
		QAction * logx = logMenu->addAction(tr("x-axis"));
		QAction * logy = logMenu->addAction(tr("y-axis"));
		logx->setCheckable(true);
		logy->setCheckable(true);
		logScale->setMenu(logMenu);
		logScale->setPopupMode ( QToolButton::MenuButtonPopup );
		logScale->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
		
		connect(logx,SIGNAL(toggled(bool)),this,SLOT(logX(bool)));
		connect(logy,SIGNAL(toggled(bool)),this,SLOT(logY(bool)));
		
		QToolButton * changeColors = new QToolButton(this);
		changeColors->setIcon(QIcon(":/images/pencil.png"));
		changeColors->setText(tr("Colors"));
		changeColors->setToolTip(tr("Line type and color"));
		changeColors->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
		
		this->dialog = new GetPenInfoDialog(this);
		dialog->hide();
		
		QDialog * dialog2 = new QDialog(this);
		QHBoxLayout * dialogLayout = new QHBoxLayout;
		dialogLayout->addWidget(dialogWidget());
		dialog2->setLayout(dialogLayout);
		
		connect(changeColors,SIGNAL(pressed()),dialog2,SLOT(show()));
		
		QHBoxLayout * layout1 = new QHBoxLayout;
		layout1->addWidget(axisNames);
		
		QGroupBox * groupBox1 = new QGroupBox(tr(" x-axis "));
		groupBox1->setLayout(layout1);
		
		toolBar.addWidget(groupBox1);
		toolBar.addWidget(setLabels);
		toolBar.addWidget(logScale);
		toolBar.addWidget(changeColors);
		
		connect(&buttonsGroup,SIGNAL(buttonPressed(int)),this,SLOT(buttonPressed(int)));
		
		setMinimumHeight(200);
	}
	
	void Plot2DWidget::buttonPressed(int i)
	{
		if (dialog && DataPlot::penList.size() > i)
		{
			DataPlot::penList[i] = dialog->getPen(DataPlot::penList[i]);
			QAbstractButton * button = buttonsGroup.button(i);
			if (button)
				button->setStyleSheet(tr("background-color: ") + DataPlot::penList[i].color().name());
		}
	}
	
	void Plot2DWidget::plot(const DataTable<qreal>& matrix,const QString& title,int x)
	{
		if (!dataPlot) return;
		
		int dt = 1;
		if (matrix.rows() > 100)
		{
			dt = (int)(matrix.rows() / 100) + 1;
		}
		
		dataPlot->plot(matrix,x,title,dt);
		if (axisNames)
		{
			axisNames->clear();
			if (x >= 0)
				axisNames->addItems(matrix.getColNames());
		}
	}
	
	DataTable<qreal>* Plot2DWidget::data()
	{
		if (!dataPlot) return 0;
		
		return &(dataPlot->visibleDataTable);
	}
	
	void Plot2DWidget::exportData(const QString& type)
	{
		if (!dataPlot) return;
		
		if (type.toLower() == tr("image"))
		{
			QString fileName = 
				QFileDialog::getSaveFileName(this, tr("Print to File"),
                                          MainWindow::userHome(),
                                          tr("PDF Files (*.pdf)"));
			
			if (fileName.isEmpty()) return;
			
			//QPrinter printer(QPrinter::HighResolution);
			QPrinter printer(QPrinter::ScreenResolution);
			printer.setOutputFormat(QPrinter::PdfFormat);
			printer.setOrientation(QPrinter::Landscape);
			
			printer.setOutputFileName(fileName);
			dataPlot->print(printer);
		}
		else
		if (type.toLower() == tr("snapshot"))
		{
			QClipboard * clipboard = QApplication::clipboard();
			if (clipboard)
			{
				QImage image(600,400,QImage::Format_ARGB32);
				dataPlot->print(image);
				clipboard->setImage(image);
			}
		}
		else
		if (type.toLower() == tr("latex"))
		{
			plotTool->addWidget(new PlotTextWidget(dataPlot->dataTable,plotTool, latex()));
		}
		else
		{
			PlotWidget::exportData(type);
		}
	}
	
	void Plot2DWidget::logX(bool b)
	{
		if (dataPlot) dataPlot->setLogX(b);
	}
	
	void Plot2DWidget::logY(bool b)
	{
		if (dataPlot) dataPlot->setLogY(b);
	}
	
	void Plot2DWidget::logAxis(int i,bool b)
	{
		if (i==0) 
			logX(b);
		else 
			logY(b);
	}
	
	void Plot2DWidget::setTitle()
	{	
		QString s = QInputDialog::getText(this,tr("Plot Title"),tr("Plot title :"));
		
		setTitle(s);
	}
	
	void Plot2DWidget::setTitle(const QString& s)
	{
		if (!dataPlot) return;
		
		if (s.isNull() || s.isEmpty()) return;
		
		dataPlot->setTitle(s);
	}
	
	void Plot2DWidget::setXLabel()
	{	
		QString s = QInputDialog::getText(this,tr("Plot Label"),tr("x-axis label :"));
		
		setXLabel(s);	
	}
	
	void Plot2DWidget::setXLabel(const QString& s)
	{
		if (!dataPlot) return;
		
		if (s.isNull() || s.isEmpty()) return;
		
		dataPlot->setAxisTitle(QwtPlot::xBottom, s);		
	}
	
	void Plot2DWidget::setYLabel()
	{
		QString s = QInputDialog::getText(this,tr("Plot Label"),tr("y-axis label :"));
		
		setYLabel(s);
	}
	
	void Plot2DWidget::setYLabel(const QString& s)
	{
		if (!dataPlot) return;
		
		if (s.isNull() || s.isEmpty()) return;
		
		dataPlot->setAxisTitle(QwtPlot::yLeft, s);
	}
	
	void Plot2DWidget::mouseMoveEvent ( QMouseEvent * event )
	{
		if (!event || !plotTool || !dataPlot) return;
		
		plotTool->setStatusBarMessage(
			tr("  x: ") 
			+ QString::number(dataPlot->invTransform(QwtPlot::xBottom,event->pos().x()))
			+ tr("  y: ") 
			+ QString::number(dataPlot->invTransform(QwtPlot::yLeft,event->pos().y()))
		);
	}
	
	QWidget * Plot2DWidget::dialogWidget()
	{
		if (DataPlot::penList.isEmpty())
		{
			DataPlot::penList 	<< QPen(QColor(tr("#232CE6")),2,Qt::SolidLine)
								<< QPen(QColor(tr("#CA420D")),2,Qt::SolidLine)
								<< QPen(QColor(tr("#11A306")),2,Qt::SolidLine)
								<< QPen(QColor(tr("#BF0CB0")),2,Qt::SolidLine)
								<< QPen(QColor(tr("#D9C11F")),2,Qt::SolidLine)
								<< QPen(QColor(tr("#0CBDBF")),2,Qt::SolidLine)
								<< QPen(QColor(tr("#232CE6")),2,Qt::DotLine)
								<< QPen(QColor(tr("#CA420D")),2,Qt::DotLine)
								<< QPen(QColor(tr("#11A306")),2,Qt::DotLine)
								<< QPen(QColor(tr("#BF0CB0")),2,Qt::DotLine)
								<< QPen(QColor(tr("#D9C11F")),2,Qt::DotLine)
								<< QPen(QColor(tr("#0CBDBF")),2,Qt::DotLine);
		}
		
		QTableWidget * tableWidget = new QTableWidget(DataPlot::penList.size(),1);
		tableWidget->horizontalHeader()->hide();
		tableWidget->verticalHeader()->hide();
		
		for (int i=0; i < DataPlot::penList.size(); ++i)
		{
			QPushButton * button = new QPushButton;
			button->setStyleSheet(tr("background-color: ") + DataPlot::penList[i].color().name());
			tableWidget->setCellWidget(i,0,button);
			buttonsGroup.addButton(button,i);
		}
		
		return tableWidget;
	}
	
	QString Plot2DWidget::latex()
	{
		if (!dataPlot) return QString();
		
		QString output;
		
		DataTable<qreal> & table = *(data());
		
		double xmin = table.at(0,dataPlot->xcolumn),
			   xmax = table.at(table.rows()-1,dataPlot->xcolumn),
			   ymin = table.at(0,1),
			   ymax = table.at(0,1);
			
		QStringList colnames(table.getColNames()),
					rownames(table.getRowNames());

		int c = 0;

		for (int i=0; i < colnames.size(); ++i)
		{
			for (int j=0; j < table.rows(); ++j)
				for (int k=0; k < table.cols(); ++k)
					if (k != dataPlot->xcolumn)
					{
						if (ymin > table.at(j,k))
							ymin = table.at(j,k);

						if (ymax < table.at(j,k))
							ymax = table.at(j,k);
					}
				
			if (c >= DataPlot::penList.size())
			{
				c = 0;
			}				
				
			QPen pen = DataPlot::penList[c];
		
			output += tr("\\documentclass{article}\n\n\\usepackage{tikz}\n\n\\usepackage{pgfplots}\n\n\n\\begin{document}\n\n");
			output += tr("\\begin{ticzpicture}\n\\begin{axis}[\ngrid=major,\nxlabel=")
						+ colnames.at(dataPlot->xcolumn) 
						+ tr(",\nylabel=Values")
						+ tr(",\nxmin=") + QString::number(xmin)
						+ tr(",\nxmax=") + QString::number(xmax)
						+ tr(",\nymin=") + QString::number(ymin)
						+ tr(",\nymax=") + QString::number(ymax)
						+ tr(",\nwidth=8cm,\nheight=6cm,]\n\\addplot[smooth,color=")
						+ pen.color().name()
						+ tr(",line width=1.5pt] coordinates {\n");
		
			for (int j=0; j < table.rows(); ++j)
			{
				output += 	tr("(") 
							+ QString::number(table.at(j,dataPlot->xcolumn))
							+ tr(", ")
							+ QString::number(table.at(j,i))
							+ tr(")\n");
			}
			output += tr("};\n\\addlegendentry{") + colnames.at(i) + tr("}\n");
		}
		
		output += tr("\\end{axis}\n\\end{tikzpicture}\n");
		
		return output;
	}
	
	GetPenInfoDialog::GetPenInfoDialog(QWidget * parent) : QDialog(parent)
	{
		connect(&colorDialog,SIGNAL(currentColorChanged(const QColor&)),this,SLOT(currentColorChanged(const QColor&)));
		
		QHBoxLayout * layout1 = new QHBoxLayout;
		layout1->addWidget(&colorDialog);
		QGroupBox * colorGrp = new QGroupBox(tr(" pen color "));
		colorGrp->setLayout(layout1);
		
		QHBoxLayout * layout2 = new QHBoxLayout;
		layout2->addWidget(&comboBox);
		comboBox.addItems(QStringList() << "Solid line" << "Dotted line");
		layout2->addWidget(&spinBox);
		spinBox.setRange(0,10);
		spinBox.setValue(2.0);
		QGroupBox * spinBoxGrp = new QGroupBox(tr(" line type and width "));
		colorGrp->setLayout(layout2);
		
		setSizeGripEnabled(true);
		
		QHBoxLayout * layout3 = new QHBoxLayout;		
		QPushButton * okButton = new QPushButton;
		QPushButton * cancelButton = new QPushButton;
		okButton->setText(tr(" Set Pen "));
		cancelButton->setText(tr(" Cancel "));
		layout3->addWidget(okButton);
		layout3->addWidget(cancelButton);
		
		connect(okButton,SIGNAL(pressed()),this,SLOT(accept()));
		connect(cancelButton,SIGNAL(pressed()),this,SLOT(reject()));
		
		QVBoxLayout * layout = new QVBoxLayout;
		layout->addLayout(layout1);
		layout->addLayout(layout2);
		layout->addLayout(layout3);
		
		setLayout(layout);
	}
	
	void GetPenInfoDialog::currentColorChanged ( const QColor & color )
	{
		this->color = color;
	}
	
	QPen GetPenInfoDialog::getPen(const QPen& pen)
	{
		colorDialog.setCurrentColor(pen.color());
		color = pen.color();
		spinBox.setValue(pen.widthF());
		if (pen.style() == Qt::SolidLine)
			comboBox.setCurrentIndex(0);
		else
			comboBox.setCurrentIndex(1);
		
		open();
		if (result() == 0) return pen;
		
		if (comboBox.currentIndex() == 0)
			return QPen(color,spinBox.value(),Qt::SolidLine);
		else
			return QPen(color,spinBox.value(),Qt::DotLine);
			
		QCoreApplication::setOrganizationName(Tinkercell::ORGANIZATIONNAME);
		QCoreApplication::setOrganizationDomain(Tinkercell::PROJECTWEBSITE);
		QCoreApplication::setApplicationName(Tinkercell::ORGANIZATIONNAME);

		QSettings settings(Tinkercell::ORGANIZATIONNAME, Tinkercell::ORGANIZATIONNAME);
		
		settings.beginGroup("Plot2DWidget");
		
		QStringList colors, penWidth, penStyles;
		
		QList<QPen>& penList = DataPlot::penList;
		
		for (int i=0; i < penList.size(); ++i)
		{
			colors << penList[i].color().name();
			penStyles << QString::number((int)(penList[i].style()));
			penWidth << QString::number(penList[i].widthF());
		}
		
		settings.setValue(tr("colors"),colors);
		settings.setValue(tr("widths"),penWidth);
		settings.setValue(tr("styles"),penStyles);
		
		settings.endGroup();
	}
}
