#include <QPainter>

#include "music_table_item_delegate.h"

#define ICON_SIZE 32
#define FONT_SIZE 12

MusicTableItemDelegate::MusicTableItemDelegate(QObject *parent) :
   QStyledItemDelegate(parent)
{
}

void MusicTableItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	QColor background;

	if (option.state & QStyle::State_Selected)
	{
		background = Qt::darkGray;
	}
	else
	{
		background = Qt::lightGray;
	}

//	QPixmap icon = index.model()->data(index, FileListItem::IconRole).value<QIcon>().pixmap(ICON_SIZE, ICON_SIZE);
//	painter->drawPixmap(option.rect.x(), option.rect.y(), icon);

	painter->setFont(QFont("Arial", FONT_SIZE));
	painter->drawText(option.rect.adjusted(ICON_SIZE+5, 0, 0, 0), Qt::AlignLeft | Qt::AlignVCenter, index.model()->data(index).toString());
}

QSize MusicTableItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	Q_UNUSED(option);
	Q_UNUSED(index);

	return QSize(100, 32);
}
