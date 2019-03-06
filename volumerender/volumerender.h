#pragma once


#ifdef INTERNAL_DLL
#include "gl3w/GL/gl3w.h"
#endif

#include <QOpenGLWidget>

class VolumeRendererPrivate;

class Q_DECL_EXPORT VolumeRenderer :public QOpenGLWidget
{
public:
	VolumeRenderer(QWidget * parent = nullptr);
	void setVolumeData(unsigned char * data, std::size_t width, std::size_t height, std::size_t depth, float xSpacing, float ySpacing, float zSpacing);
	void setTransferFunction(float * transferFuncs);
	void setIlluminationParams(float ka,float ks,float kd,float shininess);
	void initializeGL() override;
	void paintGL() override;
	void resizeGL(int w, int h) override;
	~VolumeRenderer();
protected:
	void mouseMoveEvent(QMouseEvent* event) override;
	void mousePressEvent(QMouseEvent* event) override;
	void mouseReleaseEvent(QMouseEvent* event) override;
private:
	QScopedPointer<VolumeRendererPrivate> const d_ptr;
	Q_DECLARE_PRIVATE(VolumeRenderer);
};