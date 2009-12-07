/****************************************************************************

Copyright (c) 2008 Deepak Chandran
Contact: Deepak Chandran (dchandran1@gmail.com)
See COPYRIGHT.TXT

This is source file for Tinkercell's main window
The MainWindow contains a set of GraphicScenes, which is the class
that performs all the drawing. Each GraphicsScene emits various signals. Those
signals are then emitted by the MainWindow; in this way, a plugin does not need
to listen to each of the GraphicsScene signals but only the MainWindow's signals.

The MainWindow also has its own signals, such as a toolLoaded, modelSaved, etc.

The MainWindow keeps a list of all plugins, and it is also responsible for loading plugins.


****************************************************************************/

#include <QLibrary>
#include <QSettings>
#include <QInputDialog>
#include <QActionGroup>
#include <QCoreApplication>
#include <QDesktopServices>
#include <QtDebug>
#include <QSvgGenerator>
#include <QColorDialog>
#include <QImage>
#include "TextEditor.h"
#include "TextItem.h"
#include "NetworkWindow.h"
#include "GraphicsScene.h"
#include "MainWindow.h"
#include "NodeGraphicsItem.h"
#include "ConnectionGraphicsItem.h"
#include "TextGraphicsItem.h"
#include "ItemHandle.h"
#include "Tool.h"
#include "MainWindow.h"
#include "CThread.h"
#include "ConsoleWindow.h"
#include "AbstractInputWindow.h"
#include "TextParser.h"

namespace Tinkercell
{
	typedef void (*TinkercellPluginEntryFunction)(MainWindow*);
	typedef void (*TinkercellCEntryFunction)();

	MainWindow::TOOL_WINDOW_OPTION MainWindow::defaultToolWindowOption = MainWindow::ToolBoxWidget;
	MainWindow::TOOL_WINDOW_OPTION MainWindow::defaultHistoryWindowOption = MainWindow::ToolBoxWidget;
	MainWindow::TOOL_WINDOW_OPTION MainWindow::defaultConsoleWindowOption = MainWindow::DockWidget;
	QString MainWindow::previousFileName;

	QString MainWindow::userHomePath;
	QString MainWindow::userHome()
	{
		if (!userHomePath.isEmpty() && QDir(userHomePath).exists())		
			return userHomePath;
		
		QDir dir = QDir::homePath();
		QString tcdir = PROJECTNAME;

		if (!dir.exists(tcdir))		
			dir.mkdir(tcdir);

		dir.cd(tcdir);
		userHomePath = dir.absolutePath();

		return userHomePath;
	}

	void MainWindow::setUserHome()
	{
		QString home = QFileDialog::getExistingDirectory(this,tr("Select new user home directory"),userHome());
		if (home.isEmpty() || home.isNull()) return;

		QCoreApplication::setOrganizationName(ORGANIZATIONNAME);
		QCoreApplication::setOrganizationDomain(PROJECTWEBSITE);
		QCoreApplication::setApplicationName(ORGANIZATIONNAME);

		QSettings settings(ORGANIZATIONNAME, ORGANIZATIONNAME);

		settings.beginGroup("MainWindow");
		settings.setValue("home", home);
		userHomePath = home;
		settings.endGroup();
	}

	typedef void (*progress_api_initialize)(void (*tc_showProgress)(const char *, int));

	void MainWindow::loadDynamicLibrary(const QString& dllFile)
	{
		QString home = userHome(),
			current = QDir::currentPath(),
			appDir = QCoreApplication::applicationDirPath();

		QString name[] = {	
			home + tr("/") + dllFile,
			current + tr("/") + dllFile,
			appDir + tr("/") + dllFile,
			dllFile
		};

		QLibrary * lib = new QLibrary(this);

		bool loaded = false;
		for (int i=0; i < 4; ++i) //try different possibilities
		{
			lib->setFileName(name[i]);
			loaded = lib->load();
			if (loaded)
				break;
		}

		if (loaded)
		{
			dynamicallyLoadedLibraries.insert(lib->fileName(),lib);
			statusBar()->showMessage(lib->fileName() + tr(" loading ..."));
			TinkercellPluginEntryFunction f1 = (TinkercellPluginEntryFunction)lib->resolve(CPP_ENTRY_FUNCTION.toAscii().data());
			if (f1)
			{
				try
				{
					f1(this);
					dynamicallyLoadedLibraries.insert(lib->fileName(),lib);
					statusBar()->showMessage(lib->fileName() + tr(" successfully loaded"));
				}
				catch(...)
				{
					lib->unload();
					delete lib;
				}
			}
			else
			{
				TinkercellCEntryFunction f2 = (TinkercellCEntryFunction)lib->resolve(C_ENTRY_FUNCTION.toAscii().data());
				if (f2)
				{
					try
					{
						emit setupFunctionPointersSlot(0,lib);
						f2();
						dynamicallyLoadedLibraries.insert(lib->fileName(),lib);
						statusBar()->showMessage(lib->fileName() + tr(" successfully loaded"));
					}
					catch(...)
					{
						lib->unload();
						delete lib;
					}
				}
				else
				{
					lib->unload();
					delete lib;
				}
			}
		}
		else
		{
			statusBar()->showMessage(lib->fileName() + tr(" could not be opened"));
			delete lib;
		}
	}

	void MainWindow::setupNewThread(QSemaphore* s,QLibrary * f)
	{
		if (!f)
		{
			if (s)
				s->release();
		}
		emit funtionPointersToMainThread(s,f );
	}

	MainWindow::MainWindow(bool enableScene, bool enableText, bool enableConsoleWindow, bool showHistory, bool allowViews)
	{
		allowViewModeToChange = allowViews;

		setMouseTracking(true);
		RegisterDataTypes();
		previousFileName = QDir::currentPath();

		readSettings();

        consoleWindow = 0;
		currentNetworkWindow = 0;
		toolBox = 0;
		setAutoFillBackground(true);
		setAcceptDrops(true);

		initializeMenus(enableScene,enableText);
		//setIconSize(QSize(25,25));

		tabWidget = new QTabWidget;
		tabWidget->setStyleSheet(tr("QTabBar::tab { min-width: 36ex; }"));
		connect(tabWidget,SIGNAL(currentChanged(int)),this,SLOT(tabIndexChanged(int)));
		
		QToolButton * upButton = new QToolButton;
		upButton->setIcon(QIcon(tr(":/images/rightarrow.png")));
		tabWidget->setCornerWidget(upButton);
		connect(upButton,SIGNAL(pressed()),this,SLOT(popOut()));
		upButton->setToolTip(tr("Pop-out"));
		
		setCentralWidget(tabWidget);

		setWindowTitle(tr("Tinkercell"));
		setStyleSheet("QMainWindow::separator { width: 0px; height: 0px; }");

		connect(this,SIGNAL(funtionPointersToMainThread(QSemaphore*,QLibrary*)),this,SLOT(setupFunctionPointersSlot(QSemaphore*,QLibrary*)));

		connect(this,SIGNAL(itemsInserted(GraphicsScene*,QList<QGraphicsItem*>,QList<ItemHandle*>)),
			this,SLOT(itemsInsertedSlot(GraphicsScene*,QList<QGraphicsItem*>,QList<ItemHandle*>)));

		connect(this,SIGNAL(itemsRemoved(GraphicsScene*,QList<QGraphicsItem*>,QList<ItemHandle*>)),
			this,SLOT(itemsRemovedSlot(GraphicsScene*,QList<QGraphicsItem*>,QList<ItemHandle*>)));

		connect(this,SIGNAL(itemsInserted(TextEditor*,QList<TextItem*>,QList<ItemHandle*>)),
			this,SLOT(itemsInsertedSlot(TextEditor*,QList<TextItem*>,QList<ItemHandle*>)));

		connect(this,SIGNAL(itemsRemoved(TextEditor*,QList<TextItem*>,QList<ItemHandle*>)),
			this,SLOT(itemsRemovedSlot(TextEditor*,QList<TextItem*>,QList<ItemHandle*>)));

		if (showHistory)
		{
			historyWindow.setWindowTitle(tr("History"));
			historyWindow.setWindowIcon(QIcon(tr(":/images/undo.png")));
			addToolWindow(&historyWindow,MainWindow::defaultHistoryWindowOption,Qt::RightDockWidgetArea);
		}
		
		if (enableConsoleWindow)
		{
			consoleWindow = new ConsoleWindow(this);
			if (settingsMenu)
			{
				QMenu * consoleColorMenu = settingsMenu->addMenu(tr("Console window colors"));

				consoleColorMenu->addAction(tr("Background color"),this,SLOT(changeConsoleBgColor()));
				consoleColorMenu->addAction(tr("Text color"),this,SLOT(changeConsoleTextColor()));
				consoleColorMenu->addAction(tr("Output color"),this,SLOT(changeConsoleMsgColor()));
				consoleColorMenu->addAction(tr("Error message color"),this,SLOT(changeConsoleErrorMsgColor()));
			}
		}

		connectTCFunctions();

		parsersMenu = 0;
	}

	ConsoleWindow * MainWindow::console()
	{
	    return consoleWindow;
	}

	void MainWindow::saveSettings()
	{
		QCoreApplication::setOrganizationName(ORGANIZATIONNAME);
		QCoreApplication::setOrganizationDomain(PROJECTWEBSITE);
		QCoreApplication::setApplicationName(ORGANIZATIONNAME);

		QSettings settings(ORGANIZATIONNAME, ORGANIZATIONNAME);

		settings.beginGroup("MainWindow");
		settings.setValue("size", size());
		settings.setValue("pos", pos());
		settings.setValue("maximized",(isMaximized()));
		settings.setValue("previousFileName", previousFileName);
		settings.setValue("defaultToolWindowOption", (int)(defaultToolWindowOption));
		settings.setValue("defaultHistoryWindowOption", (int)(defaultHistoryWindowOption));
		settings.setValue("defaultConsoleWindowOption", (int)(defaultConsoleWindowOption));

		settings.endGroup();
	}

	void MainWindow::readSettings()
	{
		QCoreApplication::setOrganizationName(ORGANIZATIONNAME);
		QCoreApplication::setOrganizationDomain(PROJECTWEBSITE);
		QCoreApplication::setApplicationName(ORGANIZATIONNAME);

		QSettings settings(ORGANIZATIONNAME, ORGANIZATIONNAME);

		settings.beginGroup("MainWindow");

		resize(settings.value("size", QSize(1000, 500)).toSize());
		move(settings.value("pos", QPoint(100, 100)).toPoint());
		if (settings.value("maximized",false).toBool())
			showMaximized();
		previousFileName = settings.value("previousFileName", tr("")).toString();
		defaultToolWindowOption = (TOOL_WINDOW_OPTION)(settings.value("defaultToolWindowOption", (int)defaultToolWindowOption).toInt());
		defaultHistoryWindowOption = (TOOL_WINDOW_OPTION)(settings.value("defaultHistoryWindowOption", (int)defaultHistoryWindowOption).toInt());
		defaultConsoleWindowOption = (TOOL_WINDOW_OPTION)(settings.value("defaultConsoleWindowOption", (int)defaultConsoleWindowOption).toInt());

		settings.endGroup();
	}

	/*! \brief destructor*/
	MainWindow::~MainWindow()
	{
		GraphicsScene::clearStaticItems();
		saveSettings();
	}
	
	void MainWindow::tabIndexChanged(int i)
	{
		QWidget * w = tabWidget->currentWidget();
		if (w)
			setCurrentWindow(static_cast<NetworkWindow*>(w));
	}

	void MainWindow::setCurrentWindow(NetworkWindow * window)
	{
		if (window)
		{
			if (tabWidget)
			{
				int i = tabWidget->indexOf(window);
				if (i > -1 && i < tabWidget->count() && i != tabWidget->currentIndex())
					tabWidget->setCurrentIndex(i);
			}
			if (!window->hasFocus())
				window->setFocus();
			
			historyWindow.setStack(&(window->history));
			
			if (window != currentNetworkWindow)
				emit windowChanged(currentNetworkWindow,window);
		}
		currentNetworkWindow = window;
	}

	GraphicsScene * MainWindow::newGraphicsWindow()
	{
		GraphicsScene * scene = new GraphicsScene;
		NetworkWindow * subWindow = new NetworkWindow(this, scene);
		
		if (!allNetworkWindows.contains(subWindow))
			allNetworkWindows << subWindow;
		subWindow->move(pos());
		subWindow->resize(width()/2,height()/2);
		
		connect (subWindow,SIGNAL(closing(NetworkWindow *, bool*)),this,SIGNAL(windowClosing(NetworkWindow *, bool*)));
		
		emit windowOpened(subWindow);
		popIn(subWindow);
		
		return scene;
	}

	TextEditor * MainWindow::newTextWindow()
	{
		TextEditor * textedit = new TextEditor;
		NetworkWindow * subWindow = new NetworkWindow(this, textedit);

		subWindow->move(pos());
		subWindow->resize(width()/2,height()/2);
		
		if (!allNetworkWindows.contains(subWindow))
			allNetworkWindows << subWindow;
		connect (subWindow,SIGNAL(closing(NetworkWindow *, bool*)),this,SIGNAL(windowClosing(NetworkWindow *, bool*)));
		
		emit windowOpened(subWindow);
		popIn(subWindow);
		
		return textedit;
	}

	void MainWindow::allowMultipleViewModes(bool b)
	{
		allowViewModeToChange = b;
	}

	void MainWindow::closeWindow()
	{
		if (currentWindow())
		{
			currentWindow()->close();
		}
	}

	void MainWindow::saveWindow()
	{
		NetworkWindow * win = currentWindow();

		if (!win) return;

		bool b = false;
		emit prepareModelForSaving(win,&b);

		if (!b) return;

		QString fileName = win->filename;

		if (fileName.isEmpty())
		{
			fileName =
				QFileDialog::getSaveFileName(this, tr("Save Current Model"),
				previousFileName,
				tr("XML Files (*.xml)"));
			if (fileName.isEmpty())
				return;
			else
				previousFileName = fileName;
		}
		QFile file (fileName);

		if (!file.open(QFile::WriteOnly | QFile::Text)) {
			QMessageBox::warning(this, tr("Tinkercell File"),
				tr("Cannot write file %1:\n%2.")
				.arg(fileName)
				.arg(file.errorString()));
			return;
		}

		emit saveModel(fileName);
	}

	void MainWindow::saveWindowAs()
	{
		QString fileName =
			QFileDialog::getSaveFileName(this, tr("Save Current Model"),
			previousFileName,
			tr("XML Files (*.xml)"));
		if (fileName.isEmpty())
			return;

		previousFileName = fileName;
		QFile file (fileName);

		if (!file.open(QFile::WriteOnly | QFile::Text)) {
			QMessageBox::warning(this, tr("Tinkercell File"),
				tr("Cannot write file %1:\n%2.")
				.arg(fileName)
				.arg(file.errorString()));
			return;
		}

		emit saveModel(fileName);
	}

	void MainWindow::open(const QString& fileName)
	{
		previousFileName = fileName;

		QFile file (fileName);

		if (!file.open(QFile::ReadOnly | QFile::Text)) {
			QMessageBox::warning(this, tr("Tinkercell File"),
				tr("Cannot read file %1:\n%2.")
				.arg(fileName)
				.arg(file.errorString()));
			return;
		}
		emit loadModel(fileName);
	}

	void MainWindow::open()
	{
		QStringList fileNames =
			QFileDialog::getOpenFileNames(this, tr("Open File"),
			previousFileName /*, tr("XML Files (*.xml)")*/);
		for (int i=0; i < fileNames.size(); ++i)
			if (!fileNames[i].isEmpty())
				open(fileNames[i]);
	}

	/*! \brief print the current scene*/
	void MainWindow::print()
	{
		QPrinter printer(QPrinter::HighResolution);
		//printer.setResolution(300);
		printer.setPageSize(QPrinter::B0);

		if (QPrintDialog(&printer,this).exec() == QDialog::Accepted)
		{
			if (currentScene())
				currentScene()->print(&printer);
			else
				if (currentTextEditor())
					currentTextEditor()->print(&printer);

		}
	}

	/*! \brief print the current scene*/
	void MainWindow::printToFile()
	{
		GraphicsScene * scene = currentScene();
		if (!scene) return;

		QString def = previousFileName;
		def.replace(QRegExp("\\..*$"),tr(""));

		QString fileName =
			QFileDialog::getSaveFileName(this, tr("Print to File"),
			def,
			//tr("PDF Files (*.pdf *.PDF)"));
			tr("PNG Files (*.png *.PNG)"));
		if (fileName.isEmpty())
			return;

		previousFileName = fileName;

		/*
		QPrinter printer(QPrinter::HighResolution);
		//printer.setResolution(150);
		printer.setOutputFormat(QPrinter::PdfFormat);
		printer.setOrientation(QPrinter::Landscape);
		printer.setPageSize(QPrinter::A4);
		//printer.setPageSize(QPrinter::B0);
		printer.setOutputFileName(fileName);
		*/

		/*
		QSvgGenerator printer;
		printer.setFileName(fileName);
		*/

		QRectF viewport = scene->viewport();
		int w = 800;
		int h = (int)(viewport.height() * w/viewport.width());
		QImage printer(w,h,QImage::Format_ARGB32);
		//QImage printer(1024,1024,QImage::Format_ARGB32);
		scene->print(&printer);
		printer.save(fileName,"png");

	}

	QDockWidget * MainWindow::addToolWindow(QWidget * tool, TOOL_WINDOW_OPTION option, Qt::DockWidgetArea initArea, Qt::DockWidgetAreas allowedAreas, bool inMenu)
	{
		if (!tool || toolWindows.contains(tool)) return 0;

		toolWindows << tool;

		if (option == DockWidget)
		{
			QDockWidget *dock = new QDockWidget(tool->windowTitle(), this);
			dock->setWindowIcon(tool->windowIcon());
			dock->setAllowedAreas(allowedAreas);
			dock->setWidget(tool);
			addDockWidget(initArea,dock);
			if (inMenu)
				viewMenu->addAction(dock->toggleViewAction());

			return dock;
		}

		QDockWidget * dock = 0;

		if (!toolBox || option == NewToolBoxWidget)
		{
			dock = new QDockWidget(tr("Tools Window"), this);
			if (option == NewToolBoxWidget)
				dock->setWindowTitle(tool->windowTitle());
			toolBox = new QToolBox;
			toolBox->setMinimumWidth(300);
			dock->setWidget(toolBox);
			dock->setAllowedAreas(allowedAreas);
			addDockWidget(initArea,dock);
			if (inMenu)
				viewMenu->addAction(dock->toggleViewAction());
		}
		else
		{
			dock = static_cast<QDockWidget*>(toolBox->parentWidget()); //safe?
		}

		toolBox->addItem(tool,tool->windowIcon(),tool->windowTitle());
		toolBox->setCurrentWidget(tool);

		return dock;
	}

	Tool * MainWindow::tool(const QString& s) const
	{
		if (toolsHash.contains(s))
			return toolsHash.value(s);
		return 0;
	}

	QList<Tool*> MainWindow::tools() const
	{
		return toolsHash.values();
	}

	void MainWindow::addTool(Tool * tool)
	{
		if (!tool) return;

		bool add = true;
		emit toolAboutToBeLoaded(tool,&add);

		if (!toolsHash.contains(tool->name) && add)
		{
			toolsHash.insert(tool->name,tool);
			if (tool->mainWindow != this)
			{
				tool->setMainWindow( static_cast<MainWindow*>(this) );
			}
			emit toolLoaded(tool);
		}
		else
		if (!add)
		{
			if (toolsHash.contains(tool->name))
				toolsHash.remove(tool->name);
			if (!tool->parentWidget())
				delete tool;
		}
	}

	GraphicsScene* MainWindow::currentScene()
	{
		NetworkWindow * net = currentWindow();
		if (net)
			return net->scene;
		return 0;
	}

	TextEditor* MainWindow::currentTextEditor()
	{
		NetworkWindow * net = currentWindow();
		if (net)
			return net->textEditor;
		return 0;
	}

	NetworkWindow* MainWindow::currentWindow()
	{	
	    return currentNetworkWindow;
	}

	QList<NetworkWindow*> MainWindow::allWindows()
	{
		return allNetworkWindows;
	}

	void MainWindow::fitAll()
	{
		if (currentScene())
			currentScene()->fitAll();
	}

	void MainWindow::fitSelected()
	{
		if (currentScene())
			currentScene()->fitSelected();
	}

	void MainWindow::initializeMenus(bool enableScene, bool enableText)
	{
		fileMenu = menuBar()->addMenu(tr("&File"));
		toolBarBasic = new QToolBar(tr("Open/save/new toolbar"),this);
		toolBarEdits = new QToolBar(tr("Edit options"),this);
		toolBarForTools = new QToolBar(tr("Plug-ins"),this);

		if (enableScene)
		{
			QAction* newAction = fileMenu->addAction(QIcon(tr(":/images/newscene.png")),tr("New Graphics Scene"));
			newAction->setShortcut(QKeySequence::New);
			connect (newAction, SIGNAL(triggered()),this,SLOT(newGraphicsWindow()));
			toolBarBasic->addAction(newAction);
		}

		if (enableText)
		{
			QAction* newAction2 = fileMenu->addAction(QIcon(tr(":/images/newtext.png")),tr("New Text Editor"));
			newAction2->setShortcut(tr("CTRL+SHIFT+N"));
			connect (newAction2, SIGNAL(triggered()),this,SLOT(newTextWindow()));
			toolBarBasic->addAction(newAction2);
		}

		QAction* openAction = fileMenu->addAction(QIcon(tr(":/images/open.png")),tr("Open"));
		openAction->setShortcut(QKeySequence::Open);
		connect (openAction, SIGNAL(triggered()),this,SLOT(open()));

		QAction* saveAction = fileMenu->addAction(QIcon(tr(":/images/save.png")),tr("Save"));
		saveAction->setShortcut(QKeySequence::Save);
		connect (saveAction, SIGNAL(triggered()),this,SLOT(saveWindow()));

		QAction* saveAsAction = fileMenu->addAction(QIcon(tr(":/images/save.png")),tr("SaveAs"));
		connect (saveAsAction, SIGNAL(triggered()),this,SLOT(saveWindowAs()));

		QAction* closeAction = fileMenu->addAction(QIcon(tr(":/images/close.png")), tr("Close Page"));
		closeAction->setShortcut(QKeySequence::Close);
		connect (closeAction, SIGNAL(triggered()),this,SLOT(closeWindow()));

		fileMenu->addSeparator();

		QAction * printAction = fileMenu->addAction(QIcon(tr(":/images/print.png")),tr("Print"));
		printAction->setShortcut(QKeySequence::Print);
		connect(printAction,SIGNAL(triggered()),this,SLOT(print()));

		QAction * printToFileAction = fileMenu->addAction(QIcon(tr(":/images/camera.png")),tr("Screenshot"));
		printToFileAction->setShortcut(tr("Ctrl+F5"));
		connect(printToFileAction,SIGNAL(triggered()),this,SLOT(printToFile()));

		fileMenu->addSeparator();

		QAction * exitAction = fileMenu->addAction(QIcon(tr(":/images/exit.png")), tr("Exit"));
		exitAction->setShortcut(tr("Ctrl+Q"));
		connect(exitAction,SIGNAL(triggered()),this,SLOT(close()));

		editMenu = menuBar()->addMenu(tr("&Edit"));
		QAction * undoAction = editMenu->addAction(QIcon(tr(":/images/undo.png")),tr("Undo"));
		undoAction->setShortcut(QKeySequence::Undo);
		connect(undoAction,SIGNAL(triggered()),this,SLOT(undo()));


		QAction * redoAction = editMenu->addAction(QIcon(tr(":/images/redo.png")),tr("Redo"));
		redoAction->setShortcut(QKeySequence::Redo);
		connect(redoAction,SIGNAL(triggered()),this,SLOT(redo()));

		QAction* fitAll = editMenu->addAction(QIcon(tr(":/images/fitAll.png")),tr("Fit all"));
		fitAll->setShortcut(tr("F5"));
		connect(fitAll,SIGNAL(triggered()),this,SLOT(fitAll()));

		viewMenu = menuBar()->addMenu(tr("&View"));

		settingsMenu = menuBar()->addMenu(tr("&Settings"));
		QAction * changeUserHome = settingsMenu->addAction(QIcon(tr(":/images/appicon.png")), tr("Set Home Directory"));
		connect (changeUserHome, SIGNAL(triggered()),this,SLOT(setUserHome()));

		QMenu * setGridModeMenu = settingsMenu->addMenu(tr("Grid mode"));

		setGridModeMenu->addAction(tr("Grid ON"),this,SLOT(gridOn()));
		setGridModeMenu->addAction(tr("Grid OFF"),this,SLOT(gridOff()));
		setGridModeMenu->addAction(tr("Grid size"),this,SLOT(setGridSize()));

		helpMenu = menuBar()->addMenu(tr("&Help"));

		QAction * copyAction = new QAction(QIcon(":/images/copy.png"),tr("Copy"),this);
		editMenu->addAction(copyAction);
		copyAction->setToolTip(tr("Copy selected items"));
		copyAction->setShortcut(QKeySequence::Copy);
		connect(copyAction,SIGNAL(triggered()),this,SLOT(copy()));

		QAction * cutAction = new QAction(QIcon(":/images/cut.png"),tr("Cut"),this);
		editMenu->addAction(cutAction);
		cutAction->setToolTip(tr("Cut selected items"));
		cutAction->setShortcut(QKeySequence::Cut);
		connect(cutAction,SIGNAL(triggered()),this,SLOT(cut()));

		QAction * pasteAction = new QAction(QIcon(":/images/paste.png"),tr("Paste"),this);
		editMenu->addAction(pasteAction);
		pasteAction->setToolTip(tr("Paste copied items"));
		pasteAction->setShortcut(QKeySequence::Paste);
		connect(pasteAction,SIGNAL(triggered()),this,SLOT(paste()));

		QAction * deleteAction = new QAction(QIcon(":/images/delete.png"),tr("Delete"),this);
		editMenu->addAction(deleteAction);
		deleteAction->setToolTip(tr("Delete selected items"));
		connect(deleteAction,SIGNAL(triggered()),this,SLOT(remove()));

		QAction * selectAllAction = new QAction(tr("Select all"),this);
		editMenu->addAction(selectAllAction);
		selectAllAction->setToolTip(tr("Select all items"));
		selectAllAction->setShortcut(QKeySequence::SelectAll);
		connect(selectAllAction,SIGNAL(triggered()),this,SLOT(selectAll()));

		QAction* arrowAction = editMenu->addAction(QIcon(tr(":/images/arrow.png")),tr("Cursor"));
		connect(arrowAction,SIGNAL(triggered()),this,SLOT(sendEscapeSignal()));

		toolBarBasic->addAction(openAction);
		toolBarBasic->addAction(closeAction);
		toolBarBasic->addAction(saveAction);

		toolBarEdits->addAction(arrowAction);
		toolBarEdits->addAction(undoAction);
		toolBarEdits->addAction(redoAction);
		toolBarEdits->addAction(copyAction);
		toolBarEdits->addAction(cutAction);
		toolBarEdits->addAction(pasteAction);
		toolBarEdits->addAction(deleteAction);

		/*QSize iconSize(16,16);
		toolBarBasic->setIconSize(iconSize);
		toolBarEdits->setIconSize(iconSize);
		toolBarForTools->setIconSize(iconSize);*/

		addToolBar(Qt::TopToolBarArea, toolBarBasic);
		addToolBar(Qt::TopToolBarArea, toolBarEdits);
		addToolBar(Qt::TopToolBarArea, toolBarForTools);

		contextScreenMenu.addAction(fitAll);
		contextScreenMenu.addAction(closeAction);
		contextScreenMenu.addAction(undoAction);
		contextScreenMenu.addAction(redoAction);

		contextEditorMenu.addAction(undoAction);
		contextEditorMenu.addAction(redoAction);
		contextEditorMenu.addAction(closeAction);

		contextItemsMenu.addAction(copyAction);
		contextItemsMenu.addAction(cutAction);
		contextScreenMenu.addAction(pasteAction);
		contextItemsMenu.addAction(deleteAction);

		contextSelectionMenu.addAction(copyAction);
		contextSelectionMenu.addAction(cutAction);
		contextEditorMenu.addAction(pasteAction);

		contextScreenMenu.addAction(fitAll);
		contextScreenMenu.addAction(closeAction);

		contextScreenMenu.addAction(undoAction);
		contextScreenMenu.addAction(redoAction);
	}

	void MainWindow::sendEscapeSignal(const QWidget * widget)
	{
		emit escapeSignal(widget);
	}

	QUndoStack * MainWindow::historyStack()
	{
		if (historyWindow.stack())
			return historyWindow.stack();
		return 0;
	}

	QUndoView * MainWindow::historyWidget()
	{
		return &historyWindow;
	}

	void MainWindow::undo()
	{
		if (historyWindow.stack())
			historyWindow.stack()->undo();
	}

	void MainWindow::redo()
	{
		if (historyWindow.stack())
			historyWindow.stack()->redo();
	}
	void MainWindow::copy()
	{
		if (currentScene())
			currentScene()->copy();
		else
			if (currentTextEditor())
				currentTextEditor()->copy();
	}

	void MainWindow::cut()
	{
		if (currentScene())
			currentScene()->cut();
		else
			if (currentTextEditor())
				currentTextEditor()->cut();
	}

	void MainWindow::remove()
	{
		if (currentScene())
			currentScene()->removeSelected();
	}

	void MainWindow::selectAll()
	{
		if (currentScene())
			currentScene()->selectAll();
		else
			if (currentTextEditor())
				currentTextEditor()->selectAll();
	}

	void MainWindow::paste()
	{
		if (currentScene())
			currentScene()->paste();
		else
			if (currentTextEditor())
				currentTextEditor()->paste();
	}

	void MainWindow::closeEvent(QCloseEvent *event)
	{
		QList<NetworkWindow*> list = allNetworkWindows;
		for (int i=0; i < list.size(); ++i)
			if (list[i] && allNetworkWindows.contains(list[i]))			
				list[i]->close();
		
		if (allNetworkWindows.isEmpty())
		{
			event->accept();
		}
		else
		{
			event->ignore();
			return;
		}

		QList<QString> keys = this->toolsHash.keys();
		QList<Tool*> toolsHash = this->toolsHash.values();
		for (int i=0; i < toolsHash.size(); ++i)
		{
			if (toolsHash[i])
			{
				if (toolsHash[i]->parentWidget() == 0)
				{
					for (int j=0; j < toolsHash.size(); ++j)
						if (i != j && toolsHash[j] == toolsHash[i])
							toolsHash[j] = 0;

					disconnect(toolsHash[i]);
					toolsHash[i]->close();
					delete toolsHash[i];
				}
				else
				{

				}
			}
		}
	}

	void MainWindow::dragEnterEvent(QDragEnterEvent *event)
	{
		event->acceptProposedAction();
	}

	void MainWindow::dropEvent(QDropEvent * event)
	{
		QList<QUrl> urlList;
		QList<QFileInfo> files;
		QString fName;
		QFileInfo info;

		if (event->mimeData()->hasUrls())
		{
			urlList = event->mimeData()->urls(); // returns list of QUrls

			// if just text was dropped, urlList is empty (size == 0)
			if ( urlList.size() > 0) // if at least one QUrl is present in list
			{
				fName = urlList[0].toLocalFile(); // convert first QUrl to local path
				info.setFile( fName ); // information about file
				if ( info.isFile() )
					files += info;
			}
		}
		event->acceptProposedAction();
		dragAndDropFiles(files);
	}

	void MainWindow::dragAndDropFiles(const QList<QFileInfo>& files)
	{
		for (int i=0; i < files.size(); ++i)
		{
			if (files[i].isFile())
			{
				if (QLibrary::isLibrary(files[i].fileName()))
				{
					loadDynamicLibrary(files[i].absoluteFilePath());
				}
				else
				{
					previousFileName = files[i].absoluteFilePath();
					emit loadModel(files[i].absoluteFilePath());
				}
			}
		}

		emit filesDropped(files);
	}

	void MainWindow::itemsRemovedSlot(GraphicsScene * scene, const QList<QGraphicsItem*>& item, const QList<ItemHandle*>& handles)
	{
		if (handles.size() > 0 && scene)
			emit itemsRemoved(scene->networkWindow, handles);
	}

	void MainWindow::itemsInsertedSlot(GraphicsScene * scene, const QList<QGraphicsItem*>& item, const QList<ItemHandle*>& handles)
	{
		if (handles.size() > 0 && scene)
			emit itemsInserted(scene->networkWindow, handles);
	}

	void MainWindow::itemsRemovedSlot(TextEditor * editor, const QList<TextItem*>& item, const QList<ItemHandle*>& handles)
	{
		if (handles.size() > 0 && editor)
			emit itemsRemoved(editor->networkWindow, handles);
	}

	void MainWindow::itemsInsertedSlot(TextEditor * editor, const QList<TextItem*>& item, const QList<ItemHandle*>& handles)
	{
		if (handles.size() > 0 && editor)
			emit itemsInserted(editor->networkWindow, handles);
	}

	/***********************************************/
	/**************C Functions**********************/
	/***********************************************/
	void MainWindow::zoom(QSemaphore* sem, qreal factor)
	{
		if (currentScene())
		{
			currentScene()->scaleView(factor);
		}
		if (sem)
			sem->release();
	}

	void MainWindow::clearText(QSemaphore* sem)
	{
	    if (console())
            console()->clear();
		if (sem)
			sem->release();
	}

	void MainWindow::outputText(QSemaphore* sem,const QString& text)
	{
		if (console())
            console()->message(text);
		if (sem)
			sem->release();
	}

	void MainWindow::errorReport(QSemaphore* sem,const QString& text)
	{
		if (console())
            console()->error(text);
		if (sem)
			sem->release();
	}

	void MainWindow::printFile(QSemaphore* sem,const QString& filename)
	{
		QString appDir = QCoreApplication::applicationDirPath();

		QString name[] = {	MainWindow::userHome() + tr("/") + filename,
			filename,
			QDir::currentPath() + tr("/") + filename,
			appDir + tr("/") + filename };

		QFile file;
		bool opened = false;
		for (int i=0; i < 4; ++i)
		{
			file.setFileName(name[i]);
			if (file.open(QFile::ReadOnly | QFile::Text))
			{
				opened = true;
				break;
			}
		}
		if (!opened)
		{
			if (console())
                console()->error(tr("file not found"));
		}
		else
		{
			QString allText(file.readAll());
			if (console())
                console()->message(allText);
			file.close();
		}


		if (sem)
			sem->release();
	}

	void MainWindow::outputTable(QSemaphore* sem,const DataTable<qreal>& table)
	{
		if (console())
            console()->printTable(table);
		if (sem)
			sem->release();
	}
	void MainWindow::createInputWindow(QSemaphore* s,const DataTable<qreal>& data, const QString& dll,const QString& function,const QString& title)
	{
		SimpleInputWindow::CreateWindow(this,title,dll,function,data);
		if (s)
			s->release();
	}
	void MainWindow::createInputWindow(QSemaphore* s,const DataTable<qreal>& dat,const QString& title, MatrixInputFunction f)
	{
		SimpleInputWindow::CreateWindow(this,title,f,dat);
		if (s)
			s->release();
	}
	void MainWindow::addInputWindowOptions(QSemaphore* s,const QString& name, int i, int j,const QStringList& options)
	{
		SimpleInputWindow::AddOptions(name,i,j,options);
		if (s)
			s->release();
	}
	void MainWindow::addInputWindowCheckbox(QSemaphore* s,const QString& name, int i, int j)
	{
		SimpleInputWindow::AddOptions(name,i,j);
		if (s)
			s->release();
	}
	void MainWindow::openNewWindow(QSemaphore* s,const QString& name)
	{
		newGraphicsWindow();
		if (currentWindow())
			currentWindow()->setWindowTitle(name);
		if (s)
			s->release();
	}
	void MainWindow::isWindows(QSemaphore* s, int * i)
	{
#ifdef Q_WS_WIN
		if (i) (*i) = 1;
#else
		if (i) (*i) = 0;
#endif
		if (s)
			s->release();
	}
	void MainWindow::isMac(QSemaphore* s, int * i)
	{
#ifdef Q_WS_MAC
		if (i) (*i) = 1;
#else
		if (i) (*i) = 0;
#endif
		if (s)
			s->release();
	}
	void MainWindow::isLinux(QSemaphore* s, int * i)
	{
#ifdef Q_WS_WIN
		if (i) (*i) = 0;
#else
#ifdef Q_WS_MAC
		if (i) (*i) = 0;
#else
		if (i) (*i) = 1;
#endif
#endif
		if (s)
			s->release();
	}

	void MainWindow::appDir(QSemaphore* s, QString * dir)
	{
		QString appDir = QCoreApplication::applicationDirPath();
		if (dir)
			(*dir) = appDir;
		if (s)
			s->release();
	}

	void MainWindow::allItems(QSemaphore* s,QList<ItemHandle*>* returnPtr)
	{
		NetworkWindow * win = currentWindow();
		if (!win || !returnPtr)
		{
			if (s) s->release();
			return;
		}

		if (returnPtr)
			(*returnPtr) = win->allHandles();

		if (s)
			s->release();
	}

	void MainWindow::itemsOfFamily(QSemaphore* s,QList<ItemHandle*>* returnPtr, const QString& family)
	{
		NetworkWindow * win = currentWindow();
		if (!win || !returnPtr)
		{
			if (s) s->release();
			return;
		}

		if (returnPtr)
		{
			QList<ItemHandle*> handles = win->allHandles();
			for (int i=0; i < handles.size(); ++i)
				if (handles[i] && handles[i]->isA(family))
					(*returnPtr) += handles[i];
		}

		if (s)
			s->release();
	}

	void MainWindow::itemsOfFamily(QSemaphore* s,QList<ItemHandle*>* returnPtr,const QList<ItemHandle*>& handles,const QString& family)
	{
		NetworkWindow * win = currentWindow();
		if (!win || !returnPtr)
		{
			if (s) s->release();
			return;
		}

		if (returnPtr)
		{
			for (int i=0; i < handles.size(); ++i)
				if (handles[i] && handles[i]->isA(family))
					(*returnPtr) += handles[i];
		}

		if (s)
			s->release();
	}

	void MainWindow::selectedItems(QSemaphore* s,QList<ItemHandle*>* returnPtr)
	{
		NetworkWindow * win = currentWindow();
		if (!win || !returnPtr)
		{
			if (s) s->release();
			return;
		}

		QList<ItemHandle*> list = win->selectedHandles();

		if (returnPtr)
			(*returnPtr) = list;

		if (s)
			s->release();
	}

	void MainWindow::itemNames(QSemaphore* s,QStringList* list,const QList<ItemHandle*>& items)
	{
		if (list)
		{
			(*list).clear();
			for (int i=0; i < items.size(); ++i)
			{
				if (items[i])
					(*list) << items[i]->fullName(tr("_"));
			}
		}
		if (s)
			s->release();
	}

	void MainWindow::itemName(QSemaphore* s,QString* name,ItemHandle* handle)
	{
		if (handle && name)
		{
			(*name) = handle->fullName(tr("_"));
		}
		if (s)
			s->release();
	}

	void MainWindow::setName(QSemaphore* s,ItemHandle* handle,const QString& name)
	{
		if (handle && !name.isNull() && !name.isEmpty() && currentWindow())
		{
			currentWindow()->rename(handle,name);
		}
		if (s)
			s->release();
	}

	void MainWindow::itemFamily(QSemaphore* s,QString* family,ItemHandle* handle)
	{
		if (handle && family && handle->family())
		{
			(*family) = handle->family()->name;
		}
		if (s)
			s->release();
	}

	void MainWindow::isA(QSemaphore* s,int* boolean,ItemHandle* handle,const QString& family)
	{
		if (handle && boolean)
		{
			(*boolean) = 0;

			if (handle->family())
			{
				(*boolean) = (int)(handle->family()->isA(family));
			}
		}
		if (s)
			s->release();
	}

	void MainWindow::findItem(QSemaphore* s,ItemHandle** returnPtr,const QString& name)
	{
		NetworkWindow * win = currentWindow();
		if (!win || !returnPtr)
		{
			if (returnPtr)
				(*returnPtr) = 0;
			if (s) s->release();
			return;
		}

		(*returnPtr) = 0;

		if (win->symbolsTable.handlesFullName.contains(name))
			(*returnPtr) = win->symbolsTable.handlesFullName[name];
		else
		{
			QString s = name;

			if (win->symbolsTable.handlesFullName.contains(s))
				(*returnPtr) = win->symbolsTable.handlesFullName[s];
			else
			{
				if (win->symbolsTable.handlesFirstName.contains(s))
					(*returnPtr) = win->symbolsTable.handlesFirstName[s];
				else
					if (win->symbolsTable.dataRowsAndCols.contains(s))
						(*returnPtr) = win->symbolsTable.dataRowsAndCols[s].first;
			}

			if ((*returnPtr) == 0)
			{
				s.replace(tr("_"),tr("."));
				if (win->symbolsTable.handlesFullName.contains(s))
					(*returnPtr) = win->symbolsTable.handlesFullName[s];
				else
				{
					if (win->symbolsTable.handlesFirstName.contains(s))
						(*returnPtr) = win->symbolsTable.handlesFirstName[s];
					else
						if (win->symbolsTable.dataRowsAndCols.contains(s))
							(*returnPtr) = win->symbolsTable.dataRowsAndCols[s].first;
				}
			}
		}

		if (s)
			s->release();
	}

	void MainWindow::findItems(QSemaphore* s,QList<ItemHandle*>* returnPtr,const QStringList& names)
	{
		NetworkWindow * win = currentWindow();
		if (!win || !returnPtr)
		{
			if (returnPtr)
				returnPtr->clear();
			if (s) s->release();
			return;
		}

		returnPtr->clear();
		QString name;
		ItemHandle * handle = 0;

		for (int i=0; i < names.size(); ++i)
		{
			name = names[i];
			handle = 0;

			if (win->symbolsTable.handlesFullName.contains(name))
				handle = win->symbolsTable.handlesFullName[name];
			else
			{
				QString s = name;

				if (win->symbolsTable.handlesFullName.contains(s))
					handle = win->symbolsTable.handlesFullName[s];
				else
				{
					if (win->symbolsTable.handlesFirstName.contains(s))
						handle = win->symbolsTable.handlesFirstName[s];
					else
						if (win->symbolsTable.dataRowsAndCols.contains(s))
							handle= win->symbolsTable.dataRowsAndCols[s].first;
				}

				if (handle == 0)
				{
					s.replace(tr("_"),tr("."));
					if (win->symbolsTable.handlesFullName.contains(s))
						handle = win->symbolsTable.handlesFullName[s];
					else
					{
						if (win->symbolsTable.handlesFirstName.contains(s))
							handle = win->symbolsTable.handlesFirstName[s];
						else
							if (win->symbolsTable.dataRowsAndCols.contains(s))
								handle = win->symbolsTable.dataRowsAndCols[s].first;
					}
				}
			}

			if (handle)
				returnPtr->append(handle);
		}

		if (s)
			s->release();
	}

	/*! \brief select an item in the current scene.
	* \param graphics item pointer
	*/
	void MainWindow::select(QSemaphore* sem,ItemHandle* item)
	{
		GraphicsScene * scene = currentScene();
		if (item && scene)
			scene->select(item->graphicsItems);
		if (sem)
			sem->release();
	}

	/*! \brief deselect all items in the current scene.
	* \param graphics item pointer
	*/
	void MainWindow::deselect(QSemaphore* sem)
	{
		GraphicsScene * scene = currentScene();
		if (scene)
			scene->deselect();
		if (sem)
			sem->release();
	}


	/*! \brief delete an item in the current scene. This function is intended to be used
	in a C-style plugin or script.
	*/
	void MainWindow::removeItem(QSemaphore* sem, ItemHandle * item)
	{
		if (currentScene() && item)
			currentScene()->remove(tr("item removed"),item->graphicsItems);
		//else
		//if (currentTextEditor() && item)

		if (sem)
			sem->release();
	}

	void MainWindow::getX(QSemaphore * s, qreal * returnPtr, ItemHandle * item)
	{
		if (item == 0 || item->graphicsItems.isEmpty() || item->graphicsItems[0] == 0)
		{
			if (s) s->release();
			return;
		}
		if (returnPtr)
			(*returnPtr) = item->graphicsItems[0]->scenePos().x();
		if (s)
			s->release();
	}

	void MainWindow::getY(QSemaphore * s, qreal * returnPtr, ItemHandle * item)
	{
		if (item == 0 || item->graphicsItems.isEmpty() || item->graphicsItems[0] == 0)
		{
			if (s) s->release();
			return;
		}
		if (returnPtr)
			(*returnPtr) = item->graphicsItems[0]->scenePos().y();
		if (s)
			s->release();
	}

	void MainWindow::moveSelected(QSemaphore* sem,qreal dx, qreal dy)
	{
		if (currentScene())
		{
			currentScene()->move( currentScene()->moving() , QPointF(dx,dy) );
		}
		if (sem)
			sem->release();
	}

	void MainWindow::setPos(QSemaphore* sem, ItemHandle * item, qreal x, qreal y)
	{
		if (currentScene() && item && !item->graphicsItems.isEmpty() && item->graphicsItems[0])
		{
			currentScene()->move( item->graphicsItems , QPointF(x,y) - item->graphicsItems[0]->scenePos() );
		}
		if (sem)
			sem->release();
	}

	void MainWindow::setPos(QSemaphore* sem,const QList<ItemHandle*>& items, DataTable<qreal>& pos)
	{
		if (currentScene() && !items.isEmpty() && items.size() == pos.rows() && pos.cols() == 2)
		{
			QList<QGraphicsItem*> graphicsItems;
			QGraphicsItem* item;
			QList<QPointF> p;
			QPointF diff, target;
			ConnectionGraphicsItem * connection = 0;
			NodeGraphicsItem *  node = 0;
			int m;
			for (int i=0; i < items.size(); ++i)
			{
				for (int j=0; j < items[i]->graphicsItems.size(); ++j)
					if ((item = items[i]->graphicsItems[j]))
					{
						if ((connection = qgraphicsitem_cast<ConnectionGraphicsItem*>(item)))
						{
							ConnectionGraphicsItem::ControlPoint * cp;
							if ((cp = connection->centerPoint()))
							{
								graphicsItems << cp;
								diff = QPointF(pos.value(i,0),pos.value(i,1)) - cp->scenePos();
								p << diff;

								for (int k=0; k < connection->curveSegments.size(); ++k)
								{
									ConnectionGraphicsItem::ControlPoint * cp1 = connection->curveSegments[k].first();
									if (cp1 && cp && cp1 != cp)
									{
										QPointF p1 = cp1->scenePos(), p2 = QPointF(pos.value(i,0),pos.value(i,1));
										if ((node = qgraphicsitem_cast<NodeGraphicsItem*>(cp1->parentItem())) &&
											((m = items.indexOf(node->handle())) > -1))
										{
											p1 = QPointF(pos.value(m,0),pos.value(m,1)) - node->scenePos();
										}
										for (int l=1; l < connection->curveSegments[k].size(); ++l)
										{
											if (connection->curveSegments[k][l] && connection->curveSegments[k][l] != cp)
											{
												target =
													p1*((double)(connection->curveSegments[k].size() - l - 0.5))/((double)(connection->curveSegments[k].size()))
													+
													p2*((double)(l + 0.5))/((double)(connection->curveSegments[k].size()));
												diff = target - connection->curveSegments[k][l]->scenePos();
												if ((m = graphicsItems.indexOf(connection->curveSegments[k][l])) > -1)
												{
													p[m] = (p[m] + diff)/2.0;
												}
												else
												{
													p << diff;
													graphicsItems << connection->curveSegments[k][l];
												}
											}
										}
									}
								}
							}
						}
						else
						{
							graphicsItems << item;
							p << QPointF(pos.value(i,0),pos.value(i,1)) - items[i]->graphicsItems[j]->scenePos();
						}
					}
			}
			currentScene()->move( graphicsItems , p );
		}
		else
			if (console())
                console()->message(QString::number(items.size()) + tr(" ") + QString::number(pos.cols()));

		if (sem)
			sem->release();
	}

	void MainWindow::getPos(QSemaphore* sem,const QList<ItemHandle*>& items, DataTable<qreal>* pos)
	{
		if (currentScene() && !items.isEmpty() && pos)
		{
			QList<QGraphicsItem*> graphicsItems;
			QList<QPointF> p;
			for (int i=0; i < items.size(); ++i)
			{
				for (int j=0; j < items[i]->graphicsItems.size(); ++j)
				{
					if (qgraphicsitem_cast<NodeGraphicsItem*>(items[i]->graphicsItems[j]) || (j == (items[i]->graphicsItems.size() - 1)))
					{
						p << items[i]->graphicsItems[j]->scenePos();
						break;
					}
					else
						if (qgraphicsitem_cast<ConnectionGraphicsItem*>(items[i]->graphicsItems[j]))
						{
							p << (qgraphicsitem_cast<ConnectionGraphicsItem*>(items[i]->graphicsItems[j]))->centerLocation();
							break;
						}
				}
			}

			(*pos).resize(p.size(),2);
			(*pos).colName(0) = tr("x");
			(*pos).colName(1) = tr("y");
			for (int i=0; i < p.size(); ++i)
			{
				(*pos).value(i,0) = p[i].rx();
				(*pos).value(i,1) = p[i].ry();
			}
		}
		if (sem)
			sem->release();
	}

	void MainWindow::getNumericalData(QSemaphore* sem,qreal* ret,ItemHandle* item,const QString& tool, const QString& row, const QString& col)
	{
		if (!item && currentWindow())
			item = &(currentWindow()->symbolsTable.modelItem);

		if (ret && item && item->data && item->data->numericalData.contains(tool))
		{
			DataTable<qreal>& dat = item->data->numericalData[tool];
			if (dat.rowNames().contains(row) &&  dat.colNames().contains(col))
				(*ret) = dat.value(row,col);
			else
				(*ret) = dat.value(0,0);
		}
		if (sem)
			sem->release();
	}

	void MainWindow::getStringData(QSemaphore* sem,QString* ret,ItemHandle* item,const QString& tool, const QString& row, const QString& col)
	{
		if (!item && currentWindow())
			item = &(currentWindow()->symbolsTable.modelItem);

		if (ret && item && item->data && item->data->textData.contains(tool))
		{
			DataTable<QString>& dat = item->data->textData[tool];
			if (dat.rowNames().contains(row) &&  dat.colNames().contains(col))
				(*ret) = dat.value(row,col);
			else
				(*ret) = dat.value(0,0);
		}
		if (sem)
			sem->release();
	}

	void MainWindow::getNumericalDataRows(QSemaphore* sem ,QStringList* list,ItemHandle* item,const QString& tool)
	{
		if (!item && currentWindow())
			item = &(currentWindow()->symbolsTable.modelItem);

		if (list && item && item->data && item->data->numericalData.contains(tool))
		{
			DataTable<qreal>& dat = item->data->numericalData[tool];
			(*list) = dat.getRowNames();
		}
		if (sem)
			sem->release();
	}

	void MainWindow::getNumericalDataCols(QSemaphore* sem ,QStringList* list,ItemHandle* item,const QString& tool)
	{
		if (!item && currentWindow())
			item = &(currentWindow()->symbolsTable.modelItem);

		if (list && item && item->data && item->data->numericalData.contains(tool))
		{
			DataTable<qreal>& dat = item->data->numericalData[tool];
			(*list) = dat.getColNames();
		}
		if (sem)
			sem->release();
	}

	void MainWindow::getStringDataRows(QSemaphore* sem ,QStringList* list,ItemHandle* item,const QString& tool)
	{
		if (!item && currentWindow())
			item = &(currentWindow()->symbolsTable.modelItem);

		if (list && item && item->data && item->data->textData.contains(tool))
		{
			DataTable<QString>& dat = item->data->textData[tool];
			(*list) = dat.getRowNames();
		}
		if (sem)
			sem->release();
	}

	void MainWindow::getStringDataCols(QSemaphore* sem ,QStringList* list,ItemHandle* item,const QString& tool)
	{
		if (!item && currentWindow())
			item = &(currentWindow()->symbolsTable.modelItem);

		if (list && item && item->data && item->data->textData.contains(tool))
		{
			DataTable<QString>& dat = item->data->textData[tool];
			(*list) = dat.getColNames();
		}
		if (sem)
			sem->release();
	}

	void MainWindow::getNumericalDataMatrix(QSemaphore* sem,DataTable<qreal>* dat,ItemHandle* item,const QString& tool)
	{
		if (!item && currentWindow())
			item = &(currentWindow()->symbolsTable.modelItem);

		if (dat && item && item->data && item->data->numericalData.contains(tool))
		{
			(*dat) = item->data->numericalData[tool];
		}
		if (sem)
			sem->release();
	}

	void MainWindow::setNumericalDataMatrix(QSemaphore* sem,ItemHandle* item,const QString& tool, const DataTable<qreal>& dat)
	{
		if (!item && currentWindow())
			item = &(currentWindow()->symbolsTable.modelItem);

		if (item && item->data)
		{
			if (!item->data->numericalData.contains(tool))
			{
				item->data->numericalData[tool] = DataTable<qreal>();
			}
			NetworkWindow * win = currentWindow();
			if (win)
			{
				win->changeData(tool + tr(" changed for ") + item->fullName(),item,tool,&dat);
			}
		}
		if (sem)
			sem->release();
	}

	void MainWindow::getStringDataRow(QSemaphore* sem,QStringList* list,ItemHandle* item,const QString& tool, const QString& row)
	{
		if (!item && currentWindow())
			item = &(currentWindow()->symbolsTable.modelItem);

		if (list && item && item->data && item->data->textData.contains(tool))
		{
			DataTable<QString>& dat = item->data->textData[tool];
			if (dat.rowNames().contains(row))
			{
				int j = dat.rowNames().indexOf(row);
				for (int i=0; i < dat.cols(); ++i)
				{
					(*list) << dat.at(j,i);
				}
			}
		}
		if (sem)
			sem->release();
	}

	void MainWindow::getStringDataCol(QSemaphore* sem,QStringList* list,ItemHandle* item,const QString& tool, const QString& col)
	{
		if (!item && currentWindow())
			item = &(currentWindow()->symbolsTable.modelItem);

		if (list && item && item->data && item->data->textData.contains(tool))
		{
			DataTable<QString>& dat = item->data->textData[tool];
			if (dat.colNames().contains(col))
			{
				int j = dat.colNames().indexOf(col);
				for (int i=0; i < dat.rows(); ++i)
				{
					(*list) << dat.at(i,j);
				}
			}
		}
		if (sem)
			sem->release();
	}

	void MainWindow::setNumericalData(QSemaphore* sem,ItemHandle* item,const QString& tool, const QString& row, const QString& col,qreal value)
	{
		if (!item && currentWindow())
			item = &(currentWindow()->symbolsTable.modelItem);

		if (item && item->data)
		{
			if (item->data->numericalData.contains(tool))
			{
				DataTable<qreal> dat = item->data->numericalData[tool];
				if (row.isEmpty() && col.isEmpty())
					dat.value(0,0) = value;
				else
				{
					if (row.isEmpty())
						dat.value(0,col) = value;
					else
						if (col.isEmpty())
							dat.value(row,0) = value;
						else
							dat.value(row,col) = value;
				}
				NetworkWindow * win = currentWindow();
				if (win)
				{
					win->changeData(tool + tr(" changed for ") + item->fullName(),item,tool,&dat);
				}
			}
			else
			{
				item->data->numericalData[tool] = DataTable<qreal>();

				DataTable<qreal> dat;
				dat.resize(1,1);
				dat.rowName(0) = row;
				dat.colName(0) = col;
				dat.value(0,0) = value;

				NetworkWindow * win = currentWindow();
				if (win)
				{
					win->changeData(tool + tr(" changed for ") + item->fullName(),item,tool,&dat);
				}
			}
		}
		if (sem)
			sem->release();
	}

	void MainWindow::setStringData(QSemaphore* sem,ItemHandle* item,const QString& tool, const QString& row, const QString& col,const QString& value)
	{
		if (!item && currentWindow())
			item = &(currentWindow()->symbolsTable.modelItem);

		if (item && item->data)
		{
			if (item->data->textData.contains(tool))
			{
				DataTable<QString> dat = item->data->textData[tool];
				if (row.isEmpty() && col.isEmpty())
					dat.value(0,0) = value;
				else
				{
					if (row.isEmpty())
						dat.value(0,col) = value;
					else
						if (col.isEmpty())
							dat.value(row,0) = value;
						else
							dat.value(row,col) = value;
				}
				NetworkWindow * win = currentWindow();
				if (win)
				{
					win->changeData(tool + tr(" changed for ") + item->fullName(),item,tool,&dat);
				}
			}
			else
			{
				item->data->textData[tool] = DataTable<QString>();

				DataTable<QString> dat;
				dat.resize(1,1);
				dat.rowName(0) = row;
				dat.colName(0) = col;
				dat.value(0,0) = value;

				NetworkWindow * win = currentWindow();
				if (win)
				{
					win->changeData(tool + tr(" changed for ") + item->fullName(),item,tool,&dat);
				}
			}
		}
		if (sem)
			sem->release();
	}

	void MainWindow::getNumericalDataNames(QSemaphore* sem,QStringList* list, ItemHandle* item)
	{
		if (!item && currentWindow())
			item = &(currentWindow()->symbolsTable.modelItem);

		if (list && item && item->data)
		{
			(*list) << item->data->numericalData.keys();
		}
		if (sem)
			sem->release();
	}

	void MainWindow::getStringDataNames(QSemaphore* sem,QStringList* list, ItemHandle* item)
	{
		if (!item && currentWindow())
			item = &(currentWindow()->symbolsTable.modelItem);

		if (list && item && item->data)
		{
			(*list) << item->data->textData.keys();
		}
		if (sem)
			sem->release();
	}

	void MainWindow::getChildren(QSemaphore* sem,QList<ItemHandle*>* ret,ItemHandle* item)
	{
		if (item && ret)
		{
			(*ret) << item->children;
		}
		if (sem)
			sem->release();
	}

	void MainWindow::getParent(QSemaphore* sem,ItemHandle** ret,ItemHandle* item)
	{
		if (item && ret)
		{
			(*ret) = item->parent;
		}
		if (sem)
			sem->release();
	}

	/*******************************************************************/
	/*******************FUNCTION TO SIGNAL******************************/
	/*******************************************************************/

	MainWindow_FtoS MainWindow::fToS;

	void MainWindow::_zoom(double x)
	{
		fToS.zoom(x);
	}

	OBJ MainWindow::_find(const char* c)
	{
		return fToS.find(c);
	}

	Array MainWindow::_findItems(char** c)
	{
		return fToS.findItems(c);
	}

	void MainWindow::_select(OBJ o)
	{
		return fToS.select(o);
	}

	void MainWindow::_deselect()
	{
		return fToS.deselect();
	}

	Array MainWindow::_allItems()
	{
		return fToS.allItems();
	}

	Array MainWindow::_itemsOfFamily(const char* f)
	{
		return fToS.itemsOfFamily(f);
	}

	Array MainWindow::_itemsOfFamily2(const char* f, Array a)
	{
		return fToS.itemsOfFamily(f,a);
	}

	Array MainWindow::_selectedItems()
	{
		return fToS.selectedItems();
	}

	char* MainWindow::_getName(OBJ o)
	{
		return fToS.getName(o);
	}

	void MainWindow::_setName(OBJ o,const char* c)
	{
		return fToS.setName(o,c);
	}

	char** MainWindow::_getNames(Array a)
	{
		return fToS.getNames(a);
	}

	char* MainWindow::_getFamily(OBJ o)
	{
		return fToS.getFamily(o);
	}

	int MainWindow::_isA(OBJ o,const char* c)
	{
		return fToS.isA(o,c);
	}

	void MainWindow::_removeItem(OBJ o)
	{
		return fToS.removeItem(o);
	}

	void MainWindow::_setPos(OBJ o,double x,double y)
	{
		return fToS.setPos(o,x,y);
	}

	void MainWindow::_setPos2(Array a,Matrix m)
	{
		return fToS.setPos(a,m);
	}

	Matrix MainWindow::_getPos(Array a)
	{
		return fToS.getPos(a);
	}

	double MainWindow::_getY(OBJ o)
	{
		return fToS.getY(o);
	}

	double MainWindow::_getX(OBJ o)
	{
		return fToS.getX(o);
	}

	void MainWindow::_moveSelected(double dx,double dy)
	{
		return fToS.moveSelected(dx,dy);
	}

	void MainWindow::_clearText()
	{
		return fToS.clearText();
	}

	void MainWindow::_outputTable(Matrix m)
	{
		return fToS.outputTable(m);
	}

	void MainWindow::_outputText(const char * c)
	{
		return fToS.outputText(c);
	}

	void MainWindow::_errorReport(const char * c)
	{
		return fToS.errorReport(c);
	}

	void MainWindow::_printFile(const char* c)
	{
		return fToS.printFile(c);
	}

	void  MainWindow::_createInputWindow1(Matrix m,const char* a,const char* b, const char* c)
	{
		return fToS.createInputWindow(m,a,b,c);
	}

	void  MainWindow::_createInputWindow2(Matrix m,const char* a, MatrixInputFunction f)
	{
		return fToS.createInputWindow(m,a,f);
	}

	void  MainWindow::_addInputWindowOptions(const char* a,int i, int j, char ** c)
	{
		return fToS.addInputWindowOptions(a,i,j,c);
	}

	void  MainWindow::_addInputWindowCheckbox(const char* a,int i, int j)
	{
		return fToS.addInputWindowCheckbox(a,i,j);
	}

	void  MainWindow::_openNewWindow(const char* c)
	{
		return fToS.openNewWindow(c);
	}

	int  MainWindow::_isWindows()
	{
		return fToS.isWindows();
	}

	int  MainWindow::_isMac()
	{
		return fToS.isMac();
	}

	int  MainWindow::_isLinux()
	{
		return fToS.isLinux();
	}

	char*  MainWindow::_appDir()
	{
		return fToS.appDir();
	}

	double MainWindow::_getNumericalData(OBJ o,const char* a, const char* b, const char* c)
	{
		return fToS.getNumericalData(o,a,b,c);
	}

	char* MainWindow::_getStringData(OBJ o,const char* a, const char* b, const char* c)
	{
		return fToS.getStringData(o,a,b,c);
	}

	char** MainWindow::_getNumericalDataRows(OBJ o,const char* a)
	{
		return fToS.getNumericalDataRows(o,a);
	}

	char** MainWindow::_getNumericalDataCols(OBJ o,const char* a)
	{
		return fToS.getNumericalDataCols(o,a);
	}

	char** MainWindow::_getStringDataRows(OBJ o,const char* a)
	{
		return fToS.getStringDataRows(o,a);
	}

	char** MainWindow::_getStringDataCols(OBJ o,const char* a)
	{
		return fToS.getStringDataCols(o,a);
	}

	Matrix MainWindow::_getNumericalDataMatrix(OBJ o,const char* a)
	{
		return fToS.getNumericalDataMatrix(o,a);
	}

	void MainWindow::_setNumericalDataMatrix(OBJ o ,const char* a,Matrix m)
	{
		return fToS.setNumericalDataMatrix(o,a,m);
	}

	char** MainWindow::_getStringDataRow(OBJ o,const char* a,const char* b)
	{
		return fToS.getStringDataRow(o,a,b);
	}

	char** MainWindow::_getStringDataCol(OBJ o,const char* a,const char* b)
	{
		return fToS.getStringDataCol(o,a,b);
	}

	void MainWindow::_setNumericalData(OBJ o,const char* a, const char* b, const char* c,double v)
	{
		return fToS.setNumericalData(o,a,b,c,v);
	}

	void MainWindow::_setStringData(OBJ o,const char* a, const char* b, const char* c,const char * v)
	{
		return fToS.setStringData(o,a,b,c,v);
	}

	char** MainWindow::_getStringDataNames(OBJ o)
	{
		return fToS.getStringDataNames(o);
	}

	char** MainWindow::_getNumericalDataNames(OBJ o)
	{
		return fToS.getNumericalDataNames(o);
	}

	Array MainWindow::_getChildren(OBJ o)
	{
		return fToS.getChildren(o);
	}

	OBJ MainWindow::_getParent(OBJ o)
	{
		return fToS.getParent(o);
	}

	void MainWindow_FtoS::zoom(double x)
	{
		emit zoom(0,x);
	}

	OBJ MainWindow_FtoS::find(const char* c)
	{
		QSemaphore * s = new QSemaphore(1);
		ItemHandle * p;
		s->acquire();
		emit find(s,&p,ConvertValue(c));
		s->acquire();
		s->release();
		delete s;
		return p;
	}

	Array MainWindow_FtoS::findItems(char** c)
	{
		QSemaphore * s = new QSemaphore(1);
		QList<ItemHandle*>* p = new QList<ItemHandle*>;
		s->acquire();
		emit findItems(s,p,ConvertValue(c));
		s->acquire();
		s->release();
		delete s;
		Array A = ConvertValue(*p);
		delete p;
		return A;
	}

	void MainWindow_FtoS::select(OBJ o)
	{
		QSemaphore * s = new QSemaphore(1);
		s->acquire();
		emit select(s,ConvertValue(o));
		s->acquire();
		s->release();
		delete s;
	}

	void MainWindow_FtoS::deselect()
	{
		QSemaphore * s = new QSemaphore(1);
		s->acquire();
		emit deselect(s);
		s->acquire();
		s->release();
		delete s;
	}

	Array MainWindow_FtoS::allItems()
	{
		QSemaphore * s = new QSemaphore(1);
		QList<ItemHandle*>* p = new QList<ItemHandle*>;
		s->acquire();
		emit allItems(s,p);
		s->acquire();
		s->release();
		delete s;
		Array A = ConvertValue(*p);
		delete p;
		return A;
	}

	Array MainWindow_FtoS::itemsOfFamily(const char * f)
	{
		QSemaphore * s = new QSemaphore(1);
		QList<ItemHandle*>* p = new QList<ItemHandle*>;
		s->acquire();
		emit itemsOfFamily(s,p,ConvertValue(f));
		s->acquire();
		s->release();
		delete s;
		Array A = ConvertValue(*p);
		delete p;
		return A;
	}

	Array MainWindow_FtoS::itemsOfFamily(const char * f, Array a)
	{
		QSemaphore * s = new QSemaphore(1);
		QList<ItemHandle*> * list = ConvertValue(a);
		QList<ItemHandle*>* p = new QList<ItemHandle*>;
		s->acquire();
		emit itemsOfFamily(s,p,*list,ConvertValue(f));
		s->acquire();
		s->release();
		delete s;
		Array A = ConvertValue(*p);
		delete p;
		delete list;
		return A;
	}

	Array MainWindow_FtoS::selectedItems()
	{
		QSemaphore * s = new QSemaphore(1);
		QList<ItemHandle*>* p = new QList<ItemHandle*>;
		s->acquire();
		emit selectedItems(s,p);
		s->acquire();
		s->release();
		delete s;
		Array A = ConvertValue(*p);
		delete p;
		return A;
	}

	char* MainWindow_FtoS::getName(OBJ o)
	{
		QSemaphore * s = new QSemaphore(1);
		QString p;
		s->acquire();
		emit getName(s,&p,ConvertValue(o));
		s->acquire();
		s->release();
		delete s;
		return ConvertValue(p);
	}

	void MainWindow_FtoS::setName(OBJ o, const char* c)
	{
		QSemaphore * s = new QSemaphore(1);
		s->acquire();
		emit setName(s,ConvertValue(o),ConvertValue(c));
		s->acquire();
		s->release();
		delete s;
	}

	char** MainWindow_FtoS::getNames(Array a0)
	{
		QSemaphore * s = new QSemaphore(1);
		QStringList p;
		s->acquire();
		QList<ItemHandle*> * list = ConvertValue(a0);
		emit getNames(s,&p,*list);
		s->acquire();
		s->release();
		delete s;
		delete list;
		return ConvertValue(p);
	}

	char* MainWindow_FtoS::getFamily(OBJ a0)
	{
		QSemaphore * s = new QSemaphore(1);
		QString p;
		s->acquire();
		emit getFamily(s,&p,ConvertValue(a0));
		s->acquire();
		s->release();
		delete s;
		return ConvertValue(p);
	}

	int MainWindow_FtoS::isA(OBJ a0, const char* name)
	{
		QSemaphore * s = new QSemaphore(1);
		int p = 0;
		s->acquire();
		emit isA(s,&p,ConvertValue(a0),ConvertValue(name));
		s->acquire();
		s->release();
		delete s;
		return p;
	}

	void MainWindow_FtoS::removeItem(OBJ a0)
	{
		QSemaphore * s = new QSemaphore(1);
		s->acquire();
		emit removeItem(s,ConvertValue(a0));
		s->acquire();
		s->release();
		delete s;
	}

	void MainWindow_FtoS::setPos(OBJ a0,double a1,double a2)
	{
		QSemaphore * s = new QSemaphore(1);
		s->acquire();
		emit setPos(s,ConvertValue(a0),a1,a2);
		s->acquire();
		s->release();
		delete s;
	}

	void MainWindow_FtoS::setPos(Array a0,Matrix m)
	{
		QSemaphore * s = new QSemaphore(1);
		s->acquire();
		QList<ItemHandle*> * list = ConvertValue(a0);
		DataTable<qreal> * dat = ConvertValue(m);
		emit setPos(s,*list,*dat);
		s->acquire();
		s->release();
		delete dat;
		delete list;
		delete s;
	}

	Matrix MainWindow_FtoS::getPos(Array a0)
	{
		QSemaphore * s = new QSemaphore(1);
		s->acquire();
		QList<ItemHandle*> * list = ConvertValue(a0);
		DataTable<qreal> * p = new DataTable<qreal>;
		emit getPos(s,*list,p);
		s->acquire();
		s->release();
		delete list;
		delete s;
		if (p)
		{
			Matrix m = ConvertValue(*p);
			delete p;
			return m;
		}
		return emptyMatrix();
	}


	double MainWindow_FtoS::getY(OBJ a0)
	{
		QSemaphore * s = new QSemaphore(1);
		qreal p;
		s->acquire();
		emit getY(s,&p,ConvertValue(a0));
		s->acquire();
		s->release();
		delete s;
		return (double)p;
	}

	double MainWindow_FtoS::getX(OBJ a0)
	{
		QSemaphore * s = new QSemaphore(1);
		qreal p;
		s->acquire();
		emit getX(s,&p,ConvertValue(a0));
		s->acquire();
		s->release();
		delete s;
		return (double)p;
	}

	void MainWindow_FtoS::moveSelected(double a0,double a1)
	{
		QSemaphore * s = new QSemaphore(1);
		s->acquire();
		emit moveSelected(s,a0,a1);
		s->acquire();
		s->release();
		delete s;
	}

	void MainWindow_FtoS::clearText()
	{
		QSemaphore * s = new QSemaphore(1);
		s->acquire();
		emit clearText(s);
		s->acquire();
		s->release();
		delete s;
	}

	void MainWindow_FtoS::outputTable(Matrix m)
	{
		QSemaphore * s = new QSemaphore(1);
		s->acquire();
		DataTable<qreal> * dat = ConvertValue(m);
		emit outputTable(s,*dat);
		s->acquire();
		s->release();
		delete s;
		delete dat;
	}

	void MainWindow_FtoS::outputText(const char * c)
	{
		QSemaphore * s = new QSemaphore(1);
		s->acquire();
		emit outputText(s,ConvertValue(c));
		s->acquire();
		s->release();
		delete s;
	}

	void MainWindow_FtoS::errorReport(const char * c)
	{
		QSemaphore * s = new QSemaphore(1);
		s->acquire();
		emit errorReport(s,ConvertValue(c));
		s->acquire();
		s->release();
		delete s;
	}

	void MainWindow_FtoS::printFile(const char * c)
	{
		QSemaphore * s = new QSemaphore(1);
		s->acquire();
		emit printFile(s,ConvertValue(c));
		s->acquire();
		s->release();
		delete s;
	}

	void MainWindow_FtoS::createInputWindow(Matrix m, const char* cfile,const char* fname, const char* title)
	{
		DataTable<qreal>* dat = ConvertValue(m);
		QSemaphore * s = new QSemaphore(1);
		s->acquire();
		emit createInputWindow(s,*dat,ConvertValue(cfile),ConvertValue(fname),ConvertValue(title));
		s->acquire();
		s->release();
		delete s;
		delete dat;
	}

	void MainWindow_FtoS::createInputWindow(Matrix m, const char* title, MatrixInputFunction f)
	{
		DataTable<qreal>* dat = ConvertValue(m);
		QSemaphore * s = new QSemaphore(1);
		s->acquire();
		emit createInputWindow(s,*dat,ConvertValue(title),f);
		s->acquire();
		s->release();
		delete s;
		delete dat;
	}

	void MainWindow_FtoS::addInputWindowOptions(const char * a, int i, int j, char** list)
	{
		QSemaphore * s = new QSemaphore(1);
		s->acquire();
		emit addInputWindowOptions(s,ConvertValue(a),i,j,ConvertValue(list));
		s->acquire();
		s->release();
		delete s;
	}

	void MainWindow_FtoS::addInputWindowCheckbox(const char * a, int i, int j)
	{
		QSemaphore * s = new QSemaphore(1);
		s->acquire();
		emit addInputWindowCheckbox(s,ConvertValue(a),i,j);
		s->acquire();
		s->release();
		delete s;
	}

	void MainWindow_FtoS::openNewWindow(const char* c)
	{
		QSemaphore * s = new QSemaphore(1);
		s->acquire();
		emit openNewWindow(s,ConvertValue(c));
		s->acquire();
		s->release();
		delete s;
	}

	int MainWindow_FtoS::isWindows()
	{
		int i;
		QSemaphore * s = new QSemaphore(1);
		s->acquire();
		emit isWindows(s,&i);
		s->acquire();
		s->release();
		delete s;
		return i;
	}

	int MainWindow_FtoS::isMac()
	{
		int i;
		QSemaphore * s = new QSemaphore(1);
		s->acquire();
		emit isMac(s,&i);
		s->acquire();
		s->release();
		delete s;
		return i;
	}

	int MainWindow_FtoS::isLinux()
	{
		int i;
		QSemaphore * s = new QSemaphore(1);
		s->acquire();
		emit isLinux(s,&i);
		s->acquire();
		s->release();
		delete s;
		return i;
	}

	char* MainWindow_FtoS::appDir()
	{
		QString dir;
		QSemaphore * s = new QSemaphore(1);
		s->acquire();
		emit appDir(s,&dir);
		s->acquire();
		s->release();
		delete s;
		return ConvertValue(dir);
	}

	double MainWindow_FtoS::getNumericalData(OBJ o,const char* a, const char* b, const char* c)
	{
		qreal d=0.0;
		QSemaphore * s = new QSemaphore(1);
		s->acquire();
		emit getNumericalData(s,&d,ConvertValue(o),ConvertValue(a),ConvertValue(b),ConvertValue(c));
		s->acquire();
		s->release();
		delete s;
		return (double)d;
	}

	char* MainWindow_FtoS::getStringData(OBJ o,const char* a, const char* b, const char* c)
	{
		QString str;
		QSemaphore * s = new QSemaphore(1);
		s->acquire();
		emit getStringData(s,&str,ConvertValue(o),ConvertValue(a),ConvertValue(b),ConvertValue(c));
		s->acquire();
		s->release();
		delete s;
		return ConvertValue(str);
	}

	void MainWindow_FtoS::setNumericalData(OBJ o,const char* a, const char* b, const char* c,double v)
	{
		QSemaphore * s = new QSemaphore(1);
		s->acquire();
		emit setNumericalData(s,ConvertValue(o),ConvertValue(a),ConvertValue(b),ConvertValue(c),(qreal)v);
		s->acquire();
		s->release();
		delete s;
	}

	char** MainWindow_FtoS::getNumericalDataRows(OBJ o,const char* c)
	{
		QSemaphore * s = new QSemaphore(1);
		QStringList p;
		s->acquire();
		emit getNumericalDataRows(s,&p,ConvertValue(o),ConvertValue(c));
		s->acquire();
		s->release();
		delete s;
		return ConvertValue(p);
	}

	char** MainWindow_FtoS::getNumericalDataCols(OBJ o,const char* c)
	{
		QSemaphore * s = new QSemaphore(1);
		QStringList p;
		s->acquire();
		emit getNumericalDataCols(s,&p,ConvertValue(o),ConvertValue(c));
		s->acquire();
		s->release();
		delete s;
		return ConvertValue(p);
	}
	char** MainWindow_FtoS::getStringDataRows(OBJ o,const char* c)
	{
		QSemaphore * s = new QSemaphore(1);
		QStringList p;
		s->acquire();
		emit getStringDataRows(s,&p,ConvertValue(o),ConvertValue(c));
		s->acquire();
		s->release();
		delete s;
		return ConvertValue(p);
	}
	char** MainWindow_FtoS::getStringDataCols(OBJ o,const char* c)
	{
		QSemaphore * s = new QSemaphore(1);
		QStringList p;
		s->acquire();
		emit getStringDataCols(s,&p,ConvertValue(o),ConvertValue(c));
		s->acquire();
		s->release();
		delete s;
		return ConvertValue(p);
	}

	Matrix MainWindow_FtoS::getNumericalDataMatrix(OBJ o,const char* c)
	{
		QSemaphore * s = new QSemaphore(1);
		DataTable<qreal> * p = new DataTable<qreal>;
		s->acquire();
		emit getNumericalDataMatrix(s,p,ConvertValue(o),ConvertValue(c));
		s->acquire();
		s->release();
		delete s;
		if (p)
		{
			Matrix m = ConvertValue(*p);
			delete p;
			return m;
		}
		return emptyMatrix();
	}

	void MainWindow_FtoS::setNumericalDataMatrix(OBJ o, const char * c, Matrix M)
	{
		DataTable<qreal>* dat = ConvertValue(M);
		QSemaphore * s = new QSemaphore(1);
		s->acquire();
		emit setNumericalDataMatrix(s,ConvertValue(o),ConvertValue(c),*dat);
		s->acquire();
		s->release();
		delete s;
		delete dat;
	}

	char** MainWindow_FtoS::getStringDataRow(OBJ o ,const char* c,const char* v)
	{
		QSemaphore * s = new QSemaphore(1);
		QStringList p;
		s->acquire();
		emit getStringDataRow(s,&p,ConvertValue(o),ConvertValue(c),ConvertValue(v));
		s->acquire();
		s->release();
		delete s;
		return ConvertValue(p);
	}

	char** MainWindow_FtoS::getStringDataCol(OBJ o ,const char* c,const char* v)
	{
		QSemaphore * s = new QSemaphore(1);
		QStringList p;
		s->acquire();
		emit getStringDataCol(s,&p,ConvertValue(o),ConvertValue(c),ConvertValue(v));
		s->acquire();
		s->release();
		delete s;
		return ConvertValue(p);
	}

	void MainWindow_FtoS::setStringData(OBJ o,const char* a, const char* b, const char* c,const char * v)
	{
		QSemaphore * s = new QSemaphore(1);
		s->acquire();
		emit setStringData(s,ConvertValue(o),ConvertValue(a),ConvertValue(b),ConvertValue(c),ConvertValue(v));
		s->acquire();
		s->release();
		delete s;
	}

	char** MainWindow_FtoS::getNumericalDataNames(OBJ o)
	{
		QStringList p;
		QSemaphore * s = new QSemaphore(1);
		s->acquire();
		emit getNumericalDataNames(s,&p,ConvertValue(o));
		s->acquire();
		s->release();
		delete s;
		return ConvertValue(p);
	}

	char** MainWindow_FtoS::getStringDataNames(OBJ o)
	{
		QStringList p;
		QSemaphore * s = new QSemaphore(1);
		s->acquire();
		emit getStringDataNames(s,&p,ConvertValue(o));
		s->acquire();
		s->release();
		delete s;
		return ConvertValue(p);
	}

	Array MainWindow_FtoS::getChildren(OBJ o)
	{
		QSemaphore * s = new QSemaphore(1);
		QList<ItemHandle*>* p = new QList<ItemHandle*>;
		s->acquire();
		emit getChildren(s,p,ConvertValue(o));
		s->acquire();
		s->release();
		delete s;
		Array A = ConvertValue(*p);
		delete p;
		return A;
	}

	OBJ MainWindow_FtoS::getParent(OBJ o)
	{
		ItemHandle * p = 0;
		QSemaphore * s = new QSemaphore(1);
		s->acquire();
		emit getParent(s,&p,ConvertValue(o));
		s->acquire();
		s->release();
		delete s;
		return ConvertValue(p);
	}

	void MainWindow::_deleteArray(Array /*A*/)
	{
	}

	void MainWindow::_deleteMatrix(Matrix /*M*/)
	{
		/*if (M.colnames)
		{
		//for (int i=0; M.colnames[i] != 0; ++i)
		//	delete M.colnames[i];
		//delete M.colnames;
		}
		if (M.rownames)
		{
		//for (int i=0; M.rownames[i] != 0; ++i)
		//	delete M.rownames[i];
		//delete M.rownames;
		}
		if (M.values) delete M.values;*/
	}

	void MainWindow::_deleteStrings(char** /*str*/)
	{
		/*if (str);
		{
		//for (int i=0; str[i] != 0; ++i)
		//	delete str[i];
		//delete str;
		}*/
	}

	/*******************************************/
	/*******************************************/
	/*******************************************/

	void MainWindow::connectTCFunctions()
	{
		connect(&fToS,SIGNAL(find(QSemaphore*,ItemHandle**,const QString&)),this,SLOT(findItem(QSemaphore*,ItemHandle**,const QString&)));
		connect(&fToS,SIGNAL(findItems(QSemaphore*,QList<ItemHandle*>*,const QStringList&)),
				this,SLOT(findItems(QSemaphore*,QList<ItemHandle*>*,const QStringList&)));

		connect(&fToS,SIGNAL(select(QSemaphore*,ItemHandle*)),this,SLOT(select(QSemaphore*,ItemHandle*)));
		connect(&fToS,SIGNAL(deselect(QSemaphore*)),this,SLOT(deselect(QSemaphore*)));
		connect(&fToS,SIGNAL(allItems(QSemaphore*,QList<ItemHandle*>*)),this,SLOT(allItems(QSemaphore*,QList<ItemHandle*>*)));
		connect(&fToS,SIGNAL(selectedItems(QSemaphore*,QList<ItemHandle*>*)),this,SLOT(selectedItems(QSemaphore*,QList<ItemHandle*>*)));
		connect(&fToS,SIGNAL(itemsOfFamily(QSemaphore*,QList<ItemHandle*>*,const QString&)),this,SLOT(itemsOfFamily(QSemaphore*,QList<ItemHandle*>*,const QString&)));
		connect(&fToS,SIGNAL(itemsOfFamily(QSemaphore*,QList<ItemHandle*>*,const QList<ItemHandle*>&, const QString&)),this,SLOT(itemsOfFamily(QSemaphore*,QList<ItemHandle*>*,const QList<ItemHandle*>&, const QString&)));
		connect(&fToS,SIGNAL(getX(QSemaphore*,qreal*,ItemHandle*)),this,SLOT(getX(QSemaphore*,qreal*,ItemHandle*)));
		connect(&fToS,SIGNAL(getY(QSemaphore*,qreal*,ItemHandle*)),this,SLOT(getY(QSemaphore*,qreal*,ItemHandle*)));

		connect(&fToS,SIGNAL(setPos(QSemaphore*,ItemHandle*,qreal,qreal)),this,SLOT(setPos(QSemaphore*,ItemHandle*,qreal,qreal)));
		connect(&fToS,SIGNAL(setPos(QSemaphore*,const QList<ItemHandle*>&,DataTable<qreal>&)),this,SLOT(setPos(QSemaphore*,const QList<ItemHandle*>&,DataTable<qreal>&)));
		connect(&fToS,SIGNAL(getPos(QSemaphore*,const QList<ItemHandle*>&,DataTable<qreal>*)),this,SLOT(getPos(QSemaphore*,const QList<ItemHandle*>&,DataTable<qreal>*)));

		connect(&fToS,SIGNAL(removeItem(QSemaphore*,ItemHandle*)),this,SLOT(removeItem(QSemaphore*,ItemHandle*)));
		connect(&fToS,SIGNAL(moveSelected(QSemaphore*,qreal,qreal)),this,SLOT(moveSelected(QSemaphore*,qreal,qreal)));
		connect(&fToS,SIGNAL(getName(QSemaphore*,QString*,ItemHandle*)),this,SLOT(itemName(QSemaphore*,QString*,ItemHandle*)));
		connect(&fToS,SIGNAL(setName(QSemaphore*,ItemHandle*,const QString&)),this,SLOT(setName(QSemaphore*,ItemHandle*,const QString&)));

		connect(&fToS,SIGNAL(getNames(QSemaphore*,QStringList*,const QList<ItemHandle*>&)),this,SLOT(itemNames(QSemaphore*,QStringList*,const QList<ItemHandle*>&)));
		connect(&fToS,SIGNAL(getFamily(QSemaphore*,QString*,ItemHandle*)),this,SLOT(itemFamily(QSemaphore*,QString*,ItemHandle*)));
		connect(&fToS,SIGNAL(isA(QSemaphore*,int*,ItemHandle*,const QString&)),this,SLOT(isA(QSemaphore*,int*,ItemHandle*,const QString&)));
		connect(&fToS,SIGNAL(clearText(QSemaphore*)),this,SLOT(clearText(QSemaphore*)));
		connect(&fToS,SIGNAL(outputText(QSemaphore*,const QString&)),this,SLOT(outputText(QSemaphore*,const QString&)));
		connect(&fToS,SIGNAL(errorReport(QSemaphore*,const QString&)),this,SLOT(errorReport(QSemaphore*,const QString&)));
		connect(&fToS,SIGNAL(printFile(QSemaphore*,const QString&)),this,SLOT(printFile(QSemaphore*,const QString&)));
		connect(&fToS,SIGNAL(outputTable(QSemaphore*,const DataTable<qreal>&)),this,SLOT(outputTable(QSemaphore*,const DataTable<qreal>&)));

		connect(&fToS,SIGNAL(createInputWindow(QSemaphore*,const DataTable<qreal>&,const QString&,const QString&,const QString&)),
			this,SLOT(createInputWindow(QSemaphore*,const DataTable<qreal>&,const QString&,const QString&,const QString&)));

		connect(&fToS,SIGNAL(createInputWindow(QSemaphore*,const DataTable<qreal>&,const QString&,MatrixInputFunction)),
			this,SLOT(createInputWindow(QSemaphore*,const DataTable<qreal>&,const QString&,MatrixInputFunction)));

		connect(&fToS,SIGNAL(addInputWindowOptions(QSemaphore*,const QString&, int, int, const QStringList&)),
			this,SLOT(addInputWindowOptions(QSemaphore*,const QString&, int, int, const QStringList&)));

		connect(&fToS,SIGNAL(addInputWindowCheckbox(QSemaphore*,const QString&, int, int)),
			this,SLOT(addInputWindowCheckbox(QSemaphore*,const QString&, int, int)));

		connect(&fToS,SIGNAL(openNewWindow(QSemaphore*,const QString&)),this,SLOT(openNewWindow(QSemaphore*,const QString&)));
		connect(&fToS,SIGNAL(isWindows(QSemaphore*,int*)),this,SLOT(isWindows(QSemaphore*,int*)));
		connect(&fToS,SIGNAL(isMac(QSemaphore*,int*)),this,SLOT(isMac(QSemaphore*,int*)));
		connect(&fToS,SIGNAL(isLinux(QSemaphore*,int*)),this,SLOT(isLinux(QSemaphore*,int*)));
		connect(&fToS,SIGNAL(appDir(QSemaphore*,QString*)),this,SLOT(appDir(QSemaphore*,QString*)));
		connect(&fToS,SIGNAL(getChildren(QSemaphore*,QList<ItemHandle*>*,ItemHandle*)),this,SLOT(getChildren(QSemaphore*,QList<ItemHandle*>*,ItemHandle*)));
		connect(&fToS,SIGNAL(getParent(QSemaphore*,ItemHandle**,ItemHandle*)),this,SLOT(getParent(QSemaphore*,ItemHandle**,ItemHandle*)));
		connect(&fToS,SIGNAL(getNumericalData(QSemaphore*,qreal*,ItemHandle*,const QString&, const QString&, const QString&)),this,SLOT(getNumericalData(QSemaphore*,qreal*,ItemHandle*,const QString&, const QString&, const QString&)));
		connect(&fToS,SIGNAL(getStringData(QSemaphore*,QString*,ItemHandle*,const QString&, const QString&, const QString&)),this,SLOT(getStringData(QSemaphore*,QString*,ItemHandle*,const QString&, const QString&, const QString&)));
		connect(&fToS,SIGNAL(setNumericalData(QSemaphore*,ItemHandle*,const QString&, const QString&, const QString&,qreal)),this,SLOT(setNumericalData(QSemaphore*,ItemHandle*,const QString&, const QString&, const QString&,qreal)));
		connect(&fToS,SIGNAL(setStringData(QSemaphore*,ItemHandle*,const QString&, const QString&, const QString&,const QString&)),this,SLOT(setStringData(QSemaphore*,ItemHandle*,const QString&, const QString&, const QString&,const QString&)));

		connect(&fToS,SIGNAL(getNumericalDataRows(QSemaphore*,QStringList*,ItemHandle*,const QString&)),this,SLOT(getNumericalDataRows(QSemaphore*,QStringList*,ItemHandle*,const QString&)));
		connect(&fToS,SIGNAL(getNumericalDataCols(QSemaphore*,QStringList*,ItemHandle*,const QString&)),this,SLOT(getNumericalDataCols(QSemaphore*,QStringList*,ItemHandle*,const QString&)));
		connect(&fToS,SIGNAL(getStringDataRows(QSemaphore*,QStringList*,ItemHandle*,const QString&)),this,SLOT(getStringDataRows(QSemaphore*,QStringList*,ItemHandle*,const QString&)));
		connect(&fToS,SIGNAL(getStringDataCols(QSemaphore*,QStringList*,ItemHandle*,const QString&)),this,SLOT(getStringDataCols(QSemaphore*,QStringList*,ItemHandle*,const QString&)));

		connect(&fToS,SIGNAL(getNumericalDataMatrix(QSemaphore*,DataTable<qreal>*,ItemHandle*,const QString&)),this,SLOT(getNumericalDataMatrix(QSemaphore*,DataTable<qreal>*,ItemHandle*,const QString&)));
		connect(&fToS,SIGNAL(setNumericalDataMatrix(QSemaphore*,ItemHandle*,const QString&,const DataTable<qreal>&)),this,SLOT(setNumericalDataMatrix(QSemaphore*,ItemHandle*,const QString&,const DataTable<qreal>&)));

		connect(&fToS,SIGNAL(getStringDataRow(QSemaphore*,QStringList*,ItemHandle*,const QString&,const QString&)),this,SLOT(getStringDataRow(QSemaphore*,QStringList*,ItemHandle*,const QString&,const QString&)));
		connect(&fToS,SIGNAL(getStringDataCol(QSemaphore*,QStringList*,ItemHandle*,const QString&,const QString&)),this,SLOT(getStringDataCol(QSemaphore*,QStringList*,ItemHandle*,const QString&,const QString&)));

		connect(&fToS,SIGNAL(getStringDataNames(QSemaphore*,QStringList*,ItemHandle*)),this,SLOT(getStringDataNames(QSemaphore*,QStringList*,ItemHandle*)));
		connect(&fToS,SIGNAL(getNumericalDataNames(QSemaphore*,QStringList*,ItemHandle*)),this,SLOT(getNumericalDataNames(QSemaphore*,QStringList*,ItemHandle*)));

		connect(&fToS,SIGNAL(zoom(QSemaphore*,qreal)),this,SLOT(zoom(QSemaphore*,qreal)));

	}

	typedef void (*main_api_func)(
		Array (*tc_allItems0)(),
		Array (*tc_selectedItems0)(),
		Array (*tc_itemsOfFamily0)(const char*),
		Array (*tc_itemsOfFamily1)(const char*,Array),
		OBJ (*tc_find0)(const char*),
		Array (*tc_finds0)(char**),
		void (*tc_select0)(OBJ),
		void (*tc_deselect0)(),
		char* (*tc_getName0)(OBJ),
		void (*tc_setName0)(OBJ,const char*),

		char** (*tc_getNames0)(Array),
		char* (*tc_getFamily0)(OBJ),
		int (*tc_isA0)(OBJ,const char*),

		void (*tc_clearText0)(),
		void (*tc_outputText0)(const char*),
		void (*tc_errorReport0)(const char*),
		void (*tc_outputTable0)(Matrix),
		void (*tc_printFile0)(const char*),

		void (*tc_removeItem0)(OBJ),

		double (*tc_getY0)(OBJ),
		double (*tc_getX0)(OBJ),
		Matrix (*tc_getPos0)(Array),
		void (*tc_setPos0)(OBJ,double,double),
		void (*tc_setPos1)(Array,Matrix),
		void (*tc_moveSelected0)(double,double),

		int (*tc_isWindows0)(),
		int (*tc_isMac0)(),
		int (*tc_isLinux0)(),

		char* (*tc_appDir0)(),

		void (*tc_createInputWindow0)(Matrix, const char*, const char*,const char*),
		void (*tc_createInputWindow1)(Matrix, const char*, void (*f)(Matrix)),
		void (*tc_addInputWindowOptions)(const char*, int i, int j, char **),
		void (*tc_addInputWindowCheckbox)(const char*, int i, int j),
		void (*tc_openNewWindow)(const char*),

		double (*tc_getNumericalData)(OBJ,const char*, const char*, const char*),
		char* (*tc_getStringData)(OBJ,const char*, const char*, const char*),
		void (*tc_setNumericalData)(OBJ,const char*, const char*, const char*,double),
		void (*tc_setStringData)(OBJ,const char*, const char*, const char*,const char*),
		Array (*tc_getChildren)(OBJ),
		OBJ (*tc_getParent)(OBJ),

		char** (*tc_getNumericalDataRows)(OBJ,const char*),
		char** (*tc_getNumericalDataCols)(OBJ,const char*),
		char** (*tc_getStringDataRows)(OBJ,const char*),
		char** (*tc_getStringDataCols)(OBJ,const char*),
		Matrix (*tc_getNumericalDataMatrix)(OBJ,const char*),
		void (*tc_setNumericalDataMatrix)(OBJ,const char*,Matrix),
		char** (*tc_getStringDataRow)(OBJ,const char*,const char*),
		char** (*tc_getStringDataCol)(OBJ,const char*,const char*),

		char** (*tc_getNumericalDataNames)(OBJ),
		char** (*tc_getStringDataNames)(OBJ),

		void (*tc_zoom)(double)
		);

	void MainWindow::setupFunctionPointersSlot(QSemaphore* s,QLibrary * library)
	{
		main_api_func f = (main_api_func)library->resolve("tc_Main_api_initialize");
		if (f)
		{
			f(
				&(_allItems),
				&(_selectedItems),
				&(_itemsOfFamily),
				&(_itemsOfFamily2),
				&(_find),
				&(_findItems),
				&(_select),
				&(_deselect),
				&(_getName),
				&(_setName),
				&(_getNames),
				&(_getFamily),
				&(_isA),
				&(_clearText),
				&(_outputText),
				&(_errorReport),
				&(_outputTable),
				&(_printFile),
				&(_removeItem),
				&(_getY),
				&(_getX),
				&(_getPos),
				&(_setPos),
				&(_setPos2),
				&(_moveSelected),
				&(_isWindows),
				&(_isMac),
				&(_isLinux),
				&(_appDir),
				&(_createInputWindow1),
				&(_createInputWindow2),
				&(_addInputWindowOptions),
				&(_addInputWindowCheckbox),
				&(_openNewWindow),
				&(_getNumericalData),
				&(_getStringData),
				&(_setNumericalData),
				&(_setStringData),
				&(_getChildren),
				&(_getParent),
				&(_getNumericalDataRows),
				&(_getNumericalDataCols),
				&(_getStringDataRows),
				&(_getStringDataCols),
				&(_getNumericalDataMatrix),
				&(_setNumericalDataMatrix),
				&(_getStringDataRow),
				&(_getStringDataCol),
				&(_getNumericalDataNames),
				&(_getStringDataNames),
				&(_zoom)
				);
		}

		emit setupFunctionPointers(library); //let all the other plug-ins setup their functions

		if (s)
			s->release();
	}

	void MainWindow::RegisterDataTypes()
	{
		//register new signal/slot data types
		qRegisterMetaType< QList<QGraphicsItem*> >("QList<QGraphicsItem*>");
		qRegisterMetaType< QStringList >("QStringList");

		qRegisterMetaType< QList<QGraphicsItem*> >("QList<QGraphicsItem*>&");
		qRegisterMetaType< QStringList >("QStringList&");

		qRegisterMetaType< QList<QGraphicsItem*>* >("QList<QGraphicsItem*>*");
		qRegisterMetaType< QStringList* >("QStringList*");

		qRegisterMetaType< DataTable<qreal> >("DataTable<qreal>");
		qRegisterMetaType< DataTable<QString> >("DataTable<qreal>");

		qRegisterMetaType< DataTable<qreal> >("DataTable<qreal>&");
		qRegisterMetaType< DataTable<QString> >("DataTable<qreal>&");

		qRegisterMetaType< DataTable<qreal>* >("DataTable<qreal>*");
		qRegisterMetaType< DataTable<QString>* >("DataTable<qreal>*");

		qRegisterMetaType< ItemHandle* >("ItemHandle*");
		qRegisterMetaType< QList<ItemHandle*> >("QList<ItemHandle*>");
		qRegisterMetaType< QList<ItemHandle*> >("QList<ItemHandle*>&");

		qRegisterMetaType< Tool* >("Tool*");
		qRegisterMetaType< QList<Tool*> >("QList<Tool*>");
		qRegisterMetaType< QList<Tool*> >("QList<Tool*>&");

		qRegisterMetaType< QList<QStringList> >("QList<QStringList>");
		qRegisterMetaType< QList<QStringList> >("QList<QStringList>&");

		qRegisterMetaType< MatrixInputFunction >("MatrixInputFunction");

		qRegisterMetaType< Matrix >("Matrix");
	}

	void MainWindow::addParser(TextParser * parser)
	{
		static QActionGroup * actionGroup = 0;

		if (!parser) return;

		if (!parsersMenu)
		{

			parsersMenu = new QMenu(tr("&Parsers"));
			menuBar()->insertMenu(helpMenu->menuAction(),parsersMenu);
		}

		if (!actionGroup)
		{
			actionGroup = new QActionGroup(this);
			actionGroup->setExclusive(true);
		}

		QAction * action = parsersMenu->addAction(QIcon(parser->icon),parser->name);
		connect(action,SIGNAL(triggered()),parser,SLOT(activate()));
		action->setCheckable(true);
		actionGroup->addAction(action);
		action->setChecked(true);

		TextParser::setParser(parser);
	}

	void MainWindow::gridOff()
	{
		GraphicsScene * scene = currentScene();
		if (!scene) return;

		scene->disableGrid();
	}

	void MainWindow::gridOn()
	{
		GraphicsScene * scene = currentScene();
		if (!scene) return;

		if (GraphicsScene::GRID == 0)
			GraphicsScene::GRID = 100;

		scene->enableGrid(GraphicsScene::GRID);
	}

	void MainWindow::setGridSize()
	{
		GraphicsScene * scene = currentScene();
		if (!scene) return;

		bool ok;
		int d = QInputDialog::getInteger (this,tr("Grid size"),tr("Set canvas grid size"),
											GraphicsScene::GRID,0,(int)(currentScene()->sceneRect().width()/10.0),1,&ok);
		if (ok)
		{
			GraphicsScene::GRID = d;
			scene->setGridSize(GraphicsScene::GRID);
		}
	}
	
	void MainWindow::changeConsoleBgColor()
	{
		if (consoleWindow && consoleWindow->editor())
		{
			QColor color = QColorDialog::getColor(ConsoleWindow::BackgroundColor,this);
			consoleWindow->editor()->setBackgroundColor(color);
		}
	}
	
	void MainWindow::changeConsoleTextColor()
	{
		if (consoleWindow && consoleWindow->editor())
		{
			QColor color = QColorDialog::getColor(ConsoleWindow::PlainTextColor,this);
			consoleWindow->editor()->setPlainTextColor(color);
		}
	}
	
	void MainWindow::changeConsoleMsgColor()
	{
		if (consoleWindow && consoleWindow->editor())
		{
			QColor color = QColorDialog::getColor(ConsoleWindow::OutputTextColor,this);
			consoleWindow->editor()->setOutputTextColor(color);
		}
	}
	
	void MainWindow::changeConsoleErrorMsgColor()
	{
		if (consoleWindow && consoleWindow->editor())
		{
			QColor color = QColorDialog::getColor(ConsoleWindow::ErrorTextColor,this);
			consoleWindow->editor()->setErrorTextColor(color);
		}
	}
	
	void MainWindow::popOut()
	{
		popOut(currentWindow());
	}
	
	void MainWindow::popOut(NetworkWindow * win)
	{
		if (allowViewModeToChange && win && tabWidget && tabWidget->count() > 1)
		{
			int i = tabWidget->indexOf(win);
			if (i > -1 && i < tabWidget->count())
			{
				tabWidget->removeTab(i);
				win->setParent(0);
				
				QDockWidget *dock = new QDockWidget(win->windowTitle(), this);
				dock->setAttribute(Qt::WA_DeleteOnClose);
				dock->setWindowIcon(win->windowIcon());
				dock->setWidget(win);
				addDockWidget(Qt::TopDockWidgetArea,dock);
				dock->setFloating(true);
				dock->setAllowedAreas(Qt::NoDockWidgetArea);
				
				dock->resize(width()/2,height()/2);
				dock->move(pos());
				dock->show();
				setCurrentWindow(win);
			}
		}
	}
	
	void MainWindow::popIn(NetworkWindow * win)
	{
		if (allowViewModeToChange && win && tabWidget)
		{
			int i = tabWidget->indexOf(win);
			if (i == -1)
			{
				if (win->parentWidget())
				{
					win->setParent(0);
					delete win->parentWidget();
				}
				tabWidget->addTab(win,win->windowIcon(),win->windowTitle());
				setCurrentWindow(win);
			}
		}
	}
}
