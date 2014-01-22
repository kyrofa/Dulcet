#include <QApplication>
#include "main_window.h"

#define APPLICATION_NAME "Dulcet"
#define APPLICATION_VERSION "0.0.1"

int main(int argc, char *argv[])
{
	QApplication application(argc, argv);
	MainWindow mainWindow;
	mainWindow.show();

	application.setApplicationName(APPLICATION_NAME);
	application.setApplicationVersion(APPLICATION_VERSION);

	return application.exec();
}
