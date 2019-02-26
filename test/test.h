#pragma once

#include <QtWidgets/QMainWindow>

#include "../isosurfacerender/isosurfacerender.h"

#include <QPushButton>
#include <QSlider>

class test : public QMainWindow
{
	Q_OBJECT

public:
	test(QWidget *parent = Q_NULLPTR);
	~test();
private:
	char * readData(const QString & fileName,std::size_t x,std::size_t y,std::size_t z);
	void freeData(char * data);
	ISOSurfaceRenderer * renderer;
	QPushButton * addButton;
	QPushButton * visibleButton;
	QPushButton * removeButton;
	QSlider * isoValueSlider;
	QSlider * isoValueSlider2;
	QSlider * innerSlider;
	QSlider * outerSlider;
	bool visible;
};
