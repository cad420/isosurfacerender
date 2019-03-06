#pragma once

#include <QtWidgets/QMainWindow>


class VolumeRenderer;
class QPushButton;
class volumerendertest : public QMainWindow
{
	Q_OBJECT

public:
	volumerendertest(QWidget *parent = Q_NULLPTR);

private:
	VolumeRenderer * renderer;
	QPushButton * addButton;

};
