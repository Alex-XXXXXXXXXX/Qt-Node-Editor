#include "include/QtNodes.h"
#include <QtWidgets/QApplication>

int main(int argc, char* argv[])
{
	QApplication app(argc, argv);
	QtNodes window;
	window.show();
	return app.exec();
}