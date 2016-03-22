#ifndef GL3INTERFACES_H
#define GL3INTERFACES_H
#include "GLCommon.h"
#define GL_COMPUTE_SHADER 0x91B9
#define GL_UNIFORM_BUFFER 0x8A11
#define GL_UNIFORM_BUFFER_BINDING 0x8A28
#define GL_SHADER_STORAGE_BUFFER 0x90D2
#define GL_SHADER_STORAGE_BUFFER_BINDING 0x90D3
#define GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT 0x00000001
#define GL_ELEMENT_ARRAY_BARRIER_BIT 0x00000002
#define GL_UNIFORM_BARRIER_BIT 0x00000004
#define GL_TEXTURE_FETCH_BARRIER_BIT 0x00000008
#define GL_SHADER_IMAGE_ACCESS_BARRIER_BIT 0x00000020
#define GL_COMMAND_BARRIER_BIT 0x00000040
#define GL_PIXEL_BUFFER_BARRIER_BIT 0x00000080
#define GL_TEXTURE_UPDATE_BARRIER_BIT 0x00000100
#define GL_BUFFER_UPDATE_BARRIER_BIT 0x00000200
#define GL_FRAMEBUFFER_BARRIER_BIT 0x00000400
#define GL_TRANSFORM_FEEDBACK_BARRIER_BIT 0x00000800
#define GL_ATOMIC_COUNTER_BARRIER_BIT 0x00001000

#define GLAPIENTRY GL_APIENTRY

typedef GLuint(GLAPIENTRY* PFNGLCREATESHADERPROGRAMVPROC)(
  GLenum type, GLsizei count, const GLchar* const* strings);
typedef void(GLAPIENTRY* PFNGLBINDBUFFERRANGEPROC)(GLenum target, GLuint index,
                                                   GLuint buffer,
                                                   GLintptr offset,
                                                   GLsizeiptr size);
typedef void(GLAPIENTRY* PFNGLBINDIMAGETEXTUREPROC)(GLuint unit, GLuint texture,
                                                    GLint level,
                                                    GLboolean layered,
                                                    GLint layer, GLenum access,
                                                    GLenum format);
typedef void(GLAPIENTRY* PFNGLMEMORYBARRIERPROC)(GLbitfield barriers);

struct GL3Interfaces
{
  PFNGLCREATESHADERPROGRAMVPROC glCreateShaderProgramv;
  PFNGLBINDBUFFERRANGEPROC glBindBufferRange;
  PFNGLBINDIMAGETEXTUREPROC glBindImageTexture;
  PFNGLMEMORYBARRIERPROC glMemoryBarrier;
};
#endif /* GL3INTERFACES_H */
