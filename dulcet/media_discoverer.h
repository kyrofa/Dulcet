#ifndef MEDIA_DISCOVERER_H
#define MEDIA_DISCOVERER_H

#include <QObject>

class MediaDiscovererPrivate;
class QFileInfo;

class MediaDiscoverer : public QObject
{
	Q_OBJECT

	signals:
		void discoveredMedia(const QStringList &mediaFilePaths);
		void beginningDiscovery();
		void finishedDiscovery();

	public:
		explicit MediaDiscoverer(QObject *parent = 0);
		~MediaDiscoverer();

		void addSearchItems(const QList<QFileInfo> &itemsToSearch);

	public slots:
		void beginDiscovery();

	protected:
		QScopedPointer<MediaDiscovererPrivate> d_ptr;

	private:
		Q_DECLARE_PRIVATE(MediaDiscoverer)
};

#endif // MEDIA_DISCOVERER_H
