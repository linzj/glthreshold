#include "GLCommon.h"
#include <stdio.h>

bool
checkError(const char* functionName)
{
  GLenum error;
  bool entered = false;
  while ((error = glGetError()) != GL_NO_ERROR) {
    fprintf(stderr, "GL error 0x%X detected in %s\n", error, functionName);
    entered = true;
  }
  return !entered;
}
