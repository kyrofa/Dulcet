// Qt includes
#include <QDebug>
#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QDragLeaveEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QMediaPlayer>
#include <QDir>

// Local includes
#include "droppable_table_view.h"

DroppableTableView::DroppableTableView(QWidget *parent) :
   QTableView(parent)
{
	setAcceptDrops(true);
}

DroppableTableView::~DroppableTableView()
{
}

void DroppableTableView::dragEnterEvent(QDragEnterEvent *event)
{
	event->acceptProposedAction();
}

void DroppableTableView::dragMoveEvent(QDragMoveEvent *event)
{
	event->acceptProposedAction();
}

void DroppableTableView::dragLeaveEvent(QDragLeaveEvent *event)
{
	event->accept();
}

void DroppableTableView::dropEvent(QDropEvent *event)
{
	const QMimeData *mimeData = event->mimeData();

	if (mimeData->hasUrls())
	{
		emit requestImport(mimeData->urls());
	}
	else
	{
		qWarning("WARNING: Cannot display data");
	}

	event->acceptProposedAction();
}
