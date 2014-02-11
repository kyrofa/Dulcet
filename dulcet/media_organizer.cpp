#include <QMediaPlayer>
#include <QThread>
#include <QMutex>
#include <QSqlTableModel>
#include <QSqlRecord>
#include <QFileInfo>
#include <QDir>
#include <QUrl>

#include "dulcet_utilities.h"
#include "music_file_type_resolver.h"
#include "media_organizer.h"

////////// BEGIN PRIVATE DATA //////////

class MediaOrganizerPrivate
{
	Q_DECLARE_PUBLIC(MediaOrganizer)

	public:
		MediaOrganizerPrivate();

		virtual void importMedia(const QFileInfo &fileInfo);
		virtual bool nextItemToOrganize(QFileInfo &item);

		MediaOrganizer *q_ptr;
		MusicFileTypeResolver fileTypeResolver;

		mutable QMutex itemsToOrganizeMutex;
		QStringList itemsToOrganize;

		mutable QMutex libraryDirectoryMutex;
		QDir libraryDirectory;

		mutable QMutex copyOnImportMutex;
		bool copyOnImport;

		mutable QMutex organizeLibraryMutex;
		bool organizeLibrary;

		bool importing;
};

MediaOrganizerPrivate::MediaOrganizerPrivate() :
   q_ptr(NULL),
   copyOnImport(true),
   organizeLibrary(false),
   importing(false)
{
}

void MediaOrganizerPrivate::importMedia(const QFileInfo &fileInfo)
{
	Q_Q(MediaOrganizer);

	QString filePath = fileInfo.absoluteFilePath();
	QByteArray filePathArray = filePath.toUtf8();
	const char *filePathData = filePathArray.constData();
	TagLib::FileRef taggedFile(filePathData);

	if (q->organizeLibrary() &&
		 !DulcetUtilities::organizeFile(libraryDirectory,
												  taggedFile,
												  fileTypeResolver.mimeTypeForFile(filePath),
												  q->copyOnImport()))
	{
		qWarning("Unable to organize %s. Skipping...", filePathData);
		return;
	}

	emit q->organizedMedia(taggedFile);
}

bool MediaOrganizerPrivate::nextItemToOrganize(QFileInfo &item)
{
	QMutexLocker locker(&itemsToOrganizeMutex);

	if (!itemsToOrganize.isEmpty())
	{
		item = itemsToOrganize.first();
		itemsToOrganize.removeFirst();

		return true;
	}

	return false;
}

////////// END PRIVATE DATA //////////

MediaOrganizer::MediaOrganizer(QObject *parent) :
   QObject(parent),
   d_ptr(new MediaOrganizerPrivate)
{
	Q_D(MediaOrganizer);

	d->q_ptr = this;
}

MediaOrganizer::~MediaOrganizer()
{
}

void MediaOrganizer::addItemsToOrganize(const QStringList &itemsToOrganize)
{
	Q_D(MediaOrganizer);

	QMutexLocker locker(&d->itemsToOrganizeMutex);

	d->itemsToOrganize.append(itemsToOrganize);
}

void MediaOrganizer::setLibraryDirectory(const QDir &libraryDirectory)
{
	Q_D(MediaOrganizer);

	QMutexLocker locker(&d->libraryDirectoryMutex);

	d->libraryDirectory = libraryDirectory;
}

QDir MediaOrganizer::libraryDirectory() const
{
	Q_D(const MediaOrganizer);

	QMutexLocker locker(&d->libraryDirectoryMutex);

	return d->libraryDirectory;
}

void MediaOrganizer::setCopyOnImport(bool enabled)
{
	Q_D(MediaOrganizer);

	QMutexLocker locker(&d->copyOnImportMutex);

	d->copyOnImport = enabled;
}

bool MediaOrganizer::copyOnImport() const
{
	Q_D(const MediaOrganizer);

	QMutexLocker locker(&d->copyOnImportMutex);

	return d->copyOnImport;
}

void MediaOrganizer::setOrganizeLibrary(bool enabled)
{
	Q_D(MediaOrganizer);

	QMutexLocker locker(&d->organizeLibraryMutex);

	d->organizeLibrary = enabled;
}

bool MediaOrganizer::organizeLibrary() const
{
	Q_D(const MediaOrganizer);

	QMutexLocker locker(&d->organizeLibraryMutex);

	return d->organizeLibrary;
}

void MediaOrganizer::beginOrganization()
{
	Q_D(MediaOrganizer);

	if (!d->importing)
	{
		d->importing = true;

		QFileInfo fileInfo;
		int count = 0;

		while (d->nextItemToOrganize(fileInfo))
		{
			d->importMedia(fileInfo);

			count++;

			// Check for interruption every ten files (or folders)
			if (((count % 10) == 0) && thread()->isInterruptionRequested())
			{
				qDebug("Interrupt requested! Stopping media organization.");
				break;
			}
		}

		d->importing = false;

		emit finishedOrganization();
	}
}

#include "moc_media_organizer.cpp"
