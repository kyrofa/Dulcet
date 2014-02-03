#include <QDebug>
#include <QUrl>
#include <QDir>
#include <QStandardPaths>
#include <QListView>
#include <QTreeView>
#include <QSqlDatabase>
#include <QSqlTableModel>
#include <QSortFilterProxyModel>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QSqlIndex>
#include <QMessageBox>
#include <QSettings>

#include <QPluginLoader>
#include "plugins/plugin_interfaces/music_view_interface.h"

#include "music_file_type_resolver.h"
#include "media_manager.h"

#include "file_dialog.h"
#include "preferences_dialog.h"

#include "main_window.h"
#include "ui_main_window.h"

#define TIME_TO_DECIDE_MS 2000 // Milliseconds
#define DEFAULT_MUSIC_VIEW "Songs View"
#define DEFAULT_MUSIC_LIBRARY_PATH QDir::homePath() + "/Music"
#define DEFAULT_COPY_ON_IMPORT false
#define DEFAULT_ORGANIZE_LIBRARY false
#define DEFAULT_WATCH_LIBRARY false
#define DEFAULT_WRITE_TAGS_TO_FILE false

MainWindow::MainWindow(QWidget *parent) :
   QMainWindow(parent),
   m_ui(new Ui::MainWindow),
   m_mediaPlayer(new QMediaPlayer(this)),
   m_mediaManager(new MediaManager(this)),
   m_fileTypeResolver(new MusicFileTypeResolver),
   m_proxyModel(new QSortFilterProxyModel(this)),
   m_currentMusicView(NULL)
{
	m_ui->setupUi(this);

	TagLib::FileRef::addFileTypeResolver(m_fileTypeResolver);

	QSqlDatabase database = QSqlDatabase::addDatabase("QSQLITE");

	QString databaseFilePath = "Dulcet.db";
	QStringList possibleDataLocations = QStandardPaths::standardLocations(QStandardPaths::DataLocation);

	if (!possibleDataLocations.isEmpty())
	{
		QString dataPath = possibleDataLocations.first();
		if (m_mediaManager->musicDirectory().mkpath(dataPath))
		{
			databaseFilePath = dataPath + "/" + databaseFilePath;
		}
		else
		{
			qWarning("Could not create data directory %s", dataPath.toUtf8().constData());
		}
	}

	database.setDatabaseName(databaseFilePath);

	if (!database.open())
	{
		QMessageBox::critical(this, tr("Cannot open database"),
		                      tr("Unable to establish a database connection.\n"
		                         "This program needs SQLite support.\n"
		                         "Click Cancel to exit."), QMessageBox::Cancel);
		exit(-1);
	}

	QSqlQuery query;
	if (!query.exec("CREATE TABLE IF NOT EXISTS music (id integer primary key)"))
	{
		QMessageBox::critical(this, tr("Cannot create table"),
		                      tr("Click Cancel to exit."), QMessageBox::Cancel);
		exit(-2);
	}

	// The following queries may or may not succeed depending on if the table already
	// has those columns. This is so the database can be expanded in the future.
	query.exec("ALTER TABLE music ADD COLUMN path text");
	query.exec("ALTER TABLE music ADD COLUMN title text");
	query.exec("ALTER TABLE music ADD COLUMN artist text");
	query.exec("ALTER TABLE music ADD COLUMN album text");
	query.exec("ALTER TABLE music ADD COLUMN comment text");
	query.exec("ALTER TABLE music ADD COLUMN genre text");
	query.exec("ALTER TABLE music ADD COLUMN year integer");
	query.exec("ALTER TABLE music ADD COLUMN track integer");
	query.exec("ALTER TABLE music ADD COLUMN rating integer");

	m_mediaTableModel = new QSqlTableModel(this);
	m_mediaTableModel->setTable("music");
	m_mediaTableModel->select();

	QSqlRecord tmpRecord = m_mediaTableModel->record();

	// Set the column header names
	for (int i = 0; i < tmpRecord.count(); ++i)
	{
		m_mediaTableModel->setHeaderData(i, Qt::Horizontal, tr("%1").arg(tmpRecord.fieldName(i)));
	}

	connect(m_ui->searchLineEdit, SIGNAL(textChanged(QString)), SLOT(handleSearch(QString)));

	connect(m_mediaPlayer, SIGNAL(stateChanged(QMediaPlayer::State)), SLOT(handlePlayerStateChange(QMediaPlayer::State)));

	connect(m_ui->viewWidget, SIGNAL(requestImport(QList<QUrl>)), SLOT(handleImport(QList<QUrl>)));

	connect(m_mediaManager, SIGNAL(importedMedia(int,QSqlRecord)), SLOT(handleImportedMedia(int,QSqlRecord)));

	connect(m_mediaManager, SIGNAL(deletedMedia(QSqlRecord)), SLOT(handleMediaDeleted(QSqlRecord)));

	connect(m_ui->importAction, SIGNAL(triggered()), SLOT(showImportDialog()));
	connect(m_ui->preferencesAction, SIGNAL(triggered()), SLOT(showPreferences()));

	m_proxyModel->setSourceModel(m_mediaTableModel);
	m_proxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);

	m_mediaManager->setModel(m_mediaTableModel);
	m_mediaManager->setCopyOnImport(true);
	m_mediaManager->setOrganizeLibrary(true);
	m_mediaManager->setWriteTagsToFile(true);

	if (loadPlugins())
	{
		QActionGroup *musicViewInterfacesGroup = new QActionGroup(this);
		musicViewInterfacesGroup->setExclusive(true);
		connect(musicViewInterfacesGroup, SIGNAL(triggered(QAction*)), SLOT(handleMusicViewChange(QAction*)));

		// Set the model for all the MusicView plugins.
		// Also ensure that they're notified as to which
		// song is currently playing.
		MusicViewMap_t::iterator iterator = m_musicViews.begin();

		while (iterator != m_musicViews.end())
		{
			MusicViewInterface *thisInterface = iterator.value();
			QString interfaceName = thisInterface->objectName();
			QAbstractItemView *interfaceView = thisInterface->view();

			if (interfaceView)
			{
				interfaceView->setParent(this); // Take ownership of the GUI
				interfaceView->hide(); // Hide the GUI
				thisInterface->setModel(m_proxyModel);
				thisInterface->connect(this, SIGNAL(playing(QPersistentModelIndex)), SLOT(handlePlaying(QPersistentModelIndex)));

				QAction *action = musicViewInterfacesGroup->addAction(interfaceName);
				action->setCheckable(true);
				action->setChecked(false);
			}
			else
			{
				qWarning("%s is an invalid plugin (it doesn't implement a view). Not activating.",
				         interfaceName.toUtf8().constData());
				iterator = m_musicViews.erase(iterator);
				continue;
			}

			++iterator;
		}

		m_ui->viewMenu->addActions(musicViewInterfacesGroup->actions());
	}
	else
	{
		qWarning("Couldn't find any plugins! Functionality will be limited.");
	}

	loadSettings();
}

MainWindow::~MainWindow()
{
	delete m_ui;
	delete m_fileTypeResolver;
}

void MainWindow::closeEvent(QCloseEvent *event)
{
	saveSettings();

	QMainWindow::closeEvent(event);
}

void MainWindow::playSong(const QModelIndex &index)
{
	QSqlRecord sqlRecord = m_mediaTableModel->record(m_proxyModel->mapToSource(index).row());
	QString title = sqlRecord.value("title").toString();
	QString artist = sqlRecord.value("artist").toString();
	QString album = sqlRecord.value("album").toString();

	m_ui->songInformationLabel->setText(tr("<strong>%1</strong><br />%2 - %3").arg(title).arg(artist).arg(album));
	setWindowTitle(tr("%1 - %2  |  %3").arg(title).arg(artist).arg(QApplication::applicationName()));

	emit playing(index);

	m_mediaPlayer->setMedia(QUrl::fromLocalFile(sqlRecord.value("path").toString()));
	m_mediaPlayer->setVolume(100);
	m_mediaPlayer->play();
}

void MainWindow::handleImport(const QList<QUrl> &itemsToImport)
{
	if (!itemsToImport.isEmpty())
	{
		m_mediaManager->beginImportMedia();

		// Extract the local paths of the files
		for (int i = 0; i < itemsToImport.size(); ++i)
		{
			QFileInfo fileInfo(itemsToImport.at(i).toLocalFile());

			if (fileInfo.exists())
			{
				m_mediaManager->importMedia(fileInfo);
			}
		}

		m_mediaManager->endImportMedia();
	}
}

void MainWindow::handleImportedMedia(int row, const QSqlRecord &record)
{
	Q_UNUSED(row);

	statusBar()->showMessage(tr("Imported %1").arg(record.value("title").toString()), 2000);

	if (m_currentMusicView && !m_currentMusicView->view()->isVisible())
	{
		showMusicView(m_currentMusicView->objectName());
	}
}

void MainWindow::handleMediaDeleted(const QSqlRecord &recordRemoved)
{
	Q_UNUSED(recordRemoved);

	if (m_currentMusicView && (m_mediaTableModel->rowCount() == 0))
	{
		m_currentMusicView->view()->hide();
		m_ui->placeHolderLabel->show();
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

void MainWindow::handleSearch(const QString &searchString)
{
	m_proxyModel->setFilterFixedString(searchString);
}

void MainWindow::handleMusicViewChange(QAction *action)
{
	QString actionText = action->text();

	if (m_musicViews.contains(actionText))
	{
		if (m_currentMusicView)
		{
			// Make sure the old view no longer has the power to influence which song is playing
			disconnect(m_currentMusicView, SIGNAL(playSong(QModelIndex)), this, SLOT(playSong(QModelIndex)));
			m_currentMusicView->view()->hide();
		}

		m_ui->placeHolderLabel->hide();

		delete m_ui->viewWidget->layout();

		m_currentMusicView = m_musicViews.value(actionText);
		QAbstractItemView *view = m_currentMusicView->view();

		QVBoxLayout *layout = new QVBoxLayout(m_ui->viewWidget);
		layout->addWidget(view);
		layout->addWidget(m_ui->placeHolderLabel);

		if (m_mediaTableModel->rowCount() > 0)
		{
			view->show();
		}
		else
		{
			m_ui->placeHolderLabel->show();
		}

		m_ui->viewWidget->setLayout(layout);

		// Give this view the power to influence which song is playing
		connect(m_currentMusicView, SIGNAL(playSong(QModelIndex)), SLOT(playSong(QModelIndex)));

		action->setChecked(true);
	}
	else
	{
		qWarning("Invalid music view: %s", actionText.toUtf8().constData());
	}
}

void MainWindow::showImportDialog()
{
	QString startingPath = QDir::homePath();
	QStringList musicLocations = QStandardPaths::standardLocations(QStandardPaths::MusicLocation);

	if (!musicLocations.isEmpty())
	{
		startingPath = musicLocations.first();
	}

	FileDialog *dialog = new FileDialog(this,
	                                    tr("Import Media"),
	                                    startingPath);

	if (dialog->exec() == QFileDialog::Accepted)
	{
		handleImport(dialog->selectedUrls());
	}

	dialog->deleteLater();
}

void MainWindow::showPreferences()
{
	PreferencesDialog *dialog = new PreferencesDialog(this);

	dialog->setMusicLibraryPath(m_mediaManager->musicDirectory().absolutePath());
	dialog->setCopyOnImport(m_mediaManager->copyOnImport());
	dialog->setOrganizeLibrary(m_mediaManager->organizeLibrary());
	dialog->setWatchLibrary(m_mediaManager->watchLibrary());
	dialog->setWriteTagsToFile(m_mediaManager->writeTagsToFile());

	if (dialog->exec() == QDialog::Accepted)
	{
		m_mediaManager->setMusicDirectory(dialog->musicLibraryPath(), false);
		m_mediaManager->setCopyOnImport(dialog->copyOnImport());
		m_mediaManager->setOrganizeLibrary(dialog->organizeLibrary());
		m_mediaManager->setWatchLibrary(dialog->watchLibrary()); // This will re-index
		m_mediaManager->setWriteTagsToFile(dialog->writeTagsToFile());

		saveSettings();
	}

	dialog->deleteLater();
}

void MainWindow::showMusicView(const QString &musicView)
{
	// Check the action if present
	QList<QAction *> viewMenuActions = m_ui->viewMenu->actions();
	foreach (QAction *thisAction, viewMenuActions)
	{
		if (thisAction->text() == musicView)
		{
			handleMusicViewChange(thisAction);
			return;
		}
	}

	qWarning("Invalid music view: %s", musicView.toUtf8().constData());
}

void MainWindow::on_playPauseButton_clicked()
{
	switch (m_mediaPlayer->state())
	{
		case QMediaPlayer::StoppedState:
		{
			// call plugin next song
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

void MainWindow::on_previousButton_clicked()
{
	if ((m_mediaPlayer->position() > TIME_TO_DECIDE_MS))
	{
		m_mediaPlayer->setPosition(0); // Restart current track
	}
	else
	{
		if (m_currentMusicView)
		{
			QModelIndex previousSong = m_currentMusicView->previousSong();
			if (previousSong.isValid())
			{
				playSong(previousSong);
			}
		}
	}
}

void MainWindow::on_nextButton_clicked()
{
	if (m_currentMusicView)
	{
		QModelIndex nextSong = m_currentMusicView->nextSong();
		if (nextSong.isValid())
		{
			playSong(nextSong);
		}
	}
}

bool MainWindow::loadPlugins()
{
	QDir pluginsDir(qApp->applicationDirPath());
	pluginsDir.cd("plugins");

	bool pluginsFound = false;

	foreach (QString directoryName, pluginsDir.entryList(QDir::AllDirs|QDir::NoDotAndDotDot))
	{
		pluginsDir.cd(pluginsDir.absoluteFilePath(directoryName));

		foreach (QString fileName, pluginsDir.entryList(QDir::Files))
		{
			QPluginLoader pluginLoader(pluginsDir.absoluteFilePath(fileName));

			QObject *plugin = pluginLoader.instance();

			if (plugin)
			{
				if (plugin->inherits("MusicViewInterface"))
				{
					QString objectName = plugin->objectName();

					if (objectName.isEmpty())
					{
						objectName = pluginLoader.fileName();
						plugin->setObjectName(objectName);
						qWarning("Plugin %s has no object name! Using filename instead...", objectName.toUtf8().constData());
					}

					if (!objectName.isEmpty())
					{
						m_musicViews[objectName] = qobject_cast<MusicViewInterface *>(plugin);
						pluginsFound = true;

						qDebug("Loaded plugin: %s", objectName.toUtf8().constData());
					}
					else
					{
						qWarning("Plugin has no name or filename... this is weird. Not using plugin.");
					}
				}
			}
		}

		pluginsDir.cdUp();
	}

	return pluginsFound;
}

void MainWindow::saveSettings()
{
	QSettings settings;

	settings.beginGroup("MainWindow");
	settings.setValue("geometry", saveGeometry());
	settings.setValue("state", saveState());
	if (m_currentMusicView)
	{
		settings.setValue("current_music_view", m_currentMusicView->objectName());
	}
	settings.endGroup();

	settings.beginGroup("Preferences");
	settings.setValue("music_library_path", m_mediaManager->musicDirectory().absolutePath());
	settings.setValue("copy_on_import", m_mediaManager->copyOnImport());
	settings.setValue("organize_library", m_mediaManager->organizeLibrary());
	settings.setValue("watch_library", m_mediaManager->watchLibrary());
	settings.setValue("write_tags_to_file", m_mediaManager->writeTagsToFile());
	settings.endGroup();

	// Save plugins settings
	MusicViewMap_t::const_iterator iterator;

	for (iterator = m_musicViews.constBegin();
	     iterator != m_musicViews.constEnd();
	     ++iterator)
	{
		if (iterator.value())
		{
			settings.beginGroup(iterator.key());
			iterator.value()->saveSettings(settings);
			settings.endGroup();
		}
	}
}

void MainWindow::loadSettings()
{
	QSettings settings;

	settings.beginGroup("MainWindow");
	restoreGeometry(settings.value("geometry").toByteArray());
	restoreState(settings.value("state").toByteArray());
	showMusicView(settings.value("current_music_view", DEFAULT_MUSIC_VIEW).toString());
	settings.endGroup();

	settings.beginGroup("Preferences");
	QString defaultMusicPath = DEFAULT_MUSIC_LIBRARY_PATH;
	QStringList musicLocations = QStandardPaths::standardLocations(QStandardPaths::MusicLocation);

	if (!musicLocations.isEmpty())
	{
		defaultMusicPath = musicLocations.first();
	}

	m_mediaManager->setMusicDirectory(settings.value("music_library_path", defaultMusicPath).toString(), false);
	m_mediaManager->setCopyOnImport(settings.value("copy_on_import", DEFAULT_COPY_ON_IMPORT).toBool());
	m_mediaManager->setOrganizeLibrary(settings.value("organize_library", DEFAULT_ORGANIZE_LIBRARY).toBool());
	m_mediaManager->setWatchLibrary(settings.value("watch_library", DEFAULT_WATCH_LIBRARY).toBool()); // This will re-index
	m_mediaManager->setWriteTagsToFile(settings.value("write_tags_to_file", DEFAULT_WRITE_TAGS_TO_FILE).toBool());
	settings.endGroup();

	// Load plugins settings
	MusicViewMap_t::const_iterator iterator;

	for (iterator = m_musicViews.constBegin();
	     iterator != m_musicViews.constEnd();
	     ++iterator)
	{
		if (iterator.value())
		{
			settings.beginGroup(iterator.key());
			iterator.value()->loadSettings(settings);
			settings.endGroup();
		}
	}
}
