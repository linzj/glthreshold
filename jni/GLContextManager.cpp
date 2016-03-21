#include "GLContextManager.h"
#include "GLCommon.h"
#define EGL_OPENGL_ES3_BIT_KHR 0x00000040

GLContextManager::GLContextManager()
  : m_dpy(EGL_NO_DISPLAY)
  , m_context(EGL_NO_CONTEXT)
  , m_surface(EGL_NO_SURFACE)
{
}

GLContextManager::~GLContextManager()
{
  if (m_dpy == EGL_NO_DISPLAY)
    return;
  if (m_context != EGL_NO_CONTEXT) {
    eglDestroyContext(m_dpy, m_context);
  }
  if (m_surface != EGL_NO_SURFACE) {
    eglDestroySurface(m_dpy, m_surface);
  }
}

bool
GLContextManager::init()
{
  EGLConfig config;
  EGLSurface surface;
  int n;
  EGLDisplay dpy = eglGetDisplay(EGL_DEFAULT_DISPLAY);
  if (dpy == EGL_NO_DISPLAY) {
    return false;
  }

  static const GLint configAttribs[] = { EGL_SURFACE_TYPE,
                                         EGL_PBUFFER_BIT,
                                         EGL_RENDERABLE_TYPE,
                                         EGL_OPENGL_ES3_BIT_KHR,
                                         EGL_RED_SIZE,
                                         8,
                                         EGL_GREEN_SIZE,
                                         8,
                                         EGL_BLUE_SIZE,
                                         8,
                                         EGL_ALPHA_SIZE,
                                         8,
                                         EGL_NONE };

  if (!eglChooseConfig(dpy, configAttribs, &config, 1, &n)) {
    return false;
  }

  static const EGLint pbufAttribs[] = { EGL_WIDTH, 1, EGL_HEIGHT, 1, EGL_NONE };

  surface = eglCreatePbufferSurface(dpy, config, pbufAttribs);
  if (surface == EGL_NO_SURFACE) {
    return NULL;
  }

  static const GLint gl2ContextAttribs[] = { EGL_CONTEXT_CLIENT_VERSION, 3,
                                             EGL_NONE };

  EGLContext ctx =
    eglCreateContext(dpy, config, EGL_NO_CONTEXT, gl2ContextAttribs);
  if (ctx == EGL_NO_CONTEXT) {
    eglDestroySurface(dpy, surface);
    return false;
  }
  m_dpy = dpy;
  m_context = ctx;
  m_surface = surface;
  bool initGL3Success;
  {
    GLContextScope scope(*this);
    GLIMPROC_LOGI("glversion: %s, vender: %s, renderer: %s.\n",
                  glGetString(GL_VERSION), glGetString(GL_VENDOR),
                  glGetString(GL_RENDERER));
    initGL3Success = initGL3Interfaces();
  }
  if (!initGL3Success) {
    eglDestroySurface(dpy, surface);
    eglDestroyContext(dpy, ctx);
    // stop the duplicate clean up in destructor.
    m_dpy = EGL_NO_DISPLAY;
  }
  return true;
}

#define INIT_GL_FUNC_OR_RETURN_FALSE(name)                                     \
  __typeof__(static_cast<GL3Interfaces*>(nullptr)->name) name =                \
    reinterpret_cast<__typeof__(static_cast<GL3Interfaces*>(nullptr)->name)>(  \
      eglGetProcAddress(#name));                                               \
  if (name == nullptr) {                                                       \
    GLIMPROC_LOGE(                                                             \
      "GLContextManager::initGL3Interfaces: eglGetProcAddress " #name          \
      " failed.\n");                                                           \
    return false;                                                              \
  }                                                                            \
  m_interfaces.name = name

bool
GLContextManager::initGL3Interfaces()
{
  INIT_GL_FUNC_OR_RETURN_FALSE(glCreateShaderProgramv);
  return true;
}

GLContextScope::GLContextScope(GLContextManager& manager)
  : m_dpy(EGL_NO_DISPLAY)
{
  if (manager.m_dpy == EGL_NO_DISPLAY) {
    return;
  }
  m_dpy = manager.m_dpy;
  m_prevContext = eglGetCurrentContext();
  m_prevSurfaceDraw = eglGetCurrentSurface(EGL_DRAW);
  m_prevSurfaceRead = eglGetCurrentSurface(EGL_READ);
  eglMakeCurrent(m_dpy, manager.m_surface, manager.m_surface,
                 manager.m_context);
}

GLContextScope::~GLContextScope()
{
  if (m_dpy == EGL_NO_DISPLAY) {
    return;
  }
  eglMakeCurrent(m_dpy, m_prevSurfaceDraw, m_prevSurfaceRead, m_prevContext);
}
