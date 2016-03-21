#ifndef GL3INTERFACES_H
#define GL3INTERFACES_H
#include "GLCommon.h"
#define GL_COMPUTE_SHADER 0x91B9
#define GLAPIENTRY GL_APIENTRY

typedef GLuint(GLAPIENTRY* PFNGLCREATESHADERPROGRAMVPROC)(
  GLenum type, GLsizei count, const GLchar* const* strings);
struct GL3Interfaces
{
  PFNGLCREATESHADERPROGRAMVPROC glCreateShaderProgramv;
};
#endif /* GL3INTERFACES_H */
