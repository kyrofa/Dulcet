#include <QString>
#include <QMimeType>
#include <QFileInfo>
#include <QSqlRecord>
#include <QVariant>
#include <QDir>
#include <QDebug>

#include <taglib/fileref.h>

#include "dulcet_utilities.h"

using namespace DulcetUtilities;

// Neither the directory nor the file need to exist
bool DulcetUtilities::isDirectoryAncestorOfFile(const QDir &directory, const QFileInfo& file)
{
	return file.absolutePath().contains(directory.absolutePath());
}

bool DulcetUtilities::saveTagsToFile(TagLib::FileRef &file, const QSqlRecord &record)
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

bool DulcetUtilities::importTagsToRecord(const TagLib::FileRef &file, QSqlRecord &record)
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

bool DulcetUtilities::generateAnticipatedPath(const QDir &libraryDirectory, const TagLib::FileRef &file,
                                              const QMimeType &mimeType, QString &path, QString &fileName,
                                              QString &fileSuffix)

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

	path = libraryDirectory.absolutePath() + "/";
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
	fileSuffix = mimeType.preferredSuffix();

	return true;
}

bool DulcetUtilities::organizeFile(const QDir &libraryDirectory, TagLib::FileRef &file,
                                   const QMimeType &mimeType, bool copyOnImport)
{
	QString path, fileName, fileSuffix;
	if (generateAnticipatedPath(libraryDirectory, file, mimeType, path, fileName, fileSuffix))
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
			if (QFileInfo::exists(newPath) || libraryDirectory.mkpath(newPath))
			{
				bool success = false;

				if (copyOnImport && !isDirectoryAncestorOfFile(libraryDirectory, oldFileInfo))
				{
					success = oldFile.copy(newFilePath);
				}
				else
				{
					success = oldFile.rename(newFilePath);
				}

				// If the reorganization succeeded, update the fileref
				// to point to the new location
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
