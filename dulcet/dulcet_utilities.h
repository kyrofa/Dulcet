#ifndef DULCET_UTILITIES_H
#define DULCET_UTILITIES_H

class QFileInfo;
class QDir;
class QSqlRecord;
class QMimeType;
class QString;

namespace TagLib
{
	class FileRef;
}

namespace DulcetUtilities
{
	// Neither the directory nor the file need to exist
	bool isDirectoryAncestorOfFile(const QDir &directory, const QFileInfo& file);

	bool saveTagsToFile(TagLib::FileRef &file, const QSqlRecord &record);

	bool importTagsToRecord(const TagLib::FileRef &file, QSqlRecord &record);

	bool generateAnticipatedPath(const QDir &libraryDirectory, const TagLib::FileRef &file,
	                             const QMimeType &mimeType, QString &path, QString &fileName,
	                             QString &fileSuffix);

	bool organizeFile(const QDir &libraryDirectory, TagLib::FileRef &file,
	                  const QMimeType &mimeType, bool copyOnImport = true);
}

#endif // DULCET_UTILITIES_H
