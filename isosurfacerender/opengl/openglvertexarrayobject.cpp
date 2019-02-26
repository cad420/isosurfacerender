
#include "openglvertexarrayobject.h"
#include "gl3w/GL/gl3w.h"
#include "openglcontext.h"
#include "utility/error.h"


OpenGLVertexArrayObject::OpenGLVertexArrayObject():m_vbo(0)
{

}

void OpenGLVertexArrayObject::create()
{
	if (!OpenGLCurrentContext::GetCurrentOpenGLContext()->IsValid())
		ysl::Error("No current OpenGL Context");
	glGenVertexArrays(1, &m_vbo);
}

void OpenGLVertexArrayObject::bind()
{
	if(!m_vbo)
		return;
	glBindVertexArray(m_vbo);
}

void OpenGLVertexArrayObject::unbind()
{
	glBindVertexArray(0);
}

void OpenGLVertexArrayObject::destroy()
{
	glDeleteVertexArrays(1,&m_vbo);
}

