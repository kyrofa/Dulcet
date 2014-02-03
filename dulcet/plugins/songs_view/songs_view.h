#ifndef SONGS_VIEW_H
#define SONGS_VIEW_H

#include <QTableView>
#include <QStack>

#include "plugin_interfaces/music_view_interface.h"

class SongsView : public MusicViewInterface
{
	Q_OBJECT
	Q_PLUGIN_METADATA(IID "dulcet.plugins.MusicViewInterface")
	Q_INTERFACES(MusicViewInterface)

	public:
		explicit SongsView(QObject *parent = 0);
		virtual ~SongsView();

		virtual QAbstractItemView* view();
		virtual void setModel(QAbstractItemModel *model);

		virtual QModelIndex nextSong();
		virtual QModelIndex previousSong();

		virtual void setShuffleEnabled(bool enabled);

		virtual void saveSettings(QSettings &settings);
		virtual void loadSettings(const QSettings &settings);

	public slots:
		virtual void handlePlaying(const QPersistentModelIndex& index);

	private:
		QTableView *m_tableView;
		QList<QPersistentModelIndex> m_shuffleHistory;
		QPersistentModelIndex m_currentSong;
		bool m_shuffleEnabled;
};

#endif // SONGS_VIEW_H
