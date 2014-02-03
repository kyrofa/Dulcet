#include <QApplication>
#include <QMediaPlayer>
#include <QFileSystemWatcher>
#include <QSqlTableModel>
#include <QSqlRecord>
#include <QDir>
#include <QTime>

#include "music_file_type_resolver.h"
#include "media_manager.h"

// Neither the directory nor the file need to exist
bool isDirectoryAncestorOfFile(const QDir &directory, const QFileInfo& file)
{
//	if (directory.exists() && file.exists())
//	{
		return file.absolutePath().contains(directory.absolutePath());
//	}

//	return false;
}

bool saveTagsToFile(TagLib::FileRef &file, const QSqlRecord &record)
{
	TagLib::Tag *tag = file.tag();

	if (tag)
	{
		tag->setTitle(record.value("title").toString().toUtf8().constData());
		tag->setArtist(record.value("artist").toString().toUtf8().constData());
		tag->setAlbum(record.value("album").toString().toUtf8().constData());
		tag->setComment(record.value("comment").toString().toUtf8().constData());
		tag->setGenre(record.value("genre").toString().toUtf8().constData());
		tag->setYear(record.value("year").toUInt());
		tag->setTrack(record.value("track").toUInt());

		return file.save();
	}

	qWarning("Invalid tag! Cannot save to file...");

	return false;
}

bool importTagsToRecord(const TagLib::FileRef &file, QSqlRecord &record)
{
	TagLib::Tag *tag = file.tag();

	if (tag)
	{
		record.setValue("path", file.file()->name());
		record.setValue("title", tag->title().toCString(true));
		record.setValue("artist", tag->artist().toCString(true));
		record.setValue("album", tag->album().toCString(true));
		record.setValue("comment", tag->comment().toCString(true));
		record.setValue("genre", tag->genre().toCString(true));
		record.setValue("year", tag->year());
		record.setValue("track", tag->track());

		return true;
	}

	qWarning("Invalid tag! Cannot import to record...");


	return false;
}

////////// BEGIN PRIVATE DATA //////////
class MediaManagerPrivate
{
	Q_DECLARE_PUBLIC(MediaManager)

	public:
		MediaManagerPrivate();

		virtual bool organizeFile(TagLib::FileRef &file);
		virtual bool generateAnticipatedPath(const TagLib::FileRef &file, QString &path,
		                                     QString &fileName, QString &fileSuffix);
		virtual void updateWatchedPaths(bool reset = false);
		virtual void validateLibrary();
		virtual void recursivelyImportMedia(const QFileInfo &fileInfo);

		// slots:
		virtual void _q_modelDestroyed();
		virtual void _q_modelBeforeUpdate(int row, QSqlRecord &record);
		virtual void _q_directoryChanged(const QString &directory);

		MediaManager *q_ptr;
		QSqlTableModel *model;
		QDir musicDirectory;
		MusicFileTypeResolver fileTypeResolver;
		QFileSystemWatcher fileSystemWatcher;
		bool copyOnImport;
		bool organizeLibrary;
		bool watchLibrary;
		bool writeTagsToFile;

		QStringList watchedItemsBackup;

		bool importing;

		QVector<int> importTimes;
		QTime importTimer;
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
	importTimer.start();
}

bool MediaManagerPrivate::organizeFile(TagLib::FileRef &file)
{
	QString path, fileName, fileSuffix;
	if (generateAnticipatedPath(file, path, fileName, fileSuffix))
	{
		QString oldPath = file.file()->name();
		QString anticipatedPath = path + fileName + "." + fileSuffix;

		// Do we need to organize this file?
		if (anticipatedPath != oldPath)
		{
			QFile oldFile(oldPath);
			QFile newFile(anticipatedPath);

			// Find a new filename that doesn't overwrite another
			int counter = 1;
			while (newFile.exists())
			{
				newFile.setFileName(QString("%1_%2.%3").arg(path+fileName).arg(counter).arg(fileSuffix));
				counter++;
			}

			QFileInfo oldFileInfo(oldFile);
			QFileInfo newFileInfo(newFile);
			QString newFilePath = newFileInfo.absoluteFilePath();
			QString newPath = newFileInfo.absolutePath();

			// Continue as long as the path either already exists or was successfully made
			if (QFileInfo::exists(newPath) || musicDirectory.mkpath(newPath))
			{
//				QString basePath = musicDirectory.absolutePath();
//				int numberOfLibraryPathItems = basePath.count('/');
//				QStringList directoryList = newPath.split('/', QString::SkipEmptyParts).mid(numberOfLibraryPathItems);

//				for (int i = 0; i < directoryList.size(); ++i)
//				{
//					basePath += "/" + directoryList.at(i);
//					directoryList[i] = basePath;
//				}

//				fileSystemWatcher.addPaths(directoryList);

				bool success = false;

				if (copyOnImport && !isDirectoryAncestorOfFile(musicDirectory, oldFileInfo))
				{
					success = oldFile.copy(newFilePath);
				}
				else
				{
					success = oldFile.rename(newFilePath);
				}

				// If the reorganization succeeded, update the record
				// before it makes it to the database
				if (success)
				{
					file = TagLib::FileRef(newFilePath.toUtf8().constData());

					return true;
				}
			}
			else
			{
				qWarning("Cannot create directory: %s. Cannot organize %s.",
				         newFilePath.toUtf8().constData(),
				         newFile.fileName().toUtf8().constData());
			}
		}
	}

	return false;
}

bool MediaManagerPrivate::generateAnticipatedPath(const TagLib::FileRef &file, QString &path,
                                                  QString &fileName, QString &fileSuffix)

{
	if (musicDirectory.exists())
	{
		TagLib::Tag *tag = file.tag();

		quint32 trackNumber = tag->track();
		QString title = tag->title().toCString(true);
		QString artist = tag->artist().toCString(true);
		QString album = tag->album().toCString(true);

		if (title.isEmpty())
		{
			title = "Unknown";
		}

		if (artist.isEmpty())
		{
			artist = "Unknown";
		}

		if (album.isEmpty())
		{
			album = "Unknown";
		}

		// Remove illegal characters
		title.replace('/', '_');
		artist.replace('/', '_');
		album.replace('/', '_');

		path = musicDirectory.absolutePath() + "/";
		path.append(artist + "/");
		path.append(album + "/");

		fileName = "";
		if (trackNumber > 0)
		{
			fileName.append(QString("%1 - ").arg(trackNumber));
		}
		fileName.append(title);

		// FileTypeResolver's suffixForFileName only uses the filename to determine
		// the suffix. So we'll use QMimeType's instead.
		fileSuffix = fileTypeResolver.mimeTypeForFile(file.file()->name()).preferredSuffix();

		return true;
	}

	return false;
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

				if (isDirectoryAncestorOfFile(musicDirectory, fileInfo))
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
			fileInfo.setFile(model->data(model->index(i, pathColumn)).toString());

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

void MediaManagerPrivate::recursivelyImportMedia(const QFileInfo &fileInfo)
{
	Q_Q(MediaManager);

	if (model)
	{
		qApp->processEvents();

		QString filePath = fileInfo.absoluteFilePath();

		if (fileInfo.isDir())
		{
			QDir directory(filePath);

			QFileInfoList fileInfoList = directory.entryInfoList(QDir::AllEntries | QDir::NoDotAndDotDot);

			foreach(const QFileInfo &thisFileInfo, fileInfoList)
			{
				recursivelyImportMedia(thisFileInfo);
			}
		}
		else
		{
			importTimer.restart();
			int pathColumn = model->record().indexOf("path");
			QString thisPath;
			for (int i = 0; i < model->rowCount(); ++i)
			{
				thisPath = model->data(model->index(i, pathColumn)).toString();

				if (filePath == thisPath)
				{
					return; // We've already imported this item
				}
			}

			if (q->isSupported(fileTypeResolver.mimeTypeForFile(fileInfo).name()))
			{
				QByteArray filePathArray = filePath.toUtf8();
				const char *filePathData = filePathArray.constData();
				TagLib::FileRef taggedFile(filePathData);

				QSqlRecord newRecord = model->record();
				// Remove the ID, or the cache will get out of sync with the database
				// and the row won't appear.
				newRecord.remove(0);

				if (organizeLibrary && !organizeFile(taggedFile))
				{
					qWarning("Unable to organize %s. Skipping...", filePathData);
					return;
				}

				importTagsToRecord(taggedFile, newRecord);

				if (model->insertRecord(-1, newRecord))
				{
					emit q->importedMedia(model->rowCount(), newRecord);
				}
			}

			importTimes.push_back(importTimer.elapsed());

			float sum = 0;
			for (int i = 0; i < importTimes.size(); ++i)
			{
				sum += importTimes.at(i);
			}

			qDebug() << "Average import time:" << sum / (float) importTimes.size();
		}
	}
	else
	{
		qWarning("Cannot import media without valid table model!");
	}
}

void MediaManagerPrivate::_q_modelDestroyed()
{
	model = NULL;
}

void MediaManagerPrivate::_q_modelBeforeUpdate(int row, QSqlRecord &record)
{
	Q_UNUSED(row);

	TagLib::FileRef taggedFile(record.value("path").toString().toUtf8().constData());

	if (writeTagsToFile)
	{
		saveTagsToFile(taggedFile, record);
	}

	if (organizeLibrary && organizeFile(taggedFile))
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

////////// END PRIVATE DATA //////////

MediaManager::MediaManager(QObject *parent) :
   QObject(parent),
   d_ptr(new MediaManagerPrivate)
{
	Q_D(MediaManager);

	d->q_ptr = this;

	connect(&d->fileSystemWatcher, SIGNAL(directoryChanged(QString)), SLOT(_q_directoryChanged(QString)));
}

void MediaManager::setModel(QSqlTableModel *model)
{
	Q_D(MediaManager);

	if (d->model)
	{
		disconnect(d->model, SIGNAL(destroyed()), this, SLOT(_q_modelDestroyed()));
		disconnect(d->model, SIGNAL(beforeUpdate(int,QSqlRecord&)), this, SLOT(_q_modelBeforeUpdate(int,QSqlRecord&)));
	}

	if (model)
	{
		d->model = model;
		connect(d->model, SIGNAL(destroyed()), SLOT(_q_modelDestroyed()));
		connect(d->model, SIGNAL(beforeUpdate(int,QSqlRecord&)), SLOT(_q_modelBeforeUpdate(int,QSqlRecord&)));
	}
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

bool MediaManager::beginImportMedia()
{
	Q_D(MediaManager);

	if (!d->importing)
	{
		d->importing = true;

		d->watchedItemsBackup = d->fileSystemWatcher.directories() + d->fileSystemWatcher.files();

		// Temporarily disable the filesystem watcher since an import is about to happen
		// and we'll probably be manually changing the watched system. Don't need the updates.
		if (!d->watchedItemsBackup.isEmpty())
		{
			d->fileSystemWatcher.removePaths(d->watchedItemsBackup);
		}

		return true; // Ready to import
	}

	return false; // Already importing
}

void MediaManager::importMedia(const QFileInfo &fileInfo)
{
	Q_D(MediaManager);

	bool beginWasNotCalled = beginImportMedia();

	d->recursivelyImportMedia(fileInfo);

	// If begin wasn't called, we'll need to call end as well
	if (beginWasNotCalled)
	{
		endImportMedia();
	}
}

void MediaManager::endImportMedia()
{
	Q_D(MediaManager);

	// Done importing-- we can re-enable the file system watcher
	// and add the newly imported media to the watch list
	if (!d->watchedItemsBackup.isEmpty())
	{
		d->fileSystemWatcher.addPaths(d->watchedItemsBackup);
		d->watchedItemsBackup.clear(); // Get rid of the backup
		d->updateWatchedPaths();
	}

	d->importing = false;
}

void MediaManager::setCopyOnImport(bool enabled)
{
	Q_D(MediaManager);

	d->copyOnImport = enabled;
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
		d->fileSystemWatcher.removePaths(d->fileSystemWatcher.directories() + d->fileSystemWatcher.files());
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
