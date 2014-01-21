#include <QDebug>
#include <QUrl>
#include <QDir>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlTableModel>
#include <QtSql/QSqlQuery>
#include <QMessageBox>

#include "music_file_type_resolver.h"
#include "music_table_item_delegate.h"

#include "main_window.h"
#include "ui_main_window.h"

MainWindow::MainWindow(QWidget *parent) :
   QMainWindow(parent),
   m_ui(new Ui::MainWindow),
   m_mediaPlayer(new QMediaPlayer(this)),
   m_fileTypeResolver(new MusicFileTypeResolver)
{
	m_ui->setupUi(this);

	TagLib::FileRef::addFileTypeResolver(m_fileTypeResolver);

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

	connect(m_mediaPlayer, SIGNAL(stateChanged(QMediaPlayer::State)), SLOT(handlePlayerStateChange(QMediaPlayer::State)));

	connect(m_ui->songsTableView, SIGNAL(requestImport(QList<QUrl>)), SLOT(handleImport(QList<QUrl>)));
	connect(m_ui->songsTableView, SIGNAL(doubleClicked(QModelIndex)), SLOT(handleSongDoubleClicked(QModelIndex)));

	MusicTableItemDelegate *delegate = new MusicTableItemDelegate(m_ui->songsTableView);

	m_ui->songsTableView->setModel(m_musicTableModel);
	m_ui->songsTableView->hideColumn(0);
	m_ui->songsTableView->setItemDelegate(delegate);
	m_ui->songsTableView->horizontalHeader()->setSectionsMovable(true);
}

MainWindow::~MainWindow()
{
	delete m_ui;
	delete m_fileTypeResolver;
}

void MainWindow::handleSongDoubleClicked(const QModelIndex &index)
{
	playSong(QUrl::fromLocalFile(m_musicTableModel->data(m_musicTableModel->index(index.row(), 0)).toString()));
}

void MainWindow::playSong(const QUrl &fileUrl)
{
	m_mediaPlayer->setMedia(fileUrl);
	m_mediaPlayer->setVolume(100);
	m_mediaPlayer->play();
}

void MainWindow::handleImport(const QList<QUrl> &itemsToImport)
{
	// Extract the local paths of the files
	for (int i = 0; i < itemsToImport.size(); ++i)
	{
		recursivelyImportMusic(itemsToImport.at(i).toLocalFile());
	}
}

void MainWindow::handlePlayerStateChange(QMediaPlayer::State newState)
{
	switch (newState)
	{
		case QMediaPlayer::PlayingState:
			m_ui->playPauseButton->setIcon(QIcon(":/images/pause_button"));
			break;
		default:
			m_ui->playPauseButton->setIcon(QIcon(":/images/play_button"));
			break;
	}
}

void MainWindow::on_playPauseButton_clicked()
{
	switch (m_mediaPlayer->state())
	{
		case QMediaPlayer::StoppedState:
		{
			QItemSelectionModel *selectionModel = m_ui->songsTableView->selectionModel();
			if (selectionModel->hasSelection())
			{
				int row = selectionModel->selectedRows().at(0).row();
				playSong(QUrl::fromLocalFile(m_musicTableModel->data(m_musicTableModel->index(row, 0)).toString()));
			}
			break;
		}
		case QMediaPlayer::PlayingState:
			m_mediaPlayer->pause();
			break;
		case QMediaPlayer::PausedState:
			m_mediaPlayer->play();
			break;
		default:
			m_mediaPlayer->stop();
			break;
	}
}

void MainWindow::recursivelyImportMusic(const QString &path)
{
	recursivelyImportMusic(QFileInfo(path));
}

void MainWindow::recursivelyImportMusic(const QFileInfo &fileInfo)
{
	qApp->processEvents();

	if (fileInfo.isDir())
	{
		QDir directory(fileInfo.absoluteFilePath());

		QFileInfoList fileInfoList = directory.entryInfoList(QDir::AllEntries | QDir::NoDotAndDotDot);

		foreach(const QFileInfo &thisFileInfo, fileInfoList)
		{
			recursivelyImportMusic(thisFileInfo);
		}
	}
	else
	{
		if (isSupported(m_fileTypeResolver->mimeTypeForFile(fileInfo).name()))
		{
			int rowCount = m_musicTableModel->rowCount();
			QString filePath = fileInfo.absoluteFilePath();
			TagLib::FileRef fileTags(filePath.toUtf8().constData());

			m_musicTableModel->insertRows(rowCount, 1);
			m_musicTableModel->setData(m_musicTableModel->index(rowCount, 0),
			                           filePath);
			m_musicTableModel->setData(m_musicTableModel->index(rowCount, 1),
			                           fileTags.tag()->title().toCString());
			m_musicTableModel->setData(m_musicTableModel->index(rowCount, 2),
			                           fileTags.tag()->artist().toCString());
			m_musicTableModel->setData(m_musicTableModel->index(rowCount, 3),
			                           fileTags.tag()->album().toCString());
			m_musicTableModel->submit();

			//qDebug() << "Artist:" << f.tag()->artist().toCString(); // artist == "Frank Zappa"
			//			f.tag()->setAlbum("Fillmore East");
			//			f.save();
			//			TagLib::FileRef g("Free City Rhymes.ogg");
			//			TagLib::String album = g.tag()->album(); // album == "NYC Ghosts & Flowers"
			//			g.tag()->setTrack(1);
			//			g.save();

			statusBar()->showMessage(tr("Importing %1").arg(fileInfo.fileName()));
		}
	}
}

bool MainWindow::isSupported(const QString &mimeType, const QStringList &codecs)
{
	// QMediaPlayer::hasSupport() is buggy right now... it just always returns MaybeSupported.
	// So we'll supplement it here by making sure it's either video or audio (thus fairly likely
	// to play successfully) and then check for support (in case it's fixed in the future).
	if (mimeType.startsWith("audio/") || mimeType.startsWith("video/"))
	{
		if (m_mediaPlayer->hasSupport(mimeType, codecs) >= QMultimedia::MaybeSupported)
		{
			return true;
		}
	}

	return false;
}
