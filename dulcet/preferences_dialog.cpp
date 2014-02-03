#include <QFileDialog>
#include <QDebug>

#include "preferences_dialog.h"
#include "ui_preferences_dialog.h"

PreferencesDialog::PreferencesDialog(QWidget *parent) :
   QDialog(parent),
   m_ui(new Ui::PreferencesDialog)
{
	m_ui->setupUi(this);
}

PreferencesDialog::~PreferencesDialog()
{
	delete m_ui;
}

void PreferencesDialog::setMusicLibraryPath(const QString &path)
{
	m_ui->musicLibraryPathLineEdit->setText(path);
}

QString PreferencesDialog::musicLibraryPath() const
{
	return m_ui->musicLibraryPathLineEdit->text();
}

void PreferencesDialog::setWatchLibrary(bool enabled)
{
	m_ui->watchLibraryCheckBox->setChecked(enabled);
}

bool PreferencesDialog::watchLibrary() const
{
	return m_ui->watchLibraryCheckBox->isChecked();
}

void PreferencesDialog::setWriteTagsToFile(bool enabled)
{
	m_ui->writeTagsToFileCheckBox->setChecked(enabled);
}

bool PreferencesDialog::writeTagsToFile() const
{
	return m_ui->writeTagsToFileCheckBox->isChecked();
}

void PreferencesDialog::setOrganizeLibrary(bool enabled)
{
	m_ui->organizeLibraryCheckBox->setChecked(enabled);
}

bool PreferencesDialog::organizeLibrary() const
{
	return m_ui->organizeLibraryCheckBox->isChecked();
}

void PreferencesDialog::setCopyOnImport(bool enabled)
{
	m_ui->copyOnImportCheckBox->setChecked(enabled);
}

bool PreferencesDialog::copyOnImport() const
{
	return m_ui->copyOnImportCheckBox->isChecked();
}

void PreferencesDialog::on_browseButton_clicked()
{
	QString startingLocation = m_ui->musicLibraryPathLineEdit->text();

	if (startingLocation.isEmpty())
	{
		startingLocation = QDir::homePath();
	}

	QString libraryPath = QFileDialog::getExistingDirectory(this,
	                                                        tr("Select Music Library Directory"),
	                                                        startingLocation);

	if (!libraryPath.isEmpty())
	{
		m_ui->musicLibraryPathLineEdit->setText(libraryPath);
	}
}
