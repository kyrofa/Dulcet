#include <QDebug>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlTableModel>
#include <QtSql/QSqlQuery>
#include <QMessageBox>

#include "music_table_item_delegate.h"

#include "main_window.h"
#include "ui_main_window.h"

MainWindow::MainWindow(QWidget *parent) :
   QMainWindow(parent),
   m_ui(new Ui::MainWindow)
{
	m_ui->setupUi(this);

	QSqlDatabase database = QSqlDatabase::addDatabase("QSQLITE");
	database.setDatabaseName("test.sqlite3");
	if (!database.open())
	{
		QMessageBox::critical(this, tr("Cannot open database"),
		                      tr("Unable to establish a database connection.\n"
		                         "This program needs SQLite support.\n"
		                         "Click Cancel to exit."), QMessageBox::Cancel);
		close();
	}

	QSqlQuery query;
	if (!query.exec("CREATE TABLE IF NOT EXISTS music (path text, title text, artist text, album text)"))
	{
		QMessageBox::critical(this, tr("Cannot create table"),
		                      tr("Click Cancel to exit."), QMessageBox::Cancel);
	}

	m_musicTableModel = new QSqlTableModel(this);
	m_musicTableModel->setTable("music");
	m_musicTableModel->select();
	m_musicTableModel->setHeaderData(0, Qt::Horizontal, tr("Path"));
	m_musicTableModel->setHeaderData(1, Qt::Horizontal, tr("Title"));
	m_musicTableModel->setHeaderData(2, Qt::Horizontal, tr("Artist"));
	m_musicTableModel->setHeaderData(3, Qt::Horizontal, tr("Album"));

	connect(m_ui->songsTableView, SIGNAL(importStatusUpdate(QString)), SLOT(handleImportStatusUpdate(QString)));

	MusicTableItemDelegate *delegate = new MusicTableItemDelegate(m_ui->songsTableView);

	m_ui->songsTableView->setModel(m_musicTableModel);
	m_ui->songsTableView->verticalHeader()->setVisible(false);
	m_ui->songsTableView->hideColumn(0);
	m_ui->songsTableView->setItemDelegate(delegate);
}

MainWindow::~MainWindow()
{
	delete m_ui;
}

void MainWindow::handleImportStatusUpdate(const QString &status)
{
	statusBar()->showMessage(status);
}
