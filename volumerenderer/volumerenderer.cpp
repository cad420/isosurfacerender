#include "volumerenderer.h"
#include "volumerendererprivate.h"

#include "../isosurfacerender/opengl/openglutils.h"
#include "../isosurfacerender/mathematics/transformation.h"

#include <QMouseEvent>

VolumeRenderer::VolumeRenderer(QWidget * parent) :QOpenGLWidget(this),d_ptr(new VolumeRendererPrivate(this))
{

}

void VolumeRenderer::setVolumeData(unsigned char * data, std::size_t width, std::size_t height, std::size_t depth, float xSpacing, float ySpacing, float zSpacing)
{
	Q_D(VolumeRenderer);
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

void VolumeRenderer::initializeGL()
{
	Q_D(VolumeRenderer);
	d->InitShader();
	d->InitFrameBuffer();
	d->CreateScreenQuads();
	d->InitProxyGeometry();
}

void VolumeRenderer::paintGL()
{

	Q_D(VolumeRenderer);

	glClear(GL_COLOR_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
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
	glDepthFunc(GL_LESS);

	d->g_positionShaderProgram.unbind();
	d->g_framebuffer->Unbind();

	d->g_rayCastingShaderProgram.bind();
	d->g_rayCastingShaderProgram.setUniformValue("viewMatrix", d->camera.view().Matrix());
	d->g_rayCastingShaderProgram.setUniformValue("orthoMatrix", d->ortho.Matrix());
	d->g_rayCastingShaderProgram.setUniformSampler("texStartPos", OpenGLTexture::TextureUnit0, *d->g_texEntryPos);
	d->g_rayCastingShaderProgram.setUniformSampler("texEndPos", OpenGLTexture::TextureUnit1, *d->g_texExitPos);
	d->g_rayCastingShaderProgram.setUniformSampler("texTransfunc", OpenGLTexture::TextureUnit2, *d->g_texTransferFunction);
	d->g_rayCastingShaderProgram.setUniformValue("step", (float)0.01);

	d->g_rayCastingShaderProgram.setUniformValue("ka", (float)0.5);
	d->g_rayCastingShaderProgram.setUniformValue("ks", (float)0.5);
	d->g_rayCastingShaderProgram.setUniformValue("kd", (float)0.5);
	d->g_rayCastingShaderProgram.setUniformValue("shininess", int(500));


	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glDrawArrays(GL_TRIANGLES, 0, 6);			// Draw into default framebuffer
	glDisable(GL_DEPTH_TEST);


	d->g_rayCastingShaderProgram.unbind();
}

void VolumeRenderer::resizeGL(int w, int h)
{
	Q_D(VolumeRenderer);
	if (!d)return;
	d->windowWidth = w;
	d->windowHeight = h;
	d->UpdateSize(w, h);
	//d->UpdateMatrix(d->windowWidth, d->windowHeight);
	update();
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
