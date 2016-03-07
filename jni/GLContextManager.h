#ifndef GLCONTEXTMANAGER_H
#define GLCONTEXTMANAGER_H
#include <EGL/egl.h>

class GLContextManager
{
public:
  GLContextManager();
  ~GLContextManager();
  bool init();

private:
  EGLDisplay m_dpy;
  EGLContext m_context;
  EGLSurface m_surface;
  friend class GLContextScope;
};

class GLContextScope
{
public:
  GLContextScope(GLContextManager& manager);
  GLContextScope(const GLContextScope& manager) = delete;
  GLContextScope& operator=(const GLContextScope& manager) = delete;
  ~GLContextScope();

private:
  EGLDisplay m_dpy;
  EGLContext m_prevContext;
  EGLSurface m_prevSurfaceDraw;
  EGLSurface m_prevSurfaceRead;
};

#endif /* GLCONTEXTMANAGER_H */
