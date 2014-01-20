#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <QMainWindow>

#include "music_table_model.h"

namespace Ui
{
	class MainWindow;
}

class QSqlTableModel;

class MainWindow : public QMainWindow
{
	Q_OBJECT

	public:
		explicit MainWindow(QWidget *parent = 0);
		~MainWindow();

	private slots:
		void handleImportStatusUpdate(const QString &status);

	private:
		Ui::MainWindow *m_ui;
		QSqlTableModel *m_musicTableModel;
};

#endif // MAIN_WINDOW_H
