#ifndef MEDIA_ORGANIZER_H
#define MEDIA_ORGANIZER_H

#include <QObject>

class MediaOrganizerPrivate;
class QSqlDatabase;
class QSqlRecord;
class QDir;
class QFileInfo;

namespace TagLib
{
	class FileRef;
}

class MediaOrganizer : public QObject
{
	Q_OBJECT

	public:
		explicit MediaOrganizer(QObject *parent = 0);
		~MediaOrganizer();

		void addItemsToOrganize(const QStringList &itemsToOrganize);

		void setLibraryDirectory(const QDir &libraryDirectory);
		QDir libraryDirectory() const;

		void setCopyOnImport(bool enabled);
		bool copyOnImport() const;

		void setOrganizeLibrary(bool enabled);
		bool organizeLibrary() const;

	public slots:
		void beginOrganization();

	signals:
		void beginningOrganization();
		void organizedMedia(const TagLib::FileRef &taggedFile);
		void finishedOrganization();

	protected:
		QScopedPointer<MediaOrganizerPrivate> d_ptr;

	private:
		Q_DECLARE_PRIVATE(MediaOrganizer)
};

#endif // MEDIA_ORGANIZER_H
