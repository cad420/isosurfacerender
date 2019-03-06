#include "volumerender.h"
#include "volumerenderprivate.h"

#include <QMouseEvent>
#include "cameras/camera.h"
#include <opengl/openglutils.h>

VolumeRenderer::VolumeRenderer(QWidget * parent) :
	QOpenGLWidget(parent),
	d_ptr(new VolumeRendererPrivate(800, 600,FocusCamera({0,0,10}),this))
{

}

void VolumeRenderer::setVolumeData(unsigned char * data, std::size_t width, std::size_t height, std::size_t depth, float xSpacing, float ySpacing, float zSpacing)
{
	Q_D(VolumeRenderer);
	makeCurrent();

	d->volumeTexture->SetData(OpenGLTexture::R8, OpenGLTexture::RED, OpenGLTexture::UInt8, width, height, depth, data);
	doneCurrent();
	// Update Matrix
	d->model = ysl::Scale(ysl::Vector3f(width*xSpacing,height*ySpacing,depth*zSpacing).Normalized());
}

void VolumeRenderer::setTransferFunction(float * transferFuncs)
{
	Q_D(VolumeRenderer);
	makeCurrent();
	d->g_texTransferFunction->SetData(OpenGLTexture::RGBA32F,
		OpenGLTexture::RGBA,
		OpenGLTexture::Float32,
		256, 0, 0, transferFuncs);
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

void VolumeRenderer::initializeGL()
{
	Q_D(VolumeRenderer);

	gl3wInit();
	if (!gl3wIsSupported(4, 4))
	{
		ysl::Error("OpenGL 4.4 or higher is needed.");
	}
	glClearColor(1, 1, 1, 1.0);

	d->InitShader();
	d->InitFrameBuffer();
	d->CreateScreenQuads();
	d->CreateVolumeTexture();
	d->CreateTFTexture();
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
	d->g_positionShaderProgram.setUniformValue("worldMatrix", ysl::Transform{}.Matrix());
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
	d->g_rayCastingShaderProgram.setUniformSampler("texStartPos", OpenGLTexture::TextureUnit0, *d->g_texEntryPos);
	d->g_rayCastingShaderProgram.setUniformSampler("texEndPos", OpenGLTexture::TextureUnit1, *d->g_texExitPos);
	d->g_rayCastingShaderProgram.setUniformSampler("texTransfunc", OpenGLTexture::TextureUnit2, *d->g_texTransferFunction);
	GL_ERROR_REPORT;
	d->g_rayCastingShaderProgram.setUniformValue("step", (float)0.01);
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
//	d->g_texExitPos->SaveAsImage(R"(d2.png)");

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
