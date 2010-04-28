/****************************************************************************
 Copyright (c) 2008 Deepak Chandran
 Contact: Deepak Chandran (dchandran1@gmail.com)
 see COPYRIGHT.TXT

 Provides a text window where C code can be written and run dynamically

****************************************************************************/

#ifndef TINKERCELL_CSCRIPT_H
#define TINKERCELL_CSCRIPT_H

#include <QMainWindow>
#include <QTextEdit>
#include <QSyntaxHighlighter>
#include <QHash>
#include <QTextCharFormat>
#include <QDialog>
#include <QCompleter>
#include <QListWidget>
#include <QTabWidget>
#include <QTreeWidget>
#include <QThread>
#include <QToolBar>
#include <QTimeLine>
#include <QActionGroup>
#include <QLineEdit>
#include <QHash>
#include "CodeEditor.h"
#include "Tool.h"
#include "ConsoleWindow.h"
#include "CThread.h"
#include "SyntaxHighlighter.h"

namespace Tinkercell
{
	class RuntimeCodeEditor : public CodeEditor
	{
		Q_OBJECT
	public:
		QCompleter completer;
		QHash<QString,QString> defaultSavedFilename;

	public slots:
		void open(const QString&);
		void save(const QString&);
		void saveAs(const QString&);
		void insertCompletion(const QString& completion);
	};

	class TCFunctionsListView : public QTreeWidget
	{
		Q_OBJECT
	public:
		TCFunctionsListView(MainWindow *, const QString&, CodeEditor * textEdit);
		ConsoleWindow * console;

	signals:
		void insertText(const QString&);

	protected:
		virtual void readCHeaders(const QString& dirName);
		virtual void mouseDoubleClickEvent ( QMouseEvent * event );
		virtual void keyPressEvent ( QKeyEvent * event );
	};

	class CodingWindow : public Tool
	{
	    Q_OBJECT

	public:
		CodingWindow();
		bool setMainWindow(MainWindow*);

	signals:
		void runPy(const QString&);
		void loadPyFromDir( QDir& );
		void stopPy();
		void compileBuildLoadC(const QString&,const QString&,const QString&);
		void compile(const QString&, QString&);

	public slots:
		void run();
		void about();
		void dialogFinished();
		virtual QSize sizeHint() const;
		void newDoc();
		void open();
		void save();
		void undo();
		void redo();
		void selectAll();
		virtual void setVisible(bool);
		void pyscesHelp();

	protected slots:
		void convertCodeToButton();

	protected:

		void setupEditor();
		void setupMenu();
		void setupDialog();
		void runC(const QString&);
		void convertCodeToButtonPy();
		void convertCodeToButtonC();

		QTimeLine timer;
		QMainWindow * window;
		QTabWidget * tabWidget;
		RuntimeCodeEditor *editorC;
		RuntimeCodeEditor *editorPy;
		QToolBar * toolBar;

		RuntimeCodeEditor *editorR;
		QLineEdit *commandCEdit, *commandPyEdit, *fileNameEdit;
		CandPythonSyntaxHighlighter *highlighterC;
		CandPythonSyntaxHighlighter *highlighterPy;

		QString fileName;
		QDialog commandDialog;

	};

}

#endif
