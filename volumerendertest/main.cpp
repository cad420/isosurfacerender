#include "volumerendertest.h"
#include <QApplication>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	volumerendertest w;
	w.show();
	return a.exec();
}
