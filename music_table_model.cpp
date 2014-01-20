#include <QDebug>

#include "music_table_item.h"
#include "music_table_model.h"

MusicTableModel::MusicTableModel(QObject *parent) :
   QAbstractTableModel(parent)
{
}

int MusicTableModel::rowCount(const QModelIndex &parent) const
{
	Q_UNUSED(parent);

	return m_music.count();
}

int MusicTableModel::columnCount(const QModelIndex &parent) const
{
	Q_UNUSED(parent);

	return 1;
}

QVariant MusicTableModel::data(const QModelIndex &index, int role) const
{
	int row = index.row();

	if ((row >= 0) && (row < m_music.size()))
	{
		switch (role)
		{
			case MusicTableItem::PathRole:
				return m_music.at(index.row())->path();
			case Qt::DisplayRole:
			case MusicTableItem::FileNameRole:
				return m_music.at(index.row())->fileName();
		}
	}

	return QVariant();
}

QVariant MusicTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	Q_UNUSED(orientation);
	if ((section == 0) && role == Qt::DisplayRole)
	{
		return tr("File");
	}

	return QVariant();
}

bool MusicTableModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
	if (index.isValid() && (role == Qt::EditRole))
	{
		m_music.at(index.row())->setFilePath(value.toString());

		emit dataChanged(index, index);

		return true;
	}

	return false;
}

bool MusicTableModel::appendRow(MusicTableItem *item)
{
	return appendRows(QList<MusicTableItem*>() << item);
}

bool MusicTableModel::appendRows(const QList<MusicTableItem *> &items)
{
	beginInsertRows(QModelIndex(), rowCount(), rowCount() + items.size());

	foreach (MusicTableItem *item, items)
	{
		item->setParent(this);

		m_music.append(item);
	}

	endInsertRows();

	return true;
}

bool MusicTableModel::insertRow(int row, const QModelIndex &parent)
{
	return insertRows(row, 1, parent);
}

bool MusicTableModel::insertRows(int row, int count, const QModelIndex &parent)
{
	Q_UNUSED(parent);

	beginInsertRows(QModelIndex(), row, row+count-1);

	for (int i = 0; i < count; i++)
	{
		m_music.insert(row, new MusicTableItem(this));
	}

	endInsertRows();

	return true;
}

bool MusicTableModel::removeRow(int row, const QModelIndex &parent)
{
	Q_UNUSED(parent);

	if ((row >= 0) && (row < m_music.size()))
	{
		beginRemoveRows(QModelIndex(), row, row);

		delete m_music.takeAt(row);

		endRemoveRows();

		return true;
	}

	return false;
}

bool MusicTableModel::removeRows(int row, int count, const QModelIndex &parent)
{
	Q_UNUSED(parent);

	if ((row >= 0) && ((row+count) <= m_music.size()))
	{
		beginRemoveRows(QModelIndex(), row, row+count-1);

		for (int i = 0; i < count; i++)
		{
			delete m_music.takeAt(row);
		}

		endRemoveRows();

		return true;
	}

	return false;
}

void MusicTableModel::clear()
{
	(void) removeRows(0, rowCount());
}

Qt::ItemFlags MusicTableModel::flags(const QModelIndex &index) const
{
	Q_UNUSED(index);

	return Qt::ItemIsSelectable |  Qt::ItemIsEditable | Qt::ItemIsEnabled ;
}
