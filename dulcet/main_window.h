#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <QMainWindow>
#include <QMediaPlayer>
#include <QFileInfo>
#include <QPersistentModelIndex>
#include <QSqlRecord>
#include <QMap>
#include <QDir>
#include <QUrl>

namespace Ui
{
	class MainWindow;
}

class MusicFileTypeResolver;
class MediaManager;
class SortMultiFilterProxyModel;
class MusicViewInterface;

class MainWindow : public QMainWindow
{
	Q_OBJECT

	public:
		explicit MainWindow(QWidget *parent = 0);
		~MainWindow();

	protected slots:
		virtual void closeEvent(QCloseEvent *event);

	private slots:
		void playSong(const QModelIndex &index);
		void handleImport(const QList<QUrl> &itemsToImport);
		void handleImportedMedia(int row);
		void handleMediaDeleted(const QSqlRecord &recordRemoved);
		void handlePlayerStateChange(QMediaPlayer::State newState);
		void handlePlayerStatusChange(QMediaPlayer::MediaStatus status);
		void handleSearch(const QString &searchString);

		void handleMusicViewChange(QAction *action);
		void handleColumnVisibilitiesChanged(const QStringList &visibleColumnNames);
		void showImportDialog();
		void showPreferences();

		void on_playPauseButton_clicked();
		void on_previousButton_clicked();
		void on_nextButton_clicked();

	private:
		void showMusicView(const QString &musicView);
		bool loadPlugins();
		void saveSettings();
		void loadSettings();

	signals:
		void playing(const QPersistentModelIndex& index);

	private:
		typedef QMap<QString, MusicViewInterface*> MusicViewMap_t;
		Ui::MainWindow *m_ui;
		QMediaPlayer *m_mediaPlayer;
		MediaManager *m_mediaManager;
		MusicFileTypeResolver *m_fileTypeResolver;
		SortMultiFilterProxyModel *m_proxyModel;
		MusicViewMap_t m_musicViews;
		MusicViewInterface *m_currentMusicView;
};

#endif // MAIN_WINDOW_H
