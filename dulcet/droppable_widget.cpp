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
#include "droppable_widget.h"

DroppableWidget::DroppableWidget(QWidget *parent) :
   QWidget(parent)
{
	setAcceptDrops(true);
}

DroppableWidget::~DroppableWidget()
{
}

void DroppableWidget::dragEnterEvent(QDragEnterEvent *event)
{
	event->acceptProposedAction();
}

void DroppableWidget::dragMoveEvent(QDragMoveEvent *event)
{
	event->acceptProposedAction();
}

void DroppableWidget::dragLeaveEvent(QDragLeaveEvent *event)
{
	event->accept();
}

void DroppableWidget::dropEvent(QDropEvent *event)
{
	const QMimeData *mimeData = event->mimeData();

	if (mimeData->hasUrls())
	{
		emit requestImport(mimeData->urls());
	}
	else
	{
		qWarning("WARNING: Cannot import music!");
	}

	event->acceptProposedAction();
}
