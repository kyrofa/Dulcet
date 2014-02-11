#include "sort_multi_filter_proxy_model.h"

SortMultiFilterProxyModel::SortMultiFilterProxyModel(QObject *parent) :
   QSortFilterProxyModel(parent)
{
}

SortMultiFilterProxyModel::~SortMultiFilterProxyModel()
{
}

void SortMultiFilterProxyModel::setFilterKeyColumns(const QVector<int> &columnIndices)
{
	m_keyColumns.clear();

	foreach (int thisColumn, columnIndices)
	{
		// If this is a valid column
		if ((thisColumn >= 0) && (thisColumn < sourceModel()->columnCount()))
		{
			m_keyColumns.append(thisColumn);
		}
	}

	invalidateFilter(); // Refresh filter
}

bool SortMultiFilterProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
	if (!m_keyColumns.isEmpty()) // Are we even taking advantage of multi-filter?
	{
		QRegExp filter = filterRegExp();

		if (!filter.isEmpty())
		{
			QAbstractItemModel *model = sourceModel();

			foreach (int thisColumn, m_keyColumns)
			{
				QModelIndex sourceIndex = model->index(sourceRow, thisColumn, sourceParent);
				QString key = model->data(sourceIndex, filterRole()).toString();
				if (key.contains(filter))
				{
					return true;
				}
			}

			return false; // No matches
		}

		return true;
	}
	else
	{
		return QSortFilterProxyModel::filterAcceptsRow(sourceRow, sourceParent);
	}
}
