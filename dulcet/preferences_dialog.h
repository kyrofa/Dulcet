#ifndef PREFERENCES_DIALOG_H
#define PREFERENCES_DIALOG_H

#include <QDialog>

namespace Ui
{
	class PreferencesDialog;
}

class PreferencesDialog : public QDialog
{
	Q_OBJECT

	public:
		explicit PreferencesDialog(QWidget *parent = 0);
		~PreferencesDialog();

		void setMusicLibraryPath(const QString &path);
		QString musicLibraryPath() const;

		void setWatchLibrary(bool enabled);
		bool watchLibrary() const;

		void setWriteTagsToFile(bool enabled);
		bool writeTagsToFile() const;

		void setOrganizeLibrary(bool enabled);
		bool organizeLibrary() const;

		void setCopyOnImport(bool enabled);
		bool copyOnImport() const;

	private slots:
		void on_browseButton_clicked();

	private:
		Ui::PreferencesDialog *m_ui;
};

#endif // PREFERENCES_DIALOG_H
