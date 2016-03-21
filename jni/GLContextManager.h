#ifndef GLCONTEXTMANAGER_H
#define GLCONTEXTMANAGER_H
#include "GL3Interfaces.h"
#include <EGL/egl.h>
#include <memory>
#include <stdint.h>

class GLContextManager
{
public:
  GLContextManager();
  ~GLContextManager();
  inline const GL3Interfaces& getGL3Interfaces() { return m_interfaces; }
  bool init();

private:
  bool initGL3Interfaces();
  EGLDisplay m_dpy;
  EGLContext m_context;
  EGLSurface m_surface;
  GL3Interfaces m_interfaces;
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
