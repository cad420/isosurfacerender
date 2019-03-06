
#include "openglcontext.h"

#include <QOpenGLContext>

OpenGLCurrentContext::OpenGLCurrentContext():ctx(QOpenGLContext::currentContext())
{

}

bool OpenGLCurrentContext::IsValid() const
{
	return ctx;
}

bool OpenGLCurrentContext::operator==(const OpenGLCurrentContext& ctx) const
{
	return this->ctx == ctx.ctx;
}

bool OpenGLCurrentContext::operator!=(const OpenGLCurrentContext& ctx) const
{
	return !(ctx == *this);
}

std::shared_ptr<OpenGLCurrentContext> OpenGLCurrentContext::GetCurrentOpenGLContext()
{
	return std::make_shared<OpenGLCurrentContext>();
}
