#ifndef FILE_LIST_ITEM_DELEGATE_H
#define FILE_LIST_ITEM_DELEGATE_H

#include <QStyledItemDelegate>

class MusicTableItemDelegate : public QStyledItemDelegate
{
	Q_OBJECT

	public:
		explicit MusicTableItemDelegate(QObject *parent = 0);

		void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
		QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;
};

#endif // FILE_LIST_ITEM_DELEGATE_H
