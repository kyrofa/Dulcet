#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <QMainWindow>
#include <QMediaPlayer>
#include <QFileInfo>
#include <QStack>
#include <QPersistentModelIndex>
#include <QUrl>

namespace Ui
{
	class MainWindow;
}

class MusicFileTypeResolver;
class QSqlTableModel;
class QSortFilterProxyModel;

class MainWindow : public QMainWindow
{
	Q_OBJECT

	public:
		explicit MainWindow(QWidget *parent = 0);
		~MainWindow();

	private slots:
		void playSong(const QModelIndex &index, bool saveHistory = true);
		void playSong(const QUrl &fileUrl);
		void handleImport(const QList<QUrl> &itemsToImport);
		void handlePlayerStateChange(QMediaPlayer::State newState);
		void handleSearch(const QString &searchString);

		void on_playPauseButton_clicked();
		void on_previousButton_clicked();
		void on_nextButton_clicked();

	private:
		void recursivelyImportMusic(const QString &path);
		void recursivelyImportMusic(const QFileInfo &fileInfo);
		bool isSupported(const QString& mimeType, const QStringList &codecs = QStringList());

	private:
		Ui::MainWindow *m_ui;
		QMediaPlayer *m_mediaPlayer;
		MusicFileTypeResolver *m_fileTypeResolver;
		QSqlTableModel *m_musicTableModel;
		QStack<QPersistentModelIndex> m_playHistory;
		QPersistentModelIndex m_currentSong;
		QSortFilterProxyModel *m_proxyModel;
};

#endif // MAIN_WINDOW_H
