#ifndef DROPPABLE_TABLE_VIEW_H
#define DROPPABLE_TABLE_VIEW_H

#include <QTableView>
#include <QFileInfo>
#include <QUrl>

class DroppableWidget : public QWidget
{
	Q_OBJECT

	public:
		explicit DroppableWidget(QWidget *parent = 0);
		~DroppableWidget();

	protected:
		void dragEnterEvent(QDragEnterEvent *event);
		void dragMoveEvent(QDragMoveEvent *event);
		void dragLeaveEvent(QDragLeaveEvent *event);
		void dropEvent(QDropEvent *event);

	signals:
		void requestImport(const QList<QUrl> &itemsToImport);
};

#endif // DROPPABLE_TABLE_VIEW_H
