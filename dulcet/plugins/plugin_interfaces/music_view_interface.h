#ifndef MUSIC_VIEW_INTERFACE_H
#define MUSIC_VIEW_INTERFACE_H

#include <QAbstractItemView>

#include "dulcet_plugin_interface.h"

class Q_DECL_EXPORT MusicViewInterface : public DulcetPluginInterface
{
	Q_OBJECT

	public:
		Q_DECL_EXPORT explicit MusicViewInterface(QObject *parent = 0) : DulcetPluginInterface(parent) {}
		Q_DECL_EXPORT virtual ~MusicViewInterface() {}

		Q_DECL_EXPORT virtual QAbstractItemView* view() = 0;
		Q_DECL_EXPORT virtual void setModel(QAbstractItemModel *model) = 0;
		Q_DECL_EXPORT virtual QStringList visibleColumns() = 0;

		Q_DECL_EXPORT virtual QModelIndex nextSong() = 0;
		Q_DECL_EXPORT virtual QModelIndex previousSong() = 0;

		Q_DECL_EXPORT virtual void setShuffleEnabled(bool enabled) = 0;

	public slots:
		Q_DECL_EXPORT virtual void handlePlaying(const QPersistentModelIndex& index) = 0;

	signals:
		Q_DECL_EXPORT void playSong(const QModelIndex &index);
		Q_DECL_EXPORT void columnVisibilitiesChanged(const QStringList &visibleColumnNames);
};

Q_DECLARE_INTERFACE(MusicViewInterface, "dulcet.plugins.MusicViewInterface")

#endif // MUSIC_VIEW_INTERFACE_H
