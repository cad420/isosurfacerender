#include "isosurfacerender.h"

#include <QMouseEvent>

#include "opengl/openglutils.h"
#include "isosurfacerendererprivate.h"

#include <QSharedPointer>
#include "mathematics/numeric.h"
#include <QPainter>
#include <QVector3D>
#include "ArcBall.h"
#include <QSurfaceFormat>


//#define TESTSHADER


static ysl::Point3f cubeVertex[8] =
{ {0,0,0},{1,0,0},{1,1,0},{0,1,0},{0,0,1},{1,0,1},{1,1,1},{0,1,1} };
static int cubeVertexIndices[24] = { 0,1,1,2,2,3,3,0,4,5,5,6,6,7,7,4,0,4,5,1,2,6,3,7 };



ISOSurfaceRenderer::ISOSurfaceRenderer(QWidget* parent) :QOpenGLWidget(parent), d_ptr(new ISOSurfaceRendererPrivate(this))
{
	QSurfaceFormat fmt;
    fmt.setVersion(4,4);
    fmt.setDepthBufferSize(24);
    fmt.setStencilBufferSize(8);
    fmt.setProfile(QSurfaceFormat::CoreProfile);
	fmt.setSamples(8);
    QSurfaceFormat::setDefaultFormat(fmt);
	resize(800, 600);

}

void ISOSurfaceRenderer::initializeGL()
{
	Q_D(ISOSurfaceRenderer);

	gl3wInit();
	if (!gl3wIsSupported(4, 4))
	{
		ysl::Error("OpenGL 4.4 or higher is needed.");
	}
	glClearColor(1,1,1,1.0);
	
	glEnable(GL_BLEND);
	//glEnable(GL_DEPTH_TEST);
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_MULTISAMPLE);

	d->InitShader();
	d->UpdateMatrix(d->windowWidth, d->windowHeight);

	
	//float c1[4] = { 0.f,1.0,0.0,0.9 };
	//float c2[4] = { 0.,0.,1.,0.9 };
	//d->CreateMesh(R"(C:\Users\ysl\Desktop\dragon_2.obj)", ysl::RGBASpectrum{ c1 });
	//d->CreateMesh(R"(C:\Users\ysl\Desktop\monu10.obj)",ysl::RGBASpectrum{c1});

	//d->CreateMesh(R"(C:\Users\ysl\Desktop\cube.obj)", ysl::RGBASpectrum{ c1 });
	//d->CreateMesh(R"(C:\Users\ysl\Desktop\monu10.obj)", ysl::RGBASpectrum{ c1 });

	d->boundingBox.vao = std::make_shared<OpenGLVertexArrayObject>();
	d->boundingBox.vao->create();
	d->boundingBox.vbo = std::make_shared<OpenGLBuffer>(OpenGLBuffer::BufferTarget::VertexArrayBuffer);
	d->boundingBox.ebo = std::make_shared<OpenGLBuffer>(OpenGLBuffer::BufferTarget::ElementArrayBuffer);
	d->CreateMesh(cubeVertex,nullptr,nullptr,8,cubeVertexIndices,24,&d->boundingBox); // Bounding box


	d->CreateScreenQuads();
	d->CreateFragmentBufferList(d->windowWidth, d->windowHeight);
	d->CreateHeadPointerImageInitializer(d->windowWidth, d->windowHeight);
	d->CreateAtomicCounter();
	d->CreateImageListTexture(d->windowWidth, d->windowHeight);
}


void ISOSurfaceRenderer::paintGL()
{
	Q_D(ISOSurfaceRenderer);

	//glEnable(GL_DEPTH_TEST);
	//glEnable(GL_DEPTH_TEST);
	glClear(GL_COLOR_BUFFER_BIT );
	glClear(GL_DEPTH_BUFFER_BIT);
	//glDepthFunc(GL_ALWAYS);
	
	// Set Camera Uniform
	// Clear HeadPointerImage
	d->ClearHeadPointerImage();
	GL_ERROR_REPORT
	d->ClearAtomicCounter();
	GL_ERROR_REPORT

	//float c[4] = { 0.5,0.7,0.8,1.0 };
	const auto view = d->camera.view().Matrix();

#ifdef TESTSHADER
	d->testShader.bind();
	d->testShader.setUniformValue("view_matrix", view);
	d->testShader.setUniformValue("projection_matrix", d->proj.Matrix());
	d->testShader.setUniformValue("light_pos", ysl::Vector3f{ 0.f,200.f,0.f });
	d->testShader.setUniformValue("light_color", ysl::Vector3f{ 1.0,1.0,1.0 });
	d->testShader.setUniformValue("view_pos", d->camera.position());
#else
	d->oitListShader.bind();
	GL_ERROR_REPORT
	// vertex shader
	d->oitListShader.setUniformValue("view_matrix", d->camera.view().Matrix());
	GL_ERROR_REPORT
	d->oitListShader.setUniformValue("projection_matrix", d->proj.Matrix());
	// fragment shader
	d->oitListShader.setUniformValue("light_pos", ysl::Vector3f{ 0.f,200.f,0.f });
	GL_ERROR_REPORT
	d->oitListShader.setUniformValue("light_color", ysl::Vector3f{ 1.0,1.0,1.0 });
	GL_ERROR_REPORT
	d->oitListShader.setUniformValue("view_pos", d->camera.position());
	GL_ERROR_REPORT


	//d->oitListShader.setUniformValue("tlsi", d->inner);
	//d->oitListShader.setUniformValue("tlso", d->outer);

	float outer_level[4] = {5.0,5.0,5.0,0.0};
	float inner_level[2] = {1.0,0.0};

#endif
	{
		//glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
		for (auto & m : d->meshes)
		{
			if (m.visible)
			{
				m.vao->bind();
#ifdef TESTSHADER
				d->testShader.setUniformValue("object_color", m.color);
				d->testShader.setUniformValue("model_matrix", (d->rotation * m.modelTransform).Matrix());
#else
				d->oitListShader.setUniformValue("model_matrix", (d->rotation*(m.modelTransform)).Matrix());
				d->oitListShader.setUniformValue("object_color", m.color);
#endif
				//glPatchParameteri(GL_PATCH_VERTICES, 3);

				//glPatchParameterfv(GL_PATCH_DEFAULT_OUTER_LEVEL,outer_level);
				//glPatchParameterfv(GL_PATCH_DEFAULT_INNER_LEVEL,inner_level);

				//glLineWidth(5);
				//glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);
				//glDrawElements(GL_PATCHES, m.indexCount, GL_UNSIGNED_INT, 0);

				glDrawElements(GL_TRIANGLES, m.indexCount, GL_UNSIGNED_INT, 0);
				glFlush();
				m.vao->unbind();
			}
		}
	}

	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	if(d->boundingBoxVisible)
	{
		float c[4] = { 1,0,0,1 };
		d->boundingBox.vao->bind();
		d->oitListShader.setUniformValue("object_color", ysl::RGBASpectrum{c});
		d->oitListShader.setUniformValue("model_matrix", d->boundingBox.modelTransform.Matrix());
		glLineWidth(2);
		glDrawElements(GL_LINES, d->boundingBox.indexCount, GL_UNSIGNED_INT, 0);
		d->boundingBox.vao->unbind();
	}

	d->oitListShader.unbind();

#ifndef TESTSHADER
	{
		d->oitRenderShader.bind();
		// Vertex Shader
		d->oitRenderShader.setUniformValue("orthoMatrix", d->ortho.Matrix());
		d->oitRenderShader.setUniformValue("viewMatrix", view);
		// Fragment Shader
		d->screenQuads.vao.bind();
		glDrawArrays(GL_TRIANGLES, 0, 6);
		d->screenQuads.vao.unbind();
		d->oitRenderShader.unbind();
	}
#endif

	QPainter p(this);
	d->DrawLegend(&p);
}

void ISOSurfaceRenderer::resizeGL(int w, int h)
{
	Q_D(ISOSurfaceRenderer);
	if (!d)return;

	d->windowWidth = w;
	d->windowHeight = h;

	d->UpdateMatrix(d->windowWidth, d->windowHeight);
	
	if (context())
	{
		GL_ERROR_REPORT;
		d->ResizeScreenQuads(d->windowWidth, d->windowHeight);
		GL_ERROR_REPORT;
		d->ResizeHeadPointerImage(d->windowWidth, d->windowHeight);
		GL_ERROR_REPORT;
		d->ResizeFragmentBufferList(d->windowWidth, d->windowHeight);
		GL_ERROR_REPORT;
		d->ResizeInitializer(d->windowWidth, d->windowHeight);
		GL_ERROR_REPORT;
	}
	update();
}

void ISOSurfaceRenderer::addVolumeData(const QString& dataName, const unsigned char* data, std::size_t width, std::size_t height,
                                       std::size_t depth)
{
	Q_D(ISOSurfaceRenderer);
	const auto itr = d->dataName2Data.find(dataName);
	if (itr == d->dataName2Data.end())
	{
		d->dataName2Data[dataName] = ISOSurfaceRendererPrivate::ValueType{ {}, QSharedPointer<MeshGenerator>{new MeshGenerator{data, { width,height,depth }}} };
	}
}

void ISOSurfaceRenderer::addVolumeData(const QString& dataName, const unsigned char* data, std::size_t width,
	std::size_t height, std::size_t depth, float xSpace, float ySpace, float zSpace)
{
	Q_D(ISOSurfaceRenderer);
	const auto itr = d->dataName2Data.find(dataName);
	if (itr == d->dataName2Data.end())
	{
		d->dataName2Data[dataName] = ISOSurfaceRendererPrivate::ValueType{ {}, QSharedPointer<MeshGenerator>{new MeshGenerator{data, { width,height,depth },{xSpace,ySpace,zSpace}}} };
	}
}

void ISOSurfaceRenderer::addIsoSurface(const QString& dataName, const QString& isoName, const QColor& color,
                                       int isovalue)
{
	Q_D(ISOSurfaceRenderer);
	const auto itr = d->dataName2Data.find(dataName);
	if (itr != d->dataName2Data.end())
	{
		//d->dataName2ISOName[dataName].insert(isoName);
		const auto itr1 = itr->first.find(isoName);

		if (itr1 == itr->first.end())		// if the isosurface does not exist
		{
			//d->CreateMesh(isovalue, itr->second, *iter);

			auto iter = d->CreateMesh();
			d->CreateMesh(itr->second->GenerateMesh(isovalue),*iter);
			itr->first[isoName] = iter;

			const auto dataSize = itr->second->DataSize();
			const auto space = itr->second->space();
			auto scale = ysl::Transform{};
			scale.SetScale(0.2*space.x,0.2*space.y,0.2*space.z);
			auto translate = ysl::Transform{};
			translate.SetTranslate((-float(dataSize.x)) / 2.0, (-float(dataSize.y)) / 2.0, (-float(dataSize.z)) / 2.0);
			iter->modelTransform = scale * translate;
			iter->name = (isoName).toStdString();

			auto boxScale = ysl::Transform{}, boxTranslate = ysl::Transform{};
			boxScale.SetScale(ysl::Vector3f(dataSize.x*space.x, dataSize.y*space.y, dataSize.z*space.z)*0.2);
			boxTranslate.SetTranslate(-.5f,-.5f,-.5f);
			d->boundingBox.modelTransform = boxScale * boxTranslate;

			//iter->visible = true;

			float c[4] = { color.redF(), color.greenF(), color.blueF(), color.alphaF() };
			iter->color = ysl::RGBASpectrum{ c };

			update();
		}
	}
}

void ISOSurfaceRenderer::setIsosurfaceValue(const QString& dataName, const QString& isoName, int value)
{
	Q_D(ISOSurfaceRenderer);
	const auto itr = d->dataName2Data.find(dataName);		// &ValueType

	if (itr != d->dataName2Data.end())
	{
		const auto itr1 = (itr->first).find(isoName);		// mesh list iterator
		if (itr1 != itr->first.end())
		{
			d->CreateMesh(itr->second->GenerateMesh(value), **itr1);
			update();
		}
	}
}

void ISOSurfaceRenderer::setIsosurfaceColor(const QString& dataName, const QString& isoName, const QColor& color)
{
	Q_D(ISOSurfaceRenderer);
	const auto itr = d->dataName2Data.find(dataName);		// &ValueType

	if (itr != d->dataName2Data.end())
	{
		const auto itr1 = (itr->first).find(isoName);		// mesh list iterator
		if (itr1 != itr->first.end())
		{
			float c[4] = { color.redF(), color.greenF(), color.blueF(), color.alphaF() };
			(*itr1)->color = ysl::RGBASpectrum{ c };
		}
	}
}

void ISOSurfaceRenderer::removeIsosurface(const QString& dataName, const QString& isoName)
{
	Q_D(ISOSurfaceRenderer);
	const auto itr = d->dataName2Data.find(dataName);		// &ValueType

	if (itr != d->dataName2Data.end())
	{
		const auto itr1 = (itr->first).find(isoName);		// mesh list iterator
		if (itr1 != itr->first.end())
		{
			makeCurrent();
			d->meshes.erase(*itr1);			// There need to destroy OpenGL Resources
			doneCurrent();

			itr->first.erase(itr1);		// Delete value item referenced by isoName
			update();
		}
	}
}

void ISOSurfaceRenderer::setIsosurfaceVisibility(const QString & dataName, const QString & isoName, bool visible)
{
	Q_D(ISOSurfaceRenderer);
	const auto itr = d->dataName2Data.find(dataName);		// &ValueType

	if (itr != d->dataName2Data.end())
	{
		const auto itr1 = (itr->first).find(isoName);		// mesh list iterator
		if (itr1 != itr->first.end())
		{
			(*itr1)->visible = visible;
			update();
		}
	}
}

void ISOSurfaceRenderer::setIsosurfacesVisibility(const QString & dataName, bool visible)
{
	Q_D(ISOSurfaceRenderer);
	const auto itr = d->dataName2Data.find(dataName);		// &ValueType

	if (itr != d->dataName2Data.end())
	{
		const auto surfaces = isosurface(dataName);
		for (const auto & name : surfaces)
		{
			setIsosurfaceVisibility(dataName, name, visible);
		}
	}
}

void ISOSurfaceRenderer::setBoundingBoxVisibility(bool visibile)
{
	Q_D(ISOSurfaceRenderer);
	d->boundingBoxVisible = visibile;
}

void ISOSurfaceRenderer::removeVolumeData(const QString & dataName)
{
	Q_D(ISOSurfaceRenderer);
	const auto itr = d->dataName2Data.find(dataName);		// &ValueType

	if (itr != d->dataName2Data.end())
	{
		const auto surfaces = isosurface(dataName);
		for (const auto & name : surfaces)
		{
			removeIsosurface(dataName, name);
		}
	}
}

void ISOSurfaceRenderer::setCameraPosition(const QVector3D& pos, const QVector3D& focus, const QVector3D& up)
{
	Q_D(ISOSurfaceRenderer);
	d->camera.setPosition({pos.x(),pos.y(),pos.z()});
	d->camera.setUpDirection({up.x(),up.y(),up.z()});
	d->camera.setCenter({focus.x(),focus.y(),focus.z()});
	update();
}

//void ISOSurfaceRenderer::setTessLevels(float inner, float outer)
//{
//	Q_D(ISOSurfaceRenderer);
//	if(inner >= 0)d->inner = inner;
//	if(outer >= 0)d->outer = outer;
//	update();
//}


QStringList ISOSurfaceRenderer::volumeData() const
{
	return d_ptr->dataName2Data.keys();
}

QStringList ISOSurfaceRenderer::isosurface(const QString& dataName) const
{
	QStringList lists;
	const auto itr = d_ptr->dataName2Data.find(dataName);
	if (itr != d_ptr->dataName2Data.end())
		lists = itr->first.keys();
	return lists;
}

ISOSurfaceRenderer::~ISOSurfaceRenderer()
{
	Q_D(ISOSurfaceRenderer);
	d->Cleanup();
}

void ISOSurfaceRenderer::mouseMoveEvent(QMouseEvent * event)
{
	Q_D(ISOSurfaceRenderer);

	const auto & p = ysl::Point2i{ event->pos().x(),event->pos().y() };
	// Update Camera
	const float dx = p.x - d->lastMousePos.x;
	const float dy = d->lastMousePos.y - p.y;
	if (dx == 0 && dy == 0)
		return;
	if ((event->buttons() & Qt::MouseButton::LeftButton) && (event->buttons() & Qt::MouseButton::RightButton))
	{
		const auto directionEx = d->camera.up()*dy + dx * d->camera.right();
		d->camera.movement(directionEx, 0.002);
	}

	else if (event->buttons() & Qt::MouseButton::LeftButton)
	{
		d->camera.rotation(dx, dy);

		//ArcBall arcball(width(), height());
		//const auto axis = arcball.RotationAxis(d->lastMousePos, p);
		//const auto radiance = arcball.RotationRadiance(d->lastMousePos, p);
		//ysl::Transform rot;
		//rot.SetRotate(axis.Normalized(), ysl::RadiansToDegrees(radiance*10));
		//d->rotation = rot;

	}
	else if (event->buttons() == Qt::MouseButton::RightButton)
	{
		const auto directionEx = d->camera.front()*dy;
		d->camera.movement(directionEx, 0.01);
	}
	d->lastMousePos = p;
	update();
}

void ISOSurfaceRenderer::mousePressEvent(QMouseEvent * event)
{
	Q_D(ISOSurfaceRenderer);
	const auto p = event->pos();
	d->lastMousePos = ysl::Point2i{ p.x(),p.y() };
}

void ISOSurfaceRenderer::mouseReleaseEvent(QMouseEvent * event)
{

}

