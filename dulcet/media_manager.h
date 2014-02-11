#ifndef MEDIA_MANAGER_H
#define MEDIA_MANAGER_H

#include <QObject>

class QSqlTableModel;
class QSqlRecord;
class QFileInfo;
class MediaManagerPrivate;

namespace TagLib
{
	class FileRef;
}

class MediaManager : public QObject
{
	Q_OBJECT

	public:
		explicit MediaManager(QObject *parent = 0);
		~MediaManager();

		bool setDatabase(const QString &adapterType, const QString &databaseFilePath);
		QSqlTableModel *model();

		bool isImporting() const;
		void cancelImport(bool shutdown = false);

		bool setMusicDirectory(const QDir &directory, bool reindex = true);
		const QDir& musicDirectory() const;

		void setCopyOnImport(bool enabled);
		bool copyOnImport() const;

		void setOrganizeLibrary(bool enabled);
		bool organizeLibrary() const;

		void setWatchLibrary(bool enabled, bool reindex = true);
		bool watchLibrary() const;

		void setWriteTagsToFile(bool enabled);
		bool writeTagsToFile() const;

		bool isSupported(const QString& mimeType, const QStringList &codecs = QStringList()) const;

	public slots:
		void importMedia(const QFileInfo &fileInfo);
		void importMedia(const QList<QFileInfo> &itemsToImport);

	protected:
		QScopedPointer<MediaManagerPrivate> d_ptr;

	signals:
		void importedMedia(int row);
		void deletedMedia(const QSqlRecord &record);

	private:
		Q_DECLARE_PRIVATE(MediaManager)

		Q_PRIVATE_SLOT(d_func(), void _q_mediaDiscovered(const QStringList&))
		Q_PRIVATE_SLOT(d_func(), void _q_mediaOrganized(const TagLib::FileRef&))
		Q_PRIVATE_SLOT(d_func(), void _q_modelBeforeUpdate(int, QSqlRecord&))
		Q_PRIVATE_SLOT(d_func(), void _q_directoryChanged(const QString&))
		Q_PRIVATE_SLOT(d_func(), void _q_endImportMedia())
};

#endif // MEDIA_MANAGER_H
