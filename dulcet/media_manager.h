#ifndef MEDIA_MANAGER_H
#define MEDIA_MANAGER_H

#include <QObject>

class QSqlTableModel;
class QSqlRecord;
class QFileInfo;
class MediaManagerPrivate;

class MediaManager : public QObject
{
	Q_OBJECT

	public:
		explicit MediaManager(QObject *parent = 0);

		void setModel(QSqlTableModel *model);
		bool setMusicDirectory(const QDir &directory, bool reindex = true);
		const QDir& musicDirectory() const;

		bool beginImportMedia();
		void importMedia(const QFileInfo &fileInfo);
		void endImportMedia();

		void setCopyOnImport(bool enabled);
		bool copyOnImport() const;

		void setOrganizeLibrary(bool enabled);
		bool organizeLibrary() const;

		void setWatchLibrary(bool enabled, bool reindex = true);
		bool watchLibrary() const;

		void setWriteTagsToFile(bool enabled);
		bool writeTagsToFile() const;

		bool isSupported(const QString& mimeType, const QStringList &codecs = QStringList()) const;

	protected:
		QScopedPointer<MediaManagerPrivate> d_ptr;

	signals:
		void importedMedia(int row, const QSqlRecord &record);
		void deletedMedia(const QSqlRecord &record);

	private:
		Q_DECLARE_PRIVATE(MediaManager)

		Q_PRIVATE_SLOT(d_func(), void _q_modelDestroyed())
		Q_PRIVATE_SLOT(d_func(), void _q_modelBeforeUpdate(int, QSqlRecord&))
		Q_PRIVATE_SLOT(d_func(), void _q_directoryChanged(const QString&))
};

#endif // MEDIA_MANAGER_H
