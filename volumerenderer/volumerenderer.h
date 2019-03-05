#pragma once
#ifndef _VOLUMERENDERER_H_
#define _VOLUMERENDERER_H_

#ifdef INTERNAL_DLL
#include "../isosurfacerender/gl3w/GL/gl3w.h"
#endif

#include <QtWidgets/QOpenGLWidget>

class VolumeRendererPrivate;

class Q_DECL_EXPORT VolumeRenderer :public QOpenGLWidget
{
public:
	VolumeRenderer(QWidget * parent = nullptr);

	void setVolumeData(unsigned char * data, std::size_t width, std::size_t height, std::size_t depth, float xSpacing, float ySpacing, float zSpacing);
	void setTransferFunction(float * transferFuncs);
	void initializeGL() override;
	void paintGL() override;
	void resizeGL(int w, int h) override;
	~VolumeRenderer() = default;
protected:
	void mouseMoveEvent(QMouseEvent* event) override;
	void mousePressEvent(QMouseEvent* event) override;
	void mouseReleaseEvent(QMouseEvent* event) override;
private:
	QScopedPointer<VolumeRendererPrivate> const d_ptr;
	Q_DECLARE_PRIVATE(VolumeRenderer);
};
#endif


