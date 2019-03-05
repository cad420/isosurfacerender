
#include "volumerendererprivate.h"
#include "../isosurfacerender/opengl/openglutils.h"



const static int g_proxyGeometryVertexIndices[] = { 1,3,7,1,7,5,0,4,6,0,6,2,2,6,7,2,7,3,0,1,5,0,5,4,4,5,7,4,7,6,0,2,3,0,3,1 };
static const float xCoord = 1.0, yCoord = 1.0, zCoord = 1.0;
static float g_proxyGeometryVertices[] = {
	0,0,0,
	xCoord, 0.0, 0.0 ,
	0 , yCoord , 0 ,
	xCoord , yCoord , 0 ,
	0 , 0 , zCoord  ,
	xCoord , 0 , zCoord ,
	0 , yCoord , zCoord,
	xCoord , yCoord , zCoord ,
		-0.5,0 - 0.5,0 - 0.5,
	xCoord - 0.5, 0.0 - 0.5, 0.0 - 0.5 ,
	0 - 0.5 , yCoord - 0.5, 0 - 0.5,
	xCoord - 0.5, yCoord - 0.5 , 0 - 0.5,
	0 - 0.5, 0 - 0.5 , zCoord - 0.5 ,
	xCoord - 0.5 , 0 - 0.5, zCoord - 0.5,
	0 - 0.5 , yCoord - 0.5, zCoord - 0.5,
	xCoord - 0.5, yCoord - 0.5, zCoord - 0.5,
};



VolumeRendererPrivate::VolumeRendererPrivate(QWidget * parent) :q_ptr(parent),
windowWidth(800),
windowHeight(600)
{

}

void VolumeRendererPrivate::ResizeScreenQuads(int width, int height)
{
	if (!screenQuads.vbo)
		return;

}

void VolumeRendererPrivate::InitFrameBuffer()
{

	g_texEntryPos = OpenGLTexture::CreateTexture2DRect(OpenGLTexture::RGBA32F,
		OpenGLTexture::Linear,
		OpenGLTexture::Linear,
		OpenGLTexture::ClampToEdge,
		OpenGLTexture::ClampToEdge,
		OpenGLTexture::RGBA,
		OpenGLTexture::Float32,
		windowWidth,
		windowHeight,
		nullptr);

	g_texExitPos = OpenGLTexture::CreateTexture2DRect(OpenGLTexture::RGBA32F,
		OpenGLTexture::Linear,
		OpenGLTexture::Linear,
		OpenGLTexture::ClampToEdge,
		OpenGLTexture::ClampToEdge,
		OpenGLTexture::RGBA,
		OpenGLTexture::Float32,
		windowWidth,
		windowHeight,
		nullptr);

	g_texDepth = OpenGLTexture::CreateTexture2DRect(OpenGLTexture::InternalDepthComponent,
		OpenGLTexture::Linear,
		OpenGLTexture::Linear,
		OpenGLTexture::ClampToEdge,
		OpenGLTexture::ClampToEdge,
		OpenGLTexture::ExternalDepthComponent,
		OpenGLTexture::Float32,
		windowWidth,
		windowHeight,
		nullptr);




	g_framebuffer = std::shared_ptr<OpenGLFramebufferObject>(new OpenGLFramebufferObject);
	g_framebuffer->Bind();
	g_framebuffer->AttachTexture(OpenGLFramebufferObject::ColorAttachment0, g_texEntryPos);
	g_framebuffer->AttachTexture(OpenGLFramebufferObject::ColorAttachment1, g_texExitPos);
	g_framebuffer->CheckFramebufferStatus();
	g_framebuffer->Unbind();
}

void VolumeRendererPrivate::InitProxyGeometry()
{
	g_proxyVAO.create();
	g_proxyVAO.bind();

	g_proxyVBO = std::make_shared<OpenGLBuffer>(OpenGLBuffer::VertexArrayBuffer);
	g_proxyVBO->Bind();
	g_proxyVBO->AllocateFor(g_proxyGeometryVertices, sizeof(g_proxyGeometryVertices));

	//glEnableVertexAttribArray(0);
	//glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), reinterpret_cast<void*>(sizeof(g_proxyGeometryVertices) / 2));

	g_proxyVBO->VertexAttribPointer(0, 3, OpenGLBuffer::Float, false, sizeof(float) * 3, reinterpret_cast<void*>(sizeof(g_proxyGeometryVertices) / 2));
	GL_ERROR_REPORT;

	//glEnableVertexAttribArray(1);
	//glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), reinterpret_cast<void*>(0));
	g_proxyVBO->VertexAttribPointer(1, 3, OpenGLBuffer::Float, false, 3 * sizeof(float), reinterpret_cast<void*>(0));

	GL_ERROR_REPORT;

	g_proxyEBO = std::make_shared<OpenGLBuffer>(OpenGLBuffer::ElementArrayBuffer);
	g_proxyEBO->Bind();
	g_proxyEBO->AllocateFor(g_proxyGeometryVertexIndices, sizeof(g_proxyGeometryVertexIndices));
	GL_ERROR_REPORT;

	g_proxyVAO.unbind();


}

void VolumeRendererPrivate::InitShader()
{
	g_rayCastingShaderProgram.create();
	g_rayCastingShaderProgram.addShaderFromFile("raycasting_v.glsl", ysl::ShaderProgram::ShaderType::Vertex);
	g_rayCastingShaderProgram.addShaderFromFile("raycasting_f.glsl", ysl::ShaderProgram::ShaderType::Fragment);
	g_rayCastingShaderProgram.link();

	// Generate entry and exit position
	g_positionShaderProgram.create();
	g_positionShaderProgram.addShaderFromFile("position_v.glsl", ysl::ShaderProgram::ShaderType::Vertex);
	g_positionShaderProgram.addShaderFromFile("position_f.glsl", ysl::ShaderProgram::ShaderType::Fragment);
	g_positionShaderProgram.link();

	//  Draw the result texture into the default framebuffer
	g_quadsShaderProgram.create();
	g_quadsShaderProgram.addShaderFromFile("quadsshader_v.glsl", ysl::ShaderProgram::ShaderType::Vertex);
	g_quadsShaderProgram.addShaderFromFile("quadsshader_f.glsl", ysl::ShaderProgram::ShaderType::Fragment);
	g_quadsShaderProgram.link();

}

void VolumeRendererPrivate::CreateScreenQuads()
{
	screenQuads.vao.create();
	screenQuads.vao.bind();
	screenQuads.vbo = std::make_shared<OpenGLBuffer>(OpenGLBuffer::BufferTarget::VertexArrayBuffer);
	screenQuads.vbo->Bind();

	float screenSize[] =
	{
		0,0,
		windowWidth,0,
		0,windowHeight,
		0,windowHeight,
		windowWidth,0,
		windowWidth,windowHeight
	};

	screenQuads.vbo->AllocateFor(screenSize, sizeof(screenSize));
	screenQuads.vbo->VertexAttribPointer(0,
		2,
		OpenGLBuffer::Float,
		false,
		sizeof(float) * 2,
		reinterpret_cast<void*>(0));
	screenQuads.vbo->Unbind();
	screenQuads.vao.unbind();

}

void VolumeRendererPrivate::CreateTFTexture()
{

	g_texTransferFunction = OpenGLTexture::CreateTexture1D(OpenGLTexture::RGBA32F,
		OpenGLTexture::Linear, OpenGLTexture::Linear, OpenGLTexture::ClampToEdge, OpenGLTexture::RGBA, OpenGLTexture::Float32, 256, nullptr);
}

void VolumeRendererPrivate::UpdateSize(int x, int y)
{
	Q_Q(VolumeRenderer);

	g_texEntryPos->SetData(OpenGLTexture::RGBA32F,
		OpenGLTexture::RGBA,
		OpenGLTexture::Float32,
		x, y, 0, nullptr);
	g_texExitPos->SetData(OpenGLTexture::RGBA32F,
		OpenGLTexture::RGBA,
		OpenGLTexture::Float32,
		x, y, 0, nullptr);
	g_texDepth->SetData(OpenGLTexture::InternalDepthComponent,
		OpenGLTexture::ExternalDepthComponent,
		OpenGLTexture::Float32, x, y, 0, nullptr);

	proj.SetPerspective(45.0f, float(x) / y, 0.01, 1000);
	ortho.SetOrtho(0, x, 0, y, -10, 100);

	screenQuads.vbo->Bind();
	float screenSize[] =
	{
		0,0,
		x,0,
		0,y,
		0,y,
		x,0,
		x,y
	};
	screenQuads.vbo->SetSubData(screenSize, sizeof(screenSize), 0);
	screenQuads.vbo->Unbind();

}
