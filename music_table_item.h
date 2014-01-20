#ifndef MUSIC_TABLE_ITEM_H
#define MUSIC_TABLE_ITEM_H

#include <QObject>

class MusicTableItem : public QObject
{
	Q_OBJECT

	public:
		enum Roles_t
		{
			PathRole = Qt::UserRole+1,
			FileNameRole
		};

		explicit MusicTableItem(QObject *parent = 0);

		void setFilePath(const QString& filePath);

		const QString& path() const;
		const QString& fileName() const;

		QVariant data(Roles_t role) const;
		QHash<int, QByteArray> roleNames() const;

	private:
		QString m_path;
		QString m_fileName;
};

#endif // MUSIC_TABLE_ITEM_H
