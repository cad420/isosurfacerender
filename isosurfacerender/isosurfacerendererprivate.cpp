

#include "isosurfacerendererprivate.h"
#include "isosurfacerender.h"
#include "utility/error.h"
#include "utility/objreader.h"
#include "opengl/openglutils.h"
#include "algorithm/marchingcubes.h"

#include <QPainter>

//#define TESTSHADER



ISOSurfaceRendererPrivate::ISOSurfaceRendererPrivate(ISOSurfaceRenderer* parent) :
q_ptr(parent), 
camera({ 0.0,0.0,50 }),
windowWidth(800),
windowHeight(600), 
AverageFragmentPerPixel(15)
{
	
}

void ISOSurfaceRendererPrivate::CreateScreenQuads()
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

void ISOSurfaceRendererPrivate::CreateMesh(const std::shared_ptr<ysl::TriangleMesh>& mesh, ISOSurfaceRendererPrivate::OpenGLRenderingObject& aMesh)
{

	Q_Q(ISOSurfaceRenderer);
	q->makeCurrent();

	//aMesh.vao->create();
	aMesh.vao->bind();

	const auto vertexSize = mesh->Vertices() ? mesh->VertexCount() * sizeof(ysl::Point3f):0;
	const auto normalSize = mesh->Normals()? mesh->VertexCount() * sizeof(ysl::Vector3f):0;
	const auto textureSize = mesh->Textures()? mesh->VertexCount() * sizeof(ysl::Point2f):0;

	aMesh.vbo->Bind();
	aMesh.vbo->AllocateFor(nullptr, vertexSize + normalSize + textureSize);
	aMesh.vbo->SetSubData(mesh->Vertices(), vertexSize, 0);
	aMesh.vbo->SetSubData(mesh->Normals(), normalSize, vertexSize);
	aMesh.vbo->SetSubData(mesh->Textures(), textureSize, vertexSize + normalSize);

	// Vertex Attribute
	aMesh.vbo->VertexAttribPointer(0, 3, OpenGLBuffer::Float, false, sizeof(ysl::Point3f), nullptr);
	// Normal Attribute
	aMesh.vbo->VertexAttribPointer(1, 3, OpenGLBuffer::Float, false, sizeof(ysl::Vector3f), reinterpret_cast<void*>(vertexSize));
	// Texture Attribute
	aMesh.vbo->VertexAttribPointer(2, 2, OpenGLBuffer::Float, false, sizeof(ysl::Point2f), reinterpret_cast<void*>(vertexSize + normalSize));

	aMesh.ebo->Bind();
	aMesh.indexCount = mesh->TriangleCount()*3;
	aMesh.ebo->AllocateFor(mesh->Indices(),aMesh.indexCount*sizeof(int));

	aMesh.vao->unbind();
	aMesh.ebo->Unbind();
	aMesh.vbo->Unbind();
	

	q->doneCurrent();
}

void ISOSurfaceRendererPrivate::CreateMesh(const std::string & fileName,const ysl::RGBASpectrum & color)
{
	ysl::ObjReader reader;

	reader.Load(fileName);

	if (!reader.IsLoaded())
		ysl::Warning("%s cannot be loaded\n", fileName.c_str());

	OpenGLRenderingObject aMesh;

	aMesh.vao = std::make_shared<OpenGLVertexArrayObject>();
	aMesh.vao->create();
	aMesh.vao->bind();

	aMesh.vbo = std::make_shared<OpenGLBuffer>(OpenGLBuffer::BufferTarget::VertexArrayBuffer);

	const auto vertexSize = reader.vertexBytes();
	const auto normalSize = reader.normalBytes();
	const auto textureSize = reader.textureBytes();


	aMesh.vbo->Bind();
	aMesh.vbo->AllocateFor(nullptr, vertexSize + normalSize + textureSize);
	aMesh.vbo->SetSubData(reader.getVertices().data(), vertexSize, 0);
	aMesh.vbo->SetSubData(reader.getNormals().data(), normalSize, vertexSize);
	aMesh.vbo->SetSubData(reader.getTextureCoord().data(), textureSize, vertexSize + normalSize);

	// Vertex Attribute
	aMesh.vbo->VertexAttribPointer(0, 3, OpenGLBuffer::Float, false, sizeof(ysl::Point3f), nullptr);
	// Normal Attribute
	aMesh.vbo->VertexAttribPointer(1, 3, OpenGLBuffer::Float, false, sizeof(ysl::Vector3f), reinterpret_cast<void*>(vertexSize));
	// Texture Attribute
	aMesh.vbo->VertexAttribPointer(2, 2, OpenGLBuffer::Float, false, sizeof(ysl::Point2f), reinterpret_cast<void*>(vertexSize + normalSize));


	aMesh.ebo = std::make_shared<OpenGLBuffer>(OpenGLBuffer::BufferTarget::ElementArrayBuffer);
	aMesh.ebo->Bind();
	aMesh.indexCount = reader.getFaceIndices().size();
	aMesh.ebo->AllocateFor(reader.getFaceIndices().data(), reader.vertexIndicesBytes());

	aMesh.vao->unbind();
	aMesh.ebo->Unbind();
	aMesh.vbo->Unbind();


	//float c[4] = { 1,0.1,0, 0.6};
	aMesh.color = color;
	meshes.push_back(aMesh);
}

void ISOSurfaceRendererPrivate::CreateMesh(const ysl::Point3f * vertices,
	const ysl::Vector3f * normals,
	const ysl::Point2f * textures,
	int nVertex,
	const int * indices, int nIndex,
	OpenGLRenderingObject * object)
{
	object->vao->bind();

	const auto vertexSize = vertices ? nVertex * sizeof(ysl::Point3f) : 0;
	const auto normalSize = normals ?nVertex * sizeof(ysl::Vector3f) : 0;
	const auto textureSize = textures ? nVertex * sizeof(ysl::Point2f) : 0;

	object->vbo->Bind();
	object->vbo->AllocateFor(nullptr, vertexSize + normalSize + textureSize);
	object->vbo->SetSubData(vertices, vertexSize, 0);
	object->vbo->SetSubData(normals, normalSize, vertexSize);
	object->vbo->SetSubData(textures, textureSize, vertexSize + normalSize);

	// Vertex Attribute
	object->vbo->VertexAttribPointer(0, 3, OpenGLBuffer::Float, false, sizeof(ysl::Point3f), nullptr);
	// Normal Attribute
	object->vbo->VertexAttribPointer(1, 3, OpenGLBuffer::Float, false, sizeof(ysl::Vector3f), reinterpret_cast<void*>(vertexSize));
	// Texture Attribute
	object->vbo->VertexAttribPointer(2, 2, OpenGLBuffer::Float, false, sizeof(ysl::Point2f), reinterpret_cast<void*>(vertexSize + normalSize));

	object->ebo->Bind();
	object->indexCount = nIndex;
	object->ebo->AllocateFor(indices, nIndex * sizeof(int));

	object->vao->unbind();
	object->ebo->Unbind();
	object->vbo->Unbind();

}


std::list<ISOSurfaceRendererPrivate::OpenGLRenderingObject>::iterator ISOSurfaceRendererPrivate::CreateMesh()
{
	Q_Q(ISOSurfaceRenderer);
	q->makeCurrent();

	meshes.push_back(OpenGLRenderingObject());
	auto iter = --meshes.end();

	iter->vao = std::make_shared<OpenGLVertexArrayObject>();
	iter->vao->create();
	iter->vbo = std::make_shared<OpenGLBuffer>(OpenGLBuffer::BufferTarget::VertexArrayBuffer);
	iter->ebo = std::make_shared<OpenGLBuffer>(OpenGLBuffer::BufferTarget::ElementArrayBuffer);


	q->doneCurrent();
	return iter;
}

void ISOSurfaceRendererPrivate::CreateImageListTexture(int width, int height)
{
	imageList = OpenGLTexture::CreateTexture2DRect(OpenGLTexture::R32UI,
		OpenGLTexture::FilterMode::Linear,
		OpenGLTexture::FilterMode::Linear,
		OpenGLTexture::WrapMode::ClampToEdge,
		OpenGLTexture::WrapMode::ClampToEdge,
		OpenGLTexture::REDInteger, OpenGLTexture::UInt32, width, height,
		nullptr);
	GL_ERROR_REPORT;
	imageList->BindToDataImage(1, 0, false, 0, OpenGLTexture::ReadAndWrite, OpenGLTexture::R32UI);
}

void ISOSurfaceRendererPrivate::CreateFragmentBufferList(int width, int height)
{

	const auto listBufferSize = std::size_t(width)*height * AverageFragmentPerPixel * 4 * sizeof(unsigned int);

	fragmentBufferListBuffer = std::make_shared<OpenGLBuffer>(OpenGLBuffer::ShaderStorageBuffer, OpenGLBuffer::BufferUsage::DynamicCopy);
	GL_ERROR_REPORT;
	fragmentBufferListBuffer->AllocateFor(nullptr, listBufferSize);
	GL_ERROR_REPORT;
	fragmentBufferListBuffer->BindBufferBase(3);
	GL_ERROR_REPORT;
	fragmentBufferListBuffer->Unbind();
	//fragmentBufferListTexture = std::make_shared<OpenGLTexture>(OpenGLTexture::TextureTarget::TextureBuffer);
	//fragmentBufferList = OpenGLTexture::CreateTextureBuffer(OpenGLTexture::R32UI, OpenGLTexture::Linear, OpenGLTexture::Linear, OpenGLTexture::ClampToEdge, OpenGLTexture::RED, OpenGLTexture::UInt32, size, nullptr);
	//fragmentBufferList->BindToDataImage(2, 0, false, 0, OpenGLTexture::ReadAndWrite, OpenGLTexture::R32UI);
	GL_ERROR_REPORT;
}

void ISOSurfaceRendererPrivate::CreateHeadPointerImageInitializer(int width, int height)
{
	initializer = std::make_shared<OpenGLBuffer>(OpenGLBuffer::BufferTarget::PixelUnpackBuffer);
	GL_ERROR_REPORT;
	ResizeInitializer(width, height);
}

void ISOSurfaceRendererPrivate::CreateAtomicCounter()
{
	atomicCounter = std::make_shared<OpenGLBuffer>(OpenGLBuffer::BufferTarget::AtomicCounterBuffer);

	atomicCounter->BindBufferBase(0);

	ClearAtomicCounter();
}

void ISOSurfaceRendererPrivate::ClearAtomicCounter()
{
	assert(atomicCounter);
	auto zero = 0;
	atomicCounter->Bind();
	atomicCounter->AllocateFor(&zero, sizeof(int));
	atomicCounter->Unbind();
	GL_ERROR_REPORT;
}

void ISOSurfaceRendererPrivate::InitShader()
{

#ifdef TESTSHADER
	testShader.create();
	testShader.addShaderFromFile(R"(phoneshadingvert.glsl)", ysl::ShaderProgram::ShaderType::Vertex);
	testShader.addShaderFromFile(R"(phoneshadingfrag.glsl)", ysl::ShaderProgram::ShaderType::Fragment);
	testShader.link();
#else
	oitListShader.create();
	oitListShader.addShaderFromFile(R"(phoneshadingvert.glsl)", ysl::ShaderProgram::ShaderType::Vertex);
	//oitListShader.addShaderFromFile(R"(vertex_tes.glsl)", ysl::ShaderProgram::ShaderType::EvaluateTessellation);
	//oitListShader.addShaderFromFile(R"(vertex_tcs.glsl)", ysl::ShaderProgram::ShaderType::ControlTessellation);
	oitListShader.addShaderFromFile(R"(oitphase1_f.glsl)", ysl::ShaderProgram::ShaderType::Fragment);
	oitListShader.link();

	oitRenderShader.create();
	oitRenderShader.addShaderFromFile(R"(screenquads_v.glsl)", ysl::ShaderProgram::ShaderType::Vertex);
	oitRenderShader.addShaderFromFile(R"(oitphase2_f.glsl)", ysl::ShaderProgram::ShaderType::Fragment);
	oitRenderShader.link();


#endif

}

void ISOSurfaceRendererPrivate::SetShaderUniform()
{
	testShader.bind();
	testShader.setUniformValue("light_pos", ysl::Vector3f{ 0.f,0.f,10.f });
	testShader.setUniformValue("light_color", ysl::Vector3f{ 1.0,1.0,1.0 });
	testShader.setUniformValue("object_color", ysl::RGBSpectrum{ 0.3 });
	testShader.setUniformValue("view_pos", camera.position());
}

void ISOSurfaceRendererPrivate::ResizeHeadPointerImage(int width, int height)
{
	if (!imageList)
		return;
	imageList->SetData(OpenGLTexture::R32UI,
		OpenGLTexture::REDInteger,
		OpenGLTexture::UInt32,
		width,
		height,
		0,
		nullptr);
}

void ISOSurfaceRendererPrivate::UpdateMatrix(int width, int height)
{
	proj.SetPerspective(45.0f, float(width) / height, 0.01, 1000);
	ortho.SetOrtho(0, width, 0, height,-10 , 100);
}

void ISOSurfaceRendererPrivate::ResizeScreenQuads(int width, int height)
{
	if (!screenQuads.vbo)
		return;
	screenQuads.vbo->Bind();
	float screenSize[] =
	{
		0,0,
		width,0,
		0,height,
		0,height,
		width,0,
		width,height
	};
	screenQuads.vbo->SetSubData(screenSize, sizeof(screenSize), 0);
	screenQuads.vbo->Unbind();
}

void ISOSurfaceRendererPrivate::ResizeFragmentBufferList(int width, int height)
{
	//assert(fragmentBufferListBuffer);
	if (!fragmentBufferListBuffer)
		return;
	//fragmentBufferList->Bind();
	const auto size = std::size_t(width)*height * AverageFragmentPerPixel * 4 * sizeof(unsigned int);
	//fragmentBufferListBuffer->SetData(OpenGLTexture::R32UI, OpenGLTexture::RED, OpenGLTexture::UInt32, size, 0, 0, nullptr);
	//fragmentBufferListBuffer->SetSize(size);
	fragmentBufferListBuffer->AllocateFor(nullptr, size);
	//fragmentBufferList->AllocateFor(nullptr, std::size_t(width)*height * 3 * 4 * sizeof(unsigned int));
	//GL_ERROR_REPORT;
	fragmentBufferListBuffer->Unbind();
}

void ISOSurfaceRendererPrivate::ResizeInitializer(int width, int height)
{
	//assert(initializer);
	if (!initializer)
		return;
	const auto listHeaderImageSize = std::size_t(width)*height * sizeof(unsigned int);
	GL_ERROR_REPORT;
	initializer->Bind();
	GL_ERROR_REPORT;
	initializer->AllocateFor(nullptr, listHeaderImageSize);
	GL_ERROR_REPORT;
	auto ptr = initializer->Map(OpenGLBuffer::ReadWrite);
	memset(ptr, 0xFF, listHeaderImageSize);
	initializer->Unmap();
	GL_ERROR_REPORT;
}

void ISOSurfaceRendererPrivate::ClearHeadPointerImage()
{
	//assert(initializer);
	//assert(imageList);
	if (!initializer || !imageList)
		return;

	initializer->Bind();
	GL_ERROR_REPORT
	imageList->Bind();
	GL_ERROR_REPORT
	imageList->SetSubData(OpenGLTexture::REDInteger,
		OpenGLTexture::UInt32,
		0, windowWidth, 0,windowHeight,
		0, 0,
		nullptr);
	GL_ERROR_REPORT
	/// TODO:: Add a Fence ?
	imageList->Unbind();
	initializer->Unbind();
	GL_ERROR_REPORT
}

void ISOSurfaceRendererPrivate::DrawLegend(QPainter * p)
{
	Q_Q(ISOSurfaceRenderer);
	const auto fm = p->fontMetrics();
	const auto fontHeight = fm.ascent() + fm.descent();
	int index = 0;
	p->setFont({"Times", 10,QFont::Bold});
	for(const auto & m:meshes)
	{
		if(m.visible)
		{
			const auto name = QString::fromStdString(m.name);
			const auto fontWidth = fm.width(name);
			const int xpos = fontHeight + 10;
			const int ypos = 10 + (fontHeight + 5) * index;
			index++;

			p->drawText(xpos, ypos + fm.ascent(), name);
			p->fillRect(QRect(QPoint(xpos - fontHeight - 5, ypos), QSize(fontHeight, fontHeight)), QColor(m.color[0] * 255, m.color[1] * 255, m.color[2] * 255));

		}

	}

	
}

void ISOSurfaceRendererPrivate::Cleanup()
{

}


