#include <QDebug>
#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QDragLeaveEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QMediaPlayer>
#include <QMimeDatabase>
#include <QDir>
#include <QUrl>

#include <taglib/fileref.h>

#include "droppable_table_view.h"

DroppableTableView::DroppableTableView(QWidget *parent) :
   QTableView(parent),
   m_mediaPlayer(new QMediaPlayer(this)),
   m_mimeDatabase(new QMimeDatabase)
{
	setAcceptDrops(true);
}

DroppableTableView::~DroppableTableView()
{
	delete m_mimeDatabase;
}

void DroppableTableView::dragEnterEvent(QDragEnterEvent *event)
{
	event->acceptProposedAction();
}

void DroppableTableView::dragMoveEvent(QDragMoveEvent *event)
{
	event->acceptProposedAction();
}

void DroppableTableView::dragLeaveEvent(QDragLeaveEvent *event)
{
	event->accept();
}

void DroppableTableView::dropEvent(QDropEvent *event)
{
	const QMimeData *mimeData = event->mimeData();

	if (mimeData->hasUrls())
	{
		QList<QUrl> urlList = mimeData->urls();

		// extract the local paths of the files
		for (int i = 0; i < urlList.size(); ++i)
		{
			recursivelyImportMusic(urlList.at(i).toLocalFile());
		}
	}
	else
	{
		qWarning("WARNING: Cannot display data");
	}

	event->acceptProposedAction();
}

void DroppableTableView::recursivelyImportMusic(const QString &path)
{
	recursivelyImportMusic(QFileInfo(path));
}

void DroppableTableView::recursivelyImportMusic(const QFileInfo &fileInfo)
{
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
		if (isSupported(m_mimeDatabase->mimeTypeForFile(fileInfo).name()))
		{
			int rowCount = model()->rowCount();
			QString filePath = fileInfo.absoluteFilePath();
			TagLib::FileRef fileTags(filePath.toUtf8().constData());

			model()->insertRows(rowCount, 1);
			model()->setData(model()->index(rowCount, 0),
			                 filePath);
			model()->setData(model()->index(rowCount, 1),
			                 fileTags.tag()->title().toCString());
			model()->setData(model()->index(rowCount, 2),
			                 fileTags.tag()->artist().toCString());
			model()->setData(model()->index(rowCount, 3),
			                 fileTags.tag()->album().toCString());
			model()->submit();

			//qDebug() << "Artist:" << f.tag()->artist().toCString(); // artist == "Frank Zappa"
//			f.tag()->setAlbum("Fillmore East");
//			f.save();
//			TagLib::FileRef g("Free City Rhymes.ogg");
//			TagLib::String album = g.tag()->album(); // album == "NYC Ghosts & Flowers"
//			g.tag()->setTrack(1);
//			g.save();


			emit importStatusUpdate(tr("Importing %1").arg(fileInfo.fileName()));

//			m_mediaPlayer->setMedia(QUrl::fromLocalFile(fileInfo.absoluteFilePath()));
//			m_mediaPlayer->setVolume(50);
//			m_mediaPlayer->play();
		}
	}
}

bool DroppableTableView::isSupported(const QString &mimeType, const QStringList &codecs)
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
