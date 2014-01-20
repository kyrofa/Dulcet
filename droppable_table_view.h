#ifndef DROPPABLE_TABLE_VIEW_H
#define DROPPABLE_TABLE_VIEW_H

#include <QTableView>
#include <QFileInfo>

class QMediaPlayer;
class QMimeDatabase;

class DroppableTableView : public QTableView
{
	Q_OBJECT

	public:
		explicit DroppableTableView(QWidget *parent = 0);
		~DroppableTableView();

	protected:
		void dragEnterEvent(QDragEnterEvent *event);
		void dragMoveEvent(QDragMoveEvent *event);
		void dragLeaveEvent(QDragLeaveEvent *event);
		void dropEvent(QDropEvent *event);

		void recursivelyImportMusic(const QString &path);
		void recursivelyImportMusic(const QFileInfo &fileInfo);
		bool isSupported(const QString& mimeType, const QStringList &codecs = QStringList());

	signals:
		void importStatusUpdate(const QString &status);

	private:
		QMediaPlayer *m_mediaPlayer;
		QMimeDatabase *m_mimeDatabase;
};

#endif // DROPPABLE_TABLE_VIEW_H
