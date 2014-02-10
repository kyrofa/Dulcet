#include <QUrl>
#include <QDirIterator>
#include <QMediaPlayer>
#include <QThread>
#include <QMutex>

#include "music_file_type_resolver.h"
#include "media_discoverer.h"

////////// BEGIN PRIVATE DATA //////////

class MediaDiscovererPrivate
{
	Q_DECLARE_PUBLIC(MediaDiscoverer)

	public:
		MediaDiscovererPrivate();

		virtual bool isSupported(const QString &mimeType, const QStringList &codecs = QStringList());
		virtual bool nextItemToSearch(QFileInfo &item);

		MediaDiscoverer *q_ptr;

		QStringList discoveredMedia;

		QMutex searchItemsMutex;
		QList<QFileInfo> searchItems;

		MusicFileTypeResolver fileTypeResolver;
		QStringList supportedMimeTypeCache;

		bool searchInProgress;
};

MediaDiscovererPrivate::MediaDiscovererPrivate() :
   q_ptr(NULL),
   searchInProgress(false)
{
}

bool MediaDiscovererPrivate::isSupported(const QString &mimeType, const QStringList &codecs)
{
	if (supportedMimeTypeCache.contains(mimeType))
	{
		return true;
	}
	else
	{
		// QMediaPlayer::hasSupport() is buggy right now... it just always returns MaybeSupported.
		// So we'll supplement it here by making sure it's either video or audio (thus fairly likely
		// to play successfully) and then check for support (in case it's fixed in the future).
		if (mimeType.startsWith("audio/") || mimeType.startsWith("video/"))
		{
			if (QMediaPlayer::hasSupport(mimeType, codecs) >= QMultimedia::MaybeSupported)
			{
				// Save this in the cache so we don't need to check again
				supportedMimeTypeCache.append(mimeType);
				return true;
			}
		}

		return false;
	}
}

bool MediaDiscovererPrivate::nextItemToSearch(QFileInfo &item)
{
	QMutexLocker locker(&searchItemsMutex);

	if (!searchItems.isEmpty())
	{
		item = searchItems.first();
		searchItems.removeFirst();

		return true;
	}

	return false;
}

////////// END PRIVATE DATA //////////

MediaDiscoverer::MediaDiscoverer(QObject *parent) :
   QObject(parent),
   d_ptr(new MediaDiscovererPrivate)
{
	Q_D(MediaDiscoverer);

	d->q_ptr = this;
}

MediaDiscoverer::~MediaDiscoverer()
{
}

void MediaDiscoverer::addSearchItems(const QList<QFileInfo> &itemsToSearch)
{
	Q_D(MediaDiscoverer);

	QMutexLocker locker(&d->searchItemsMutex);

	d->searchItems.append(itemsToSearch);
}

void MediaDiscoverer::beginDiscovery()
{
	Q_D(MediaDiscoverer);

	if (!d->searchInProgress)
	{
		d->searchInProgress = true;

		emit beginningDiscovery();

		QFileInfo fileInfo;
		int count = 0;

		while (d->nextItemToSearch(fileInfo))
		{
			if (fileInfo.isFile())
			{
				d->discoveredMedia.append(fileInfo.absoluteFilePath());
			}
			else
			{
				QDirIterator iterator(fileInfo.absoluteFilePath(), QDirIterator::Subdirectories);

				while (iterator.hasNext())
				{
					iterator.next();

					fileInfo = iterator.fileInfo();

					if (fileInfo.isFile() && d->isSupported(d->fileTypeResolver.mimeTypeForFile(fileInfo).name()))
					{
						d->discoveredMedia.append(fileInfo.absoluteFilePath());
					}

					count++;

					// Check for interruption every ten files (or folders)
					if (((count % 10) == 0) && thread()->isInterruptionRequested())
					{
						qDebug() << "Interrupt requested! Stopping media discovery.";

						d->searchInProgress = false;

						if (!d->discoveredMedia.isEmpty())
						{
							emit discoveredMedia(d->discoveredMedia);
						}

						return;
					}
				}
			}
		}

		d->searchInProgress = false;

		if (!d->discoveredMedia.isEmpty())
		{
			emit discoveredMedia(d->discoveredMedia);
		}

		emit finishedDiscovery();
	}
}

#include "moc_media_discoverer.cpp"
