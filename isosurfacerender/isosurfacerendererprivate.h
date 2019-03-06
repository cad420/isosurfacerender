#pragma once

#include <qglobal.h>
#include "opengl/texture.h"
#include "opengl/shader.h"
#include "cameras/camera.h"
#include "opengl/openglvertexarrayobject.h"
#include "opengl/framebuffer.h"
#include "algorithm/marchingcubes.h"
#include <QSharedPointer>
#include "shape/triangle.h"


class QPainter;
class ISOSurfaceRenderer;

class ISOSurfaceRendererPrivate
{
public:

	struct OpenGLRenderingObject
	{
		std::shared_ptr<OpenGLBuffer> vbo;
		std::shared_ptr<OpenGLBuffer> ebo;
		std::shared_ptr<OpenGLVertexArrayObject> vao;
		std::string name;
		ysl::RGBASpectrum color;
		bool visible = true;
		ysl::Transform modelTransform;
		int indexCount;
	};

	ISOSurfaceRendererPrivate(ISOSurfaceRenderer * parent);

	void CreateScreenQuads();

	void CreateMesh(const std::shared_ptr<ysl::TriangleMesh> & mesh, OpenGLRenderingObject& aMesh);

	void CreateMesh(const std::string & fileName,const ysl::RGBASpectrum & color);

	void CreateMesh(const ysl::Point3f * vertices,
		const ysl::Vector3f * normals,
		const ysl::Point2f * textures, 
		int nVertex, 
		const int * indices, int nIndex,
		OpenGLRenderingObject * object);

	std::list<OpenGLRenderingObject>::iterator CreateMesh();

	void CreateImageListTexture(int width, int height);

	std::shared_ptr<OpenGLTexture> fragmentBufferListTexture;

	void CreateFragmentBufferList(int width, int height);

	std::shared_ptr<OpenGLBuffer> fragmentBufferListBuffer;

	void CreateHeadPointerImageInitializer(int width, int height);

	std::shared_ptr<OpenGLBuffer> initializer;	// a PBO 

	void CreateAtomicCounter();

	void ClearAtomicCounter();

	std::shared_ptr<OpenGLBuffer> atomicCounter;

	void InitShader();

	void SetShaderUniform();

	ysl::ShaderProgram testShader;

	ysl::ShaderProgram oitListShader;

	ysl::ShaderProgram oitRenderShader;

	ysl::ShaderProgram oitFragShader;

	ysl::ShaderProgram quadShader;

	void ResizeHeadPointerImage(int width, int height);

	void UpdateMatrix(int width, int height);

	void ResizeScreenQuads(int width, int height);

	void ResizeFragmentBufferList(int width, int height);

	void ResizeInitializer(int width, int height);

	void ClearHeadPointerImage();


	void DrawLegend(QPainter * p);

	void Cleanup();

	//void ClearImageListTexture();
	int windowWidth;
	int windowHeight;
	FocusCamera camera;
	ysl::Transform rotation;
	ysl::Transform proj;
	ysl::Transform ortho;
	ysl::Point2i lastMousePos;
	std::shared_ptr<OpenGLTexture> imageList;
	std::shared_ptr<OpenGLTexture> depthTexture;

	//std::shared_ptr<OpenGLFramebufferObject> framebuffer;

	OpenGLRenderingObject boundingBox;
	bool boundingBoxVisible = true;
	const std::size_t AverageFragmentPerPixel;
	float inner = 2, outer = 3;

	struct ScreenQuads
	{
		OpenGLVertexArrayObject vao;
		std::shared_ptr<OpenGLBuffer> vbo;
	} screenQuads;

	std::list<OpenGLRenderingObject> meshes;
	using ValueType = QPair<QHash<QString, std::list<OpenGLRenderingObject>::iterator>, QSharedPointer<MeshGenerator>> ;
	QHash<QString, ValueType> dataName2Data;

private:
	ISOSurfaceRenderer * const q_ptr;
	Q_DECLARE_PUBLIC(ISOSurfaceRenderer)
};
