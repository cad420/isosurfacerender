#pragma once

#ifdef INTERNAL_DLL
#include "gl3w/GL/gl3w.h"
#endif

#include <QOpenGLWidget>


class ISOSurfaceRendererPrivate;

class Q_DECL_EXPORT ISOSurfaceRenderer: public QOpenGLWidget
{
	
public:
	ISOSurfaceRenderer(QWidget * parent = nullptr);
	void initializeGL() override;
	void paintGL() override;
	void resizeGL(int w, int h) override;
	/**
	 * \brief Adds a volume data named /a dataName, pointed by /a data
	 * \param dataName 
	 * \param data 
	 * \param width 
	 * \param height 
	 * \param depth 
	 */


	void addVolumeData(const QString & dataName, const unsigned char* data, std::size_t width, std::size_t height, std::size_t depth);
	/**
	 * \brief 
	 * \param dataName 
	 * \param isoName 
	 * \param color 
	 * \param isovalue 
	 */

	void addIsoSurface(const QString & dataName, const QString & isoName, const QColor & color, int isovalue);

	void setIsosurfaceValue(const QString & dataName, const QString & isoName, int value);

	void setIsosurfaceColor(const QString & dataName, const QString & isoName, const QColor & color);

	void removeIsosurface(const QString & dataName, const QString & isoName);

	void setIsosurfaceVisibility(const QString & dataName, const QString & isoName,bool visible);

	void setIsosurfacesVisibility(const QString & dataName, bool visible);

	void setBoundingBoxVisibility(bool visibile);

	void removeVolumeData(const QString & dataName);

	void setCameraPosition(const QVector3D & pos, const QVector3D & focus, const QVector3D & up);

	//void setTessLevels(float inner, float outer);


	QStringList volumeData()const;

	QStringList isosurface(const QString & dataName)const;

	~ISOSurfaceRenderer();

protected:
	void mouseMoveEvent(QMouseEvent* event) override;
	void mousePressEvent(QMouseEvent* event) override;
	void mouseReleaseEvent(QMouseEvent* event) override;
private:

	QScopedPointer<ISOSurfaceRendererPrivate> const d_ptr;
	Q_DECLARE_PRIVATE(ISOSurfaceRenderer)
};

