#include "volumerender.h"
#include "volumerenderprivate.h"

#include <QMouseEvent>
#include "cameras/camera.h"
#include <opengl/openglutils.h>

VolumeRenderer::VolumeRenderer(QWidget * parent) :
	QOpenGLWidget(parent),
	d_ptr(new VolumeRendererPrivate(800, 600, FocusCamera({ 0,0,10 }), this))
{

}

void VolumeRenderer::setVolumeData(unsigned char * data, std::size_t width, std::size_t height, std::size_t depth, float xSpacing, float ySpacing, float zSpacing)
{
	Q_D(VolumeRenderer);
	makeCurrent();
	d->volumeSize = ysl::Size3(width, height, depth);
	d->volumeTexture->SetData(OpenGLTexture::R8, OpenGLTexture::RED, OpenGLTexture::UInt8, width, height, depth, data);

	// Make a default mask
	//const auto bytes = width * height*depth;
	//std::unique_ptr<unsigned char[]> mask(new unsigned char[bytes]);
	doneCurrent();
	// Update Matrix
	d->model = ysl::Scale(ysl::Vector3f(width*xSpacing, height*ySpacing, depth*zSpacing).Normalized());
}

void VolumeRenderer::setMask(unsigned char* mask, const QVector<int>& masks)
{
	Q_D(VolumeRenderer);
	makeCurrent();
	if (masks.empty())
	{

		char exist[256];
		memset(exist, 0, 256);
		const auto voxels = d->volumeSize.x * d->volumeSize.y * d->volumeSize.z;
		for (int i = 0; i < voxels; i++)
		{
			if (exist[mask[i]] == 0)
				exist[mask[i]] = 1;
		}
		QVector<int> m;
		for (int i = 0; i < 256; i++)
			if (exist[i])
				m.push_back(i);

		d->CreateMask2TFIndexMapping(m);
	}
	else
	{
		d->CreateMask2TFIndexMapping(masks);
	}

	d->maskTexture->SetData(OpenGLTexture::R8,
		OpenGLTexture::RED,
		OpenGLTexture::UInt8, 
		d->volumeSize.x, 
		d->volumeSize.y, 
		d->volumeSize.z, mask);

	doneCurrent();
}

//void VolumeRenderer::setTransferFunction(float * transferFuncs)
//{
//	Q_D(VolumeRenderer);
//	makeCurrent();
//	//d->g_texTransferFunction->SetData(OpenGLTexture::RGBA32F,
//	//	OpenGLTexture::RGBA,
//	//	OpenGLTexture::Float32,
//	//	256, 0, 0, transferFuncs);
//	d->g_texTransferFunction->SetSubData(OpenGLTexture::RGBA,
//		OpenGLTexture::Float32, 0, 256, 0, 1, 0, 0, transferFuncs);
//	doneCurrent();
//}

void VolumeRenderer::setTransferFunction(int mask, float* transferFuncs)
{
	Q_D(VolumeRenderer);
	makeCurrent();
	d->g_texTransferFunction->SetSubData(OpenGLTexture::RGBA,
		OpenGLTexture::Float32, 0, 256, d->mask2TFindex[mask], 1, 0, 0, transferFuncs);
	d->g_iTexTransferFunction->SetSubData(OpenGLTexture::RGBA,
		OpenGLTexture::Float32, 0, 256, d->mask2TFindex[mask], 1, 0, 0, transferFuncs);
	GL_ERROR_REPORT;
	//std::cout << d->mask2TFindex[mask] << std::endl;
	doneCurrent();
}

void VolumeRenderer::setIlluminationParams(float ka, float kd, float ks, float shininess)
{
	Q_D(VolumeRenderer);
	d->ka = ka;
	d->kd = kd;
	d->ks = ks;
	d->shininess = shininess;
	update();
}

static void gl_debug_msg_callback(GLenum source, GLenum type, GLuint id, GLenum severity,
	GLsizei length, const GLchar* message, void* userParam)
{
	(void)userParam;

	if (id == 131169 || id == 131185 || id == 131218 || id == 131204) return;

	std::cout << "---------------" << std::endl;
	std::cout << "Debug message (" << id << "): " << message << std::endl;

	switch (source)
	{
	case GL_DEBUG_SOURCE_API:             std::cout << "Source: API"; break;
	case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   std::cout << "Source: Window System"; break;
	case GL_DEBUG_SOURCE_SHADER_COMPILER: std::cout << "Source: Shader Compiler"; break;
	case GL_DEBUG_SOURCE_THIRD_PARTY:     std::cout << "Source: Third Party"; break;
	case GL_DEBUG_SOURCE_APPLICATION:     std::cout << "Source: Application"; break;
	case GL_DEBUG_SOURCE_OTHER:           std::cout << "Source: Other"; break;
	} std::cout << std::endl;

	switch (type)
	{
	case GL_DEBUG_TYPE_ERROR:               std::cout << "Type: Error"; break;
	case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: std::cout << "Type: Deprecated Behaviour"; break;
	case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  std::cout << "Type: Undefined Behaviour"; break;
	case GL_DEBUG_TYPE_PORTABILITY:         std::cout << "Type: Portability"; break;
	case GL_DEBUG_TYPE_PERFORMANCE:         std::cout << "Type: Performance"; break;
	case GL_DEBUG_TYPE_MARKER:              std::cout << "Type: Marker"; break;
	case GL_DEBUG_TYPE_PUSH_GROUP:          std::cout << "Type: Push Group"; break;
	case GL_DEBUG_TYPE_POP_GROUP:           std::cout << "Type: Pop Group"; break;
	case GL_DEBUG_TYPE_OTHER:               std::cout << "Type: Other"; break;
	} std::cout << std::endl;

	switch (severity)
	{
	case GL_DEBUG_SEVERITY_HIGH:         std::cout << "Severity: high"; break;
	case GL_DEBUG_SEVERITY_MEDIUM:       std::cout << "Severity: medium"; break;
	case GL_DEBUG_SEVERITY_LOW:          std::cout << "Severity: low"; break;
	case GL_DEBUG_SEVERITY_NOTIFICATION: std::cout << "Severity: notification"; break;
	} std::cout << std::endl;
	std::cout << std::endl;
}

void VolumeRenderer::initializeGL()
{
	Q_D(VolumeRenderer);

	gl3wInit();
	if (!gl3wIsSupported(4, 4))
	{
		ysl::Error("OpenGL 4.4 or higher is needed.");
	}

	//
#ifndef NDBUG
	glEnable(GL_DEBUG_OUTPUT);
	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
	glDebugMessageCallback((GLDEBUGPROC)gl_debug_msg_callback, NULL);
	glDebugMessageControlARB(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, GL_TRUE);
#endif
	//


	glClearColor(1, 1, 1, 1.0);

	d->InitShader();
	d->InitFrameBuffer();
	d->CreateScreenQuads();
	d->CreateVolumeTexture();
	d->CreateVolumeMask();
	//d->CreateTFTexture();
	//d->CreateTFTextureArray(5);
	d->InitProxyGeometry();
}

void VolumeRenderer::paintGL()
{
	Q_D(VolumeRenderer);
	glClear(GL_COLOR_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	GL_ERROR_REPORT;
	// clear intermediate result
	d->g_framebuffer->Bind();
	d->g_proxyVAO.bind();
	// Cull face
	d->g_positionShaderProgram.bind();
	d->g_positionShaderProgram.setUniformValue("projMatrix", d->proj.Matrix());
	d->g_positionShaderProgram.setUniformValue("worldMatrix", d->model.Matrix());
	d->g_positionShaderProgram.setUniformValue("viewMatrix", d->camera.view().Matrix());
	glDrawBuffer(GL_COLOR_ATTACHMENT0);					// Draw into attachment 0
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	GL_ERROR_REPORT;
	// Draw frontGL_ERROR_REPORT;
	glDepthFunc(GL_LESS);
	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

	// Draw Back
	glDrawBuffer(GL_COLOR_ATTACHMENT1);					// Draw into attachment 1
	glClear(GL_COLOR_BUFFER_BIT);						// Do not clear depth buffer here.
	glDepthFunc(GL_GREATER);
	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

	GL_ERROR_REPORT;
	d->g_positionShaderProgram.unbind();
	//d->g_framebuffer->Unbind();
	glBindFramebuffer(GL_FRAMEBUFFER, defaultFramebufferObject());

	glDepthFunc(GL_LESS);
	GL_ERROR_REPORT;
	d->g_rayCastingShaderProgram.bind();
	GL_ERROR_REPORT;
	d->g_rayCastingShaderProgram.setUniformValue("viewMatrix", d->camera.view().Matrix());
	d->g_rayCastingShaderProgram.setUniformValue("orthoMatrix", d->ortho.Matrix());

	d->g_rayCastingShaderProgram.setUniformSampler("texVolume", OpenGLTexture::TextureUnit3, *d->volumeTexture);
	d->g_rayCastingShaderProgram.setUniformSampler("maskVolumeTex", OpenGLTexture::TextureUnit4, *d->maskTexture);
	//glBindImageTexture(0, d->maskTexture->NativeTextureId(), 0, false, 0, GL_READ_ONLY, GL_R8UI);
	d->g_rayCastingShaderProgram.setUniformSampler("texStartPos", OpenGLTexture::TextureUnit0, *d->g_texEntryPos);
	d->g_rayCastingShaderProgram.setUniformSampler("texEndPos", OpenGLTexture::TextureUnit1, *d->g_texExitPos);
	if(d->g_texTransferFunction)
		d->g_rayCastingShaderProgram.setUniformSampler("texTransfunc", OpenGLTexture::TextureUnit2, *d->g_texTransferFunction);
	if (d->g_iTexTransferFunction)
		d->g_rayCastingShaderProgram.setUniformSampler("iTexTransfunc", OpenGLTexture::TextureUnit5, *d->g_iTexTransferFunction);
	GL_ERROR_REPORT;
	d->g_rayCastingShaderProgram.setUniformValue("step", (float)0.001);
	GL_ERROR_REPORT;
	d->g_rayCastingShaderProgram.setUniformValue("ka", d->ka);
	d->g_rayCastingShaderProgram.setUniformValue("ks", d->kd);
	d->g_rayCastingShaderProgram.setUniformValue("kd", d->ks);
	d->g_rayCastingShaderProgram.setUniformValue("shininess", d->shininess);
	GL_ERROR_REPORT;
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	d->screenQuads.vao.bind();
	glEnable(GL_BLEND);
	glBlendFunc(GL_DST_ALPHA, GL_ONE_MINUS_DST_ALPHA);
	glDrawArrays(GL_TRIANGLES, 0, 6);			// Draw into default framebuffer
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);
	GL_ERROR_REPORT;
	d->screenQuads.vao.unbind();
	d->g_rayCastingShaderProgram.unbind();

	//d->g_texEntryPos->SaveAsImage(R"(d1.png)");
	//d->g_texExitPos->SaveAsImage(R"(d2.png)");

}

void VolumeRenderer::resizeGL(int w, int h)
{
	Q_D(VolumeRenderer);
	if (!d)return;
	d->windowWidth = w;
	d->windowHeight = h;
	d->UpdateSize(w, h);
	update();
}

VolumeRenderer::~VolumeRenderer()
{
}

void VolumeRenderer::mouseMoveEvent(QMouseEvent* event)
{
	Q_D(VolumeRenderer);

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
	}
	else if (event->buttons() == Qt::MouseButton::RightButton)
	{
		const auto directionEx = d->camera.front()*dy;
		d->camera.movement(directionEx, 0.01);
	}
	d->lastMousePos = p;
	update();
}

void VolumeRenderer::mousePressEvent(QMouseEvent * event)
{
	Q_D(VolumeRenderer);
	const auto p = event->pos();
	d->lastMousePos = ysl::Point2i{ p.x(),p.y() };
}

void VolumeRenderer::mouseReleaseEvent(QMouseEvent* event)
{

}
