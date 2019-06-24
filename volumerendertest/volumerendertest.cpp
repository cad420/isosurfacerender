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
			int x = 256, y = 256, z = 256;
			std::unique_ptr<unsigned char[]> buf(new unsigned char[x*y*z]);
			std::ifstream ifs(R"(D:\Desktop\s6.raw)");
			if (ifs.is_open() == false)
			{
				std::cout << "file can not be opened\n";
				return;
			}
			ifs.read((char *)buf.get(), x*y*z * sizeof(unsigned char));

			renderer->setVolumeData(buf.get(), x, y, z, 1, 1, 1);

			std::unique_ptr<unsigned char> mask(new unsigned char[x*y*z]);
			std::ifstream mfs(R"(D:\Desktop\mask.raw)");
			if (mfs.is_open() == false)
			{
				std::cout << "file can not be opened\n";
				return;
			}
			mfs.read((char *)mask.get(), x*y*z * sizeof(unsigned char));
			renderer->setMask(mask.get(), { 0,1,2,3,4});

			std::vector<ysl::RGBASpectrum> tf(256);
			ysl::TransferFunction tfObject1;
			tfObject1.Read("D:\\Desktop\\0.TF1D");
			tfObject1.FetchData(tf.data(), 256);
			renderer->setTransferFunction(0, reinterpret_cast<float*>(tf.data()));

			ysl::TransferFunction tfObject2;
			tfObject2.Read("D:\\Desktop\\1.TF1D");
			tfObject2.FetchData(tf.data(), 256);
			renderer->setTransferFunction(1, (float*)tf.data());

			ysl::TransferFunction tfObject3;
			tfObject3.Read("D:\\Desktop\\2.TF1D");
			tfObject3.FetchData(tf.data(), 256);
			renderer->setTransferFunction(2, (float*)tf.data());


			ysl::TransferFunction tfObject4;
			tfObject4.Read("D:\\Desktop\\3.TF1D");
			tfObject4.FetchData(tf.data(), 256);
			renderer->setTransferFunction(3, (float*)tf.data());

			ysl::TransferFunction tfObject5;
			tfObject5.Read("D:\\Desktop\\4.TF1D");
			tfObject5.FetchData(tf.data(), 256);
			renderer->setTransferFunction(4, (float*)tf.data());

		});




	auto * centralWidget = new QWidget(this);
	auto * mainLayout = new QVBoxLayout;


	mainLayout->addWidget(renderer);
	mainLayout->addWidget(addButton);
	centralWidget->setLayout(mainLayout);
	setCentralWidget(centralWidget);
}
