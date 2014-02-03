#ifndef FILE_DIALOG_H
#define FILE_DIALOG_H

#include <QFileDialog>

class QFileSystemModel;

class FileDialog : public QFileDialog
{
	Q_OBJECT

	public:
		FileDialog(QWidget *parent, Qt::WindowFlags flags);
		explicit FileDialog(QWidget *parent = 0,
		                    const QString &caption = QString(),
		                    const QString &directory = QString(),
		                    const QString &filter = QString());

	protected:
		virtual void accept();

	private slots:
		void selectionChanged();

	private:
		void setup();
};

#endif // FILE_DIALOG_H
