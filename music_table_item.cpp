#include <QVariant>
#include <QFileInfo>

#include "music_table_item.h"

MusicTableItem::MusicTableItem(QObject *parent) :
   QObject(parent)
{
}

void MusicTableItem::setFilePath(const QString& filePath)
{
	QFileInfo info(filePath);

	m_path = filePath;
	m_fileName = info.fileName();
}

const QString& MusicTableItem::path() const
{
	return m_path;
}

const QString& MusicTableItem::fileName() const
{
	return m_fileName;
}

QVariant MusicTableItem::data(Roles_t role) const
{
	switch (role)
	{
		case PathRole:
			return path();
		case FileNameRole:
			return fileName();
		default:
			return QVariant();
	}
}

QHash<int, QByteArray> MusicTableItem::roleNames() const
{
	QHash<int, QByteArray> roles;

	roles[PathRole] = "path";
	roles[FileNameRole] = "fileName";

	return roles;
}
