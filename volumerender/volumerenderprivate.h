#pragma once

#include "mathematics/geometry.h"
#include "mathematics/transformation.h"
#include "cameras/camera.h"
#include "opengl/texture.h"
#include "opengl/framebuffer.h"
#include "opengl/openglvertexarrayobject.h"
#include "opengl/shader.h"
#include <qglobal.h>


class VolumeRenderer;
class FocusCamera;


class VolumeRendererPrivate
{
public:

	VolumeRendererPrivate(int width, int height,const FocusCamera & camera,VolumeRenderer * parent = nullptr);
	void ResizeScreenQuads(int width, int height);
	void InitFrameBuffer();
	void InitProxyGeometry();
	void InitShader();
	void CreateVolumeTexture();
	
	void CreateScreenQuads();
	void CreateTFTexture();
	void UpdateSize(int x, int y);

	int windowWidth;
	int windowHeight;
	FocusCamera camera;
	ysl::Transform model;
	ysl::Transform proj;
	ysl::Transform ortho;
	ysl::Point2i lastMousePos;

	ysl::ShaderProgram g_rayCastingShaderProgram;
	ysl::ShaderProgram g_positionShaderProgram;
	//ysl::ShaderProgram g_quadsShaderProgram;

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

	float ka, kd, ks, shininess;

private:
	VolumeRenderer * const q_ptr;
	Q_DECLARE_PUBLIC(VolumeRenderer)
};

