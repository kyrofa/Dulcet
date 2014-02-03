#include <QDebug>
#include <QListView>
#include <QTreeView>
#include <QPushButton>
#include <QDialogButtonBox>

#include "file_dialog.h"

FileDialog::FileDialog(QWidget *parent, Qt::WindowFlags flags) :
   QFileDialog(parent, flags)
{
	setup();
}

FileDialog::FileDialog(QWidget *parent, const QString &caption,
                    const QString &directory, const QString &filter) :
   QFileDialog(parent, caption, directory, filter)
{
	setup();
}

void FileDialog::setup()
{
	setFileMode(QFileDialog::Directory);

	setOption(QFileDialog::DontUseNativeDialog, true);

	QListView *l = findChild<QListView*>("listView");
	if (l)
	{
		l->setSelectionMode(QAbstractItemView::ExtendedSelection);
		connect(l->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
		        SLOT(selectionChanged()));
	}
	QTreeView *t = findChild<QTreeView*>();
	if (t)
	{
		t->setSelectionMode(QAbstractItemView::ExtendedSelection);
		// The treeview and listview share a selection model
	}
}

void FileDialog::accept()
{
	QStringList files = selectedFiles();

	if (!files.isEmpty())
	{
		emit filesSelected(files);

		QDialog::accept();
	}
}

void FileDialog::selectionChanged()
{
	QDialogButtonBox *buttonBox = findChild<QDialogButtonBox*>("buttonBox");
	QPushButton *openButton = buttonBox->button(QDialogButtonBox::Open);

	if (openButton)
	{
		openButton->setEnabled(true);
	}
}
