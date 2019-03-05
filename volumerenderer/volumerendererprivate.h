
#pragma once

#include <QtGlobal>


#include "volumerenderer.h"
#include "../isosurfacerender/mathematics/geometry.h"
#include "../isosurfacerender/mathematics/transformation.h"
#include "../isosurfacerender/cameras/camera.h"
#include "../isosurfacerender/opengl/texture.h"
#include "../isosurfacerender/opengl/framebuffer.h"
#include "../isosurfacerender/opengl/openglvertexarrayobject.h"
#include "../isosurfacerender/opengl/shader.h"

class QObject;

class VolumeRendererPrivate
{
public:
	VolumeRendererPrivate(QWidget * parent = nullptr);
	void ResizeScreenQuads(int width, int height);

	void InitFrameBuffer();
	void InitProxyGeometry();
	void InitShader();
	void CreateScreenQuads();
	void CreateTFTexture();
	void UpdateSize(int x, int y);
	
	int windowWidth;
	int windowHeight;
	FocusCamera camera;
	ysl::Transform rotation;
	ysl::Transform proj;
	ysl::Transform ortho;
	ysl::Point2i lastMousePos;

	ysl::ShaderProgram g_rayCastingShaderProgram;
	ysl::ShaderProgram g_positionShaderProgram;
	ysl::ShaderProgram g_quadsShaderProgram;

	struct ScreenQuads
	{
		OpenGLVertexArrayObject vao;
		std::shared_ptr<OpenGLBuffer> vbo;
	} screenQuads;

	std::shared_ptr<OpenGLTexture> volumeTexture;
	std::shared_ptr<OpenGLTexture> g_texTransferFunction;
	std::shared_ptr<OpenGLFramebufferObject> g_framebuffer;

	std::shared_ptr<OpenGLTexture> g_texEntryPos;
	std::shared_ptr<OpenGLTexture> g_texExitPos;
	std::shared_ptr<OpenGLTexture> g_texDepth;

	std::shared_ptr<OpenGLBuffer> g_proxyEBO;
	std::shared_ptr<OpenGLBuffer> g_proxyVBO;
	OpenGLVertexArrayObject g_proxyVAO;

private:
	VolumeRenderer * const q_ptr;
	Q_DECLARE_PUBLIC(VolumeRenderer);
};

