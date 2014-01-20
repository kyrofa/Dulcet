#ifndef MUSIC_TABLE_MODEL_H
#define MUSIC_TABLE_MODEL_H

#include <QAbstractTableModel>

class MusicTableItem;

class MusicTableModel : public QAbstractTableModel
{
	Q_OBJECT

	public:
		explicit MusicTableModel(QObject *parent = 0);

		int rowCount(const QModelIndex &parent = QModelIndex()) const;
		int columnCount(const QModelIndex &parent) const;
		QVariant data(const QModelIndex &index, int role) const;
		QVariant headerData(int section, Qt::Orientation orientation, int role) const;

		bool setData(const QModelIndex &index, const QVariant &value, int role);
		bool appendRow(MusicTableItem* item);
		bool appendRows(const QList<MusicTableItem*> &items);
		bool insertRow(int row, const QModelIndex &parent = QModelIndex());
		bool insertRows(int row, int count, const QModelIndex &parent = QModelIndex());
		bool removeRow(int row, const QModelIndex &parent = QModelIndex());
		bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex());
		void clear();
		Qt::ItemFlags flags(const QModelIndex &index) const;

	private:
		QList<MusicTableItem*> m_music;
};

#endif // MUSIC_TABLE_MODEL_H
