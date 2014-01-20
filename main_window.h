#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <QMainWindow>
#include <QFileInfo>
#include <QUrl>

namespace Ui
{
	class MainWindow;
}

class QMediaPlayer;
class MusicFileTypeResolver;
class QSqlTableModel;

class MainWindow : public QMainWindow
{
	Q_OBJECT

	public:
		explicit MainWindow(QWidget *parent = 0);
		~MainWindow();

	private slots:
		void handleSongDoubleClicked(const QModelIndex &index);
		void playSong(const QUrl &fileUrl);
		void handleImport(const QList<QUrl> &itemsToImport);

		void on_playButton_clicked();
		void on_pauseButton_clicked();

	private:
		void recursivelyImportMusic(const QString &path);
		void recursivelyImportMusic(const QFileInfo &fileInfo);
		bool isSupported(const QString& mimeType, const QStringList &codecs = QStringList());

	private:
		Ui::MainWindow *m_ui;
		QMediaPlayer *m_mediaPlayer;
		MusicFileTypeResolver *m_fileTypeResolver;
		QSqlTableModel *m_musicTableModel;
};

#endif // MAIN_WINDOW_H
