#include <QHeaderView>
#include <QScrollBar>

#include "songs_view.h"

SongsView::SongsView(QObject *parent) :
   MusicViewInterface(parent),
   m_tableView(new QTableView),
   m_shuffleEnabled(false)
{
	setObjectName("Songs View");

	connect(m_tableView, SIGNAL(doubleClicked(QModelIndex)), this, SIGNAL(playSong(QModelIndex)));
}

SongsView::~SongsView()
{
}

QAbstractItemView* SongsView::view()
{
	return m_tableView;
}

void SongsView::setModel(QAbstractItemModel *model)
{
	m_tableView->setModel(model);

	int columnCount = model->columnCount();
	for (int column = 0; column < columnCount; ++column)
	{
		m_columnNames[model->headerData(column, Qt::Horizontal).toString()] = column;
		m_tableView->hideColumn(column);
	}

	// These are the default visible columns-- the state is saved
	// in the settings later
	QStringList visibleColumnNames;
	visibleColumnNames << "title" << "artist" << "album";
	foreach (const QString &thisColumnName, visibleColumnNames)
	{
		if (m_columnNames.contains(thisColumnName))
		{
			m_tableView->showColumn(m_columnNames.value(thisColumnName));
		}
	}

	emit columnVisibilitiesChanged(visibleColumnNames);

	// These are default widths-- the state is saved in the settings later
	int columnWidth = m_tableView->width()/3;
	m_tableView->setColumnWidth(1, columnWidth);
	m_tableView->setColumnWidth(2, columnWidth);
	m_tableView->setColumnWidth(3, columnWidth);

	m_tableView->setSortingEnabled(true); // Allow sorting
	m_tableView->verticalHeader()->setVisible(false); // Don't show the row index
	m_tableView->setCornerButtonEnabled(false); // Disable the "select all" button

	// Allow the columns to be reordered
	m_tableView->horizontalHeader()->setSectionsMovable(true);
	m_tableView->horizontalHeader()->setHighlightSections(false);

	// Enable cascading resize (i.e. resizing one column could move/resize another)
	m_tableView->horizontalHeader()->setCascadingSectionResizes(true);

	// Allow the user to resize column widths
	m_tableView->horizontalHeader()->resizeSections(QHeaderView::Interactive);

	// Always take up the full width, even if the last column needs to be stretched
	m_tableView->horizontalHeader()->setStretchLastSection(true);

	// Only start editing an item when it has been selected and is clicked again
	m_tableView->setEditTriggers(QAbstractItemView::SelectedClicked);

	// Only allow selection of entire rows, not table cells
	m_tableView->setSelectionBehavior(QAbstractItemView::SelectRows);

	// Don't show table borders
	m_tableView->setShowGrid(false);
}

QStringList SongsView::visibleColumns()
{
	QStringList visibleColumns;

	QHash<QString, int>::const_iterator iterator;

	for (iterator = m_columnNames.constBegin(); iterator != m_columnNames.constEnd(); ++iterator)
	{
		if (!m_tableView->horizontalHeader()->isSectionHidden(iterator.value()))
		{
			visibleColumns.append(iterator.key());
		}
	}

	return visibleColumns;
}

QModelIndex SongsView::nextSong()
{
	QAbstractItemModel *model = m_tableView->model();

	if (m_currentSong.isValid() && model)
	{
		if (!m_shuffleEnabled)
		{
			QModelIndex nextIndex = model->index(m_currentSong.row() + 1, 0);

			if (nextIndex.isValid())
			{
				return nextIndex;
			}
		}
	}
	else
	{
		QItemSelectionModel *selectionModel = m_tableView->selectionModel();
		if (selectionModel->hasSelection())
		{
			return selectionModel->selectedRows().at(0); // Return the first selected
		}
	}

	return QModelIndex();
}

QModelIndex SongsView::previousSong()
{
	QAbstractItemModel *model = m_tableView->model();

	if (m_currentSong.isValid() && model)
	{
		if (!m_shuffleEnabled)
		{
			QModelIndex previousIndex = model->index(m_currentSong.row() - 1, 0);

			if (previousIndex.isValid())
			{
				return previousIndex;
			}
		}
	}

	return QModelIndex();
}

void SongsView::setShuffleEnabled(bool enabled)
{
	m_shuffleEnabled = enabled;
}

void SongsView::handlePlaying(const QPersistentModelIndex& index)
{
	m_currentSong = index;
}

void SongsView::saveSettings(QSettings &settings)
{
	settings.setValue("state", m_tableView->horizontalHeader()->saveState());
}

void SongsView::loadSettings(const QSettings &settings)
{
	m_tableView->horizontalHeader()->restoreState(settings.value("state").toByteArray());
}
