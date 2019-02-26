#include "test.h"
#include <qfile.h>
#include <QLayout>
#include <iostream>

test::test(QWidget *parent)
	: QMainWindow(parent)
{
	renderer = new ISOSurfaceRenderer(this);
	addButton = new QPushButton(QStringLiteral("Add"),this);
	removeButton = new QPushButton(QStringLiteral("Remove"), this);
	visibleButton = new QPushButton(QStringLiteral("Visibility"),this);

	


	visible = false;

	connect(visibleButton, &QPushButton::clicked, [this]()
	{
		renderer->setIsosurfaceVisibility("head", "head_50", visible);
		renderer->setIsosurfaceVisibility("head", "head_20", visible);
		renderer->setIsosurfacesVisibility("head", visible);
		visible = !visible;
	});

	connect(removeButton, &QPushButton::clicked, [this]()
	{
		renderer->removeIsosurface("head", "head_80");
		renderer->removeVolumeData("head");
	});
	
	connect(addButton, &QPushButton::clicked, [this]()
	{
		//renderer->addVolumeData("nn", (unsigned char*)readData("D:\\scidata\\csafe-heptane-302-volume.raw",302,302,302),302,302,302);
		//renderer->addIsoSurface("nn", "isabel_100", QColor(50, 255, 10, 128), 0);

		//renderer->addVolumeData("isabel", (unsigned char*)readData("D:\\scidata\\Isabel.raw",500,500,100), 500,500,100);
		//renderer->addIsoSurface("isabel", "isabel_200", QColor(128, 54, 188, 128), 50);

		//renderer->addVolumeData("head", (unsigned char*)readData("D:\\scidata\\head.raw", 256, 256, 225), 256, 256, 225);
		//renderer->addIsoSurface("head", "head_50", QColor(10,10,255, 200), 50);
		//renderer->addIsoSurface("head", "head_80", QColor(128,255,0, 200), 120);

		//renderer->addIsoSurface("head", "head_20", QColor(128,87, 32, 100), 20);

		renderer->addVolumeData("com", (unsigned char*)readData(R"(D:\combustion\Combustion_480_720_120\hr.raw)",480,720,120), 480,720,120);
		renderer->addIsoSurface("com", "com_20", QColor(50, 255, 10, 130), 100);
		renderer->addIsoSurface("com", "com_50", QColor(10,40,255, 200), 200);

		//renderer->addVolumeData("planet", (unsigned char*)readData("D:\\scidata\\prs.raw",300,300,300),300,300,300);
		//renderer->addIsoSurface("planet", "planet_20", QColor(128, 87, 32, 250),64);
		
	});

	isoValueSlider = new QSlider(Qt::Horizontal, this);
	isoValueSlider2 = new QSlider(Qt::Horizontal, this);
	//innerSlider = new QSlider(Qt::Horizontal, this);
	//outerSlider= new QSlider(Qt::Horizontal, this);

	isoValueSlider->setMaximum(255);
	isoValueSlider2->setMaximum(255);
	isoValueSlider2->setValue(50);


	connect(isoValueSlider, &QSlider::valueChanged, [this](int value)
	{
		renderer->setIsosurfaceValue("com", "com_20", value);
		//renderer->setIsosurfaceValue("planet", "planet_20", value);
	});

	connect(isoValueSlider2, &QSlider::valueChanged, [this](int value)
	{
		renderer->setIsosurfaceValue("com", "com_50", value);
	});

	//connect(innerSlider, &QSlider::valueChanged, [this](int value)
	//	{

	//		renderer->setTessLevels(1.0*value,-1);
	//	});

	//connect(outerSlider, &QSlider::valueChanged, [this](int value)
	//	{
	//		//renderer->setIsosurfaceValue("com", "com_50", value);
	//		renderer->setTessLevels(-1,1.0*value);
	//	});



	auto layout = new QVBoxLayout(this);
	auto mainWidget = new QWidget(this);

	layout->addWidget(renderer);
	layout->addWidget(addButton);
	layout->addWidget(removeButton);
	layout->addWidget(visibleButton);
	layout->addWidget(isoValueSlider);
	layout->addWidget(isoValueSlider2);
	//layout->addWidget(innerSlider);
	//layout->addWidget(outerSlider);

	mainWidget->setLayout(layout);

	//renderer->resize(800, 600);
	resize(800, 600);
	setCentralWidget(mainWidget);
	
	//renderer->addVolumeData("head", readData("D:\\scidata\\head.raw",256,256,255), 256, 256, 255);
	//renderer->addIsoSurface("head", "head_50", QColor(128,54,188,255), 50);
	//renderer->addVolumeData("isabel", readData(R"(D:\scidata\Isabel.raw)", 500, 500, 100), 500, 500, 100);
}

test::~test()
{

}

char * test::readData(const QString & fileName, std::size_t x, std::size_t y, std::size_t z)
{
	QFile file(fileName);
	file.open(QIODevice::ReadOnly);
	if(file.isOpen() == false)
		return nullptr;
	auto * d = new  char[x*y*z];
	file.read(d, x*y*z);
	return d;
}

void test::freeData(char* data)
{
	delete[] data;
}
