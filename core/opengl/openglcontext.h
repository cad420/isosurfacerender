#ifndef _OPENGLCONTEXT_H_
#define _OPENGLCONTEXT_H_

#include <memory>


class QOpenGLContext;
class OpenGLCurrentContext
{
public:
	OpenGLCurrentContext();
	OpenGLCurrentContext(const OpenGLCurrentContext &) = delete;
	OpenGLCurrentContext & operator=(const OpenGLCurrentContext&) = delete;
	OpenGLCurrentContext(OpenGLCurrentContext &&) = delete;
	OpenGLCurrentContext & operator=(OpenGLCurrentContext&&) = delete;
	bool IsValid()const;
	bool operator==(const OpenGLCurrentContext & ctx)const;
	bool operator!=(const OpenGLCurrentContext & ctx)const;
	static std::shared_ptr<OpenGLCurrentContext> GetCurrentOpenGLContext();
private:
	QOpenGLContext * ctx;
	//GLFWwindow * windowContext;

};

#endif
