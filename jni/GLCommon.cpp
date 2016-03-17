#include "GLCommon.h"
#include <EGL/egl.h>
#include <stdlib.h>

bool
checkError(const char* functionName)
{
  GLenum error;
  bool entered = false;
  while ((error = glGetError()) != GL_NO_ERROR) {
    GLIMPROC_LOGE("GL error 0x%X detected in %s\n", error, functionName);
    entered = true;
  }
  return !entered;
}

void
checkContextNotNull(int line, const char* file)
{
  if (eglGetCurrentContext() == nullptr) {
    GLIMPROC_LOGE("file: %s, lineno: %d: no context here, crash now!", file,
                  line);
    abort();
  }
}
