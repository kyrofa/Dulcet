#ifndef DROPPABLE_TABLE_VIEW_H
#define DROPPABLE_TABLE_VIEW_H

#include <QTableView>
#include <QFileInfo>
#include <QUrl>

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

	signals:
		void requestImport(const QList<QUrl> &itemsToImport);
};

#endif // DROPPABLE_TABLE_VIEW_H
