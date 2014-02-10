#ifndef SORT_MULTI_FILTER_PROXY_MODEL_H
#define SORT_MULTI_FILTER_PROXY_MODEL_H

#include <QSortFilterProxyModel>

class SortMultiFilterProxyModel : public QSortFilterProxyModel
{
	Q_OBJECT

	public:
		explicit SortMultiFilterProxyModel(QObject *parent = 0);
		virtual ~SortMultiFilterProxyModel();

		void setFilterKeyColumns(const QVector<int> &columnIndices);

	protected:
		virtual bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const;

	private:
		QVector<int> m_keyColumns;
};

#endif // SORT_MULTI_FILTER_PROXY_MODEL_H
