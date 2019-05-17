#include "volumerendertest.h"

#include "volumerender.h"
#include <QLayout>

#include <QWidget>
#include <QPushButton>

#include <fstream>
#include <iostream>
#include "tf.h"

volumerendertest::volumerendertest(QWidget *parent)
	: QMainWindow(parent)
{
	renderer = new VolumeRenderer(this);


	addButton = new QPushButton(QStringLiteral("Add"), this);


	connect(addButton, &QPushButton::clicked, [this]()
	{
			int x = 300, y= 124, z = 124;
		std::unique_ptr<unsigned char[]> buf(new unsigned char[x*y*z]);
		std::ifstream ifs(R"(D:\density.raw)");
		if(ifs.is_open() == false)
		{
			std::cout << "file can not be opened\n";
			return;
		}
		ifs.read((char *)buf.get(), x*y*z*sizeof(unsigned char));
		renderer->setVolumeData(buf.get(), x, y, z, 1, 1, 1);


		std::vector<ysl::RGBASpectrum> tfData(256);
		ysl::TransferFunction tfObject;
		tfObject.read("d:\\scidata\\std_tf1d.TF1D");
		tfObject.FetchData(tfData.data(), 256);

		renderer->setTransferFunction(reinterpret_cast<float*>(tfData.data()));

	});




	auto * centralWidget = new QWidget(this);
	auto * mainLayout = new QVBoxLayout;


	mainLayout->addWidget(renderer);
	mainLayout->addWidget(addButton);
	centralWidget->setLayout(mainLayout);
	setCentralWidget(centralWidget);
}
