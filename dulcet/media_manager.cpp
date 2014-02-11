#include <QApplication>
#include <QMediaPlayer>
#include <QFileSystemWatcher>
#include <QSqlTableModel>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QSqlError>
#include <QThread>
#include <QDir>
#include <QTime>

#include "dulcet_utilities.h"
#include "media_discoverer.h"
#include "media_organizer.h"
#include "music_file_type_resolver.h"
#include "media_manager.h"

////////// BEGIN PRIVATE DATA //////////
class MediaManagerPrivate
{
	Q_DECLARE_PUBLIC(MediaManager)

	public:
		MediaManagerPrivate();
		~MediaManagerPrivate();

		virtual void updateWatchedPaths(bool reset = false);
		virtual void removeDuplicateDiscoveries(const QStringList &mediaDiscovered, QStringList &newList);
		virtual void validateLibrary();

		virtual bool beginImportMedia();

		// slots:
		virtual void _q_mediaDiscovered(const QStringList &mediaDiscovered);
		virtual void _q_mediaOrganized(const TagLib::FileRef &taggedFile);
		virtual void _q_modelBeforeUpdate(int row, QSqlRecord &record);
		virtual void _q_directoryChanged(const QString &directory);
		virtual void _q_endImportMedia();

		MediaManager *q_ptr;
		QSqlTableModel *model;

		QDir musicDirectory;
		MusicFileTypeResolver fileTypeResolver;
		QFileSystemWatcher fileSystemWatcher;
		bool copyOnImport;
		bool organizeLibrary;
		bool watchLibrary;
		bool writeTagsToFile;

		QThread mediaDiscovererThread;
		MediaDiscoverer mediaDiscoverer;

		QThread mediaOrganizerThread;
		MediaOrganizer mediaOrganizer;

		QStringList watchedItemsBackup;

		bool importing;
};

MediaManagerPrivate::MediaManagerPrivate() :
   q_ptr(NULL),
   model(NULL),
   copyOnImport(true),
   organizeLibrary(false),
   watchLibrary(false),
   writeTagsToFile(false),
   importing(false)
{
	qRegisterMetaType<QStringList>("QStringList");
	qRegisterMetaType<TagLib::FileRef>("TagLib::FileRef");

	mediaDiscoverer.moveToThread(&mediaDiscovererThread);
	mediaDiscovererThread.start();

	mediaOrganizer.moveToThread(&mediaOrganizerThread);
	mediaOrganizerThread.start();
}

MediaManagerPrivate::~MediaManagerPrivate()
{
	mediaDiscovererThread.requestInterruption();
	mediaOrganizerThread.requestInterruption();

	mediaDiscovererThread.quit();
	mediaOrganizerThread.quit();

	mediaDiscovererThread.wait();
	mediaOrganizerThread.wait();
}

void MediaManagerPrivate::updateWatchedPaths(bool reset)
{
	if (watchLibrary)
	{
		QStringList pathsToAdd = fileSystemWatcher.directories() + fileSystemWatcher.files();
		if (reset && !pathsToAdd.isEmpty())
		{
			fileSystemWatcher.removePaths(pathsToAdd);
			pathsToAdd.clear();
		}

		pathsToAdd.append(musicDirectory.absolutePath());

		QFileInfo fileInfo;
		int pathColumn = model->record().indexOf("path");

		if (pathColumn >= 0)
		{
			QString thisPath;

			for (int i = 0; i < model->rowCount(); ++i)
			{
				fileInfo.setFile(model->data(model->index(i, pathColumn)).toString());

				if (DulcetUtilities::isDirectoryAncestorOfFile(musicDirectory, fileInfo))
				{
					thisPath = fileInfo.dir().absolutePath();

					while (!pathsToAdd.contains(thisPath))
					{
						pathsToAdd.append(thisPath);

						fileInfo.setFile(thisPath);
						thisPath = fileInfo.dir().absolutePath();
					}
				}
			}
		}

		fileSystemWatcher.addPaths(pathsToAdd);
	}
}

void MediaManagerPrivate::removeDuplicateDiscoveries(const QStringList &mediaDiscovered, QStringList &newList)
{
	int pathColumn = model->record().indexOf("path");

	if (pathColumn >= 0)
	{
		// Build set in database
		QSet<QString> setInDatabase;
		for (int i = 0; i < model->rowCount(); ++i)
		{
			setInDatabase.insert(model->index(i, pathColumn).data().toString());
		}

		newList = mediaDiscovered.toSet().subtract(setInDatabase).toList();
	}
}

void MediaManagerPrivate::validateLibrary()
{
	Q_Q(MediaManager);

	QFileInfo fileInfo;
	QSqlRecord record;
	int pathColumn = model->record().indexOf("path");

	if (pathColumn >= 0)
	{
		for (int i = 0; i < model->rowCount(); ++i)
		{
			fileInfo.setFile(model->index(i, pathColumn).data().toString());

			if (!fileInfo.exists())
			{
				record = model->record(i);
				model->removeRow(i--);
				model->select();

				emit q->deletedMedia(record);
			}
		}
	}
}

bool MediaManagerPrivate::beginImportMedia()
{
	if (!importing)
	{
		importing = true;

		watchedItemsBackup = fileSystemWatcher.directories() + fileSystemWatcher.files();

		// Temporarily disable the filesystem watcher since an import is about to happen
		// and we'll probably be manually changing the watched system. Don't need the updates.
		if (!watchedItemsBackup.isEmpty())
		{
			fileSystemWatcher.removePaths(watchedItemsBackup);
		}

		return true; // Ready to import
	}

	return false; // Already importing
}

void MediaManagerPrivate::_q_mediaDiscovered(const QStringList &mediaDiscovered)
{
	QStringList newList;

	// Remove file discoveries already in the database
	removeDuplicateDiscoveries(mediaDiscovered, newList);

	mediaOrganizer.addItemsToOrganize(newList);

	QMetaObject::invokeMethod(&mediaOrganizer, "beginOrganization");
}

void MediaManagerPrivate::_q_mediaOrganized(const TagLib::FileRef &taggedFile)
{
	Q_Q(MediaManager);

	QSqlRecord newRecord = model->record();

	// Remove the ID, or the cache will get out of sync with the database
	// and the row won't appear.
	newRecord.remove(0);

	DulcetUtilities::importTagsToRecord(taggedFile, newRecord);

	if (model->insertRecord(-1, newRecord))
	{
		emit q->importedMedia(model->rowCount());
	}
}

void MediaManagerPrivate::_q_modelBeforeUpdate(int row, QSqlRecord &record)
{
	Q_UNUSED(row);

	QString filePath = record.value("path").toString();
	TagLib::FileRef taggedFile(filePath.toUtf8().constData());

	if (writeTagsToFile)
	{
		DulcetUtilities::saveTagsToFile(taggedFile, record);
	}

	if (organizeLibrary && DulcetUtilities::organizeFile(musicDirectory, taggedFile,
	                                                     fileTypeResolver.mimeTypeForFile(filePath),
	                                                     copyOnImport))
	{
		record.setValue("path", taggedFile.file()->name());
		record.setGenerated("path", true);
	}
}

void MediaManagerPrivate::_q_directoryChanged(const QString &directory)
{
	Q_Q(MediaManager);

	qDebug() << "Directory changed:" << directory;

	// Validate that the songs we know about are still there
	validateLibrary();

	// Find any new songs that have been put there
	q->importMedia(QFileInfo(directory));
}

void MediaManagerPrivate::_q_endImportMedia()
{
	// Done importing-- we can re-enable the file system watcher
	// and add the newly imported media to the watch list
	if (!watchedItemsBackup.isEmpty())
	{
		fileSystemWatcher.addPaths(watchedItemsBackup);
		watchedItemsBackup.clear(); // Get rid of the backup
		updateWatchedPaths();
	}

	importing = false;
}

////////// END PRIVATE DATA //////////

MediaManager::MediaManager(QObject *parent) :
   QObject(parent),
   d_ptr(new MediaManagerPrivate)
{
	Q_D(MediaManager);

	d->q_ptr = this;

	connect(&d->fileSystemWatcher, SIGNAL(directoryChanged(QString)), SLOT(_q_directoryChanged(QString)));
	connect(&d->mediaDiscoverer, SIGNAL(discoveredMedia(QStringList)), SLOT(_q_mediaDiscovered(QStringList)));
	connect(&d->mediaOrganizer, SIGNAL(organizedMedia(TagLib::FileRef)), SLOT(_q_mediaOrganized(TagLib::FileRef)));
}

MediaManager::~MediaManager()
{
}

bool MediaManager::setDatabase(const QString &adapterType, const QString &databaseFilePath)
{
	Q_D(MediaManager);

	QSqlDatabase defaultDatabase = QSqlDatabase::addDatabase(adapterType);
	defaultDatabase.setDatabaseName(databaseFilePath);

	QSqlDatabase importDatabase = QSqlDatabase::addDatabase(adapterType, "import");
	importDatabase.setDatabaseName(databaseFilePath);

	if (!defaultDatabase.open() || !importDatabase.open())
	{
		qFatal("Cannot open database! This program needs SQLite support.");
		return false;
	}

	QSqlQuery query;
	if (!query.exec("CREATE TABLE IF NOT EXISTS music (id integer primary key)"))
	{
		qFatal("Cannot create table: %s", defaultDatabase.lastError().text().toUtf8().constData());
		return false;
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

	if (d->model)
	{
		disconnect(d->model, SIGNAL(beforeUpdate(int,QSqlRecord&)), this, SLOT(_q_modelBeforeUpdate(int,QSqlRecord&)));
		d->model->deleteLater();
	}

	d->model = new QSqlTableModel(this);
	d->model->setTable("music");

	connect(d->model, SIGNAL(beforeUpdate(int,QSqlRecord&)), SLOT(_q_modelBeforeUpdate(int,QSqlRecord&)));

	QSqlRecord tmpRecord = d->model->record();

	// Set the column header names
	for (int i = 0; i < tmpRecord.count(); ++i)
	{
		d->model->setHeaderData(i, Qt::Horizontal, tr("%1").arg(tmpRecord.fieldName(i)));
	}

	return d->model->select();
}

QSqlTableModel *MediaManager::model()
{
	Q_D(MediaManager);

	return d->model;
}

bool MediaManager::setMusicDirectory(const QDir &directory, bool reindex)
{
	Q_D(MediaManager);

	if (d->musicDirectory != directory)
	{
		if (!directory.exists())
		{
			QString path = directory.absolutePath();
			QByteArray pathArray = path.toUtf8();
			const char *pathData = pathArray.constData();

			qDebug("%s does not exist. Creating it...", pathData);
			if (!directory.mkpath(path))
			{
				qDebug("Could not create %s, and thus will be unable to organize library.", pathData);
				return false;
			}
		}

		d->musicDirectory = directory;
		d->mediaOrganizer.setLibraryDirectory(directory);

		// Re-index music directory and watch the folders inside, if requested
		if (reindex)
		{
			d->updateWatchedPaths(true);
		}
	}

	return true;
}

const QDir& MediaManager::musicDirectory() const
{
	Q_D(const MediaManager);

	return d->musicDirectory;
}

void MediaManager::importMedia(const QFileInfo &fileInfo)
{
	importMedia(QList<QFileInfo>() << fileInfo);
}

void MediaManager::importMedia(const QList<QFileInfo> &itemsToImport)
{
	Q_D(MediaManager);

	d->mediaDiscoverer.addSearchItems(itemsToImport);
	QMetaObject::invokeMethod(&d->mediaDiscoverer, "beginDiscovery");
}

void MediaManager::setCopyOnImport(bool enabled)
{
	Q_D(MediaManager);

	d->copyOnImport = enabled;
	d->mediaOrganizer.setCopyOnImport(enabled);
}

bool MediaManager::copyOnImport() const
{
	Q_D(const MediaManager);

	return d->copyOnImport;
}

void MediaManager::setOrganizeLibrary(bool enabled)
{
	Q_D(MediaManager);

	d->organizeLibrary = enabled;
	d->mediaOrganizer.setOrganizeLibrary(enabled);
}

bool MediaManager::organizeLibrary() const
{
	Q_D(const MediaManager);

	return d->organizeLibrary;
}

void MediaManager::setWatchLibrary(bool enabled, bool reindex)
{
	Q_D(MediaManager);

	d->watchLibrary = enabled;

	if (enabled && reindex)
	{
		d->updateWatchedPaths();
	}
	else
	{
		QStringList currentlyWatchedPaths = d->fileSystemWatcher.directories() + d->fileSystemWatcher.files();

		if (!currentlyWatchedPaths.isEmpty())
		{
			d->fileSystemWatcher.removePaths(currentlyWatchedPaths);
		}
	}
}

bool MediaManager::watchLibrary() const
{
	Q_D(const MediaManager);

	return d->watchLibrary;
}

void MediaManager::setWriteTagsToFile(bool enabled)
{
	Q_D(MediaManager);

	d->writeTagsToFile = enabled;
}

bool MediaManager::writeTagsToFile() const
{
	Q_D(const MediaManager);

	return d->writeTagsToFile;
}

bool MediaManager::isSupported(const QString &mimeType, const QStringList &codecs) const
{
	// QMediaPlayer::hasSupport() is buggy right now... it just always returns MaybeSupported.
	// So we'll supplement it here by making sure it's either video or audio (thus fairly likely
	// to play successfully) and then check for support (in case it's fixed in the future).
	if (mimeType.startsWith("audio/") || mimeType.startsWith("video/"))
	{
		if (QMediaPlayer::hasSupport(mimeType, codecs) >= QMultimedia::MaybeSupported)
		{
			return true;
		}
	}

	return false;
}

#include "moc_media_manager.cpp" // So Q_PRIVATE_SLOT works
