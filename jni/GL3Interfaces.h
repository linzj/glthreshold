#ifndef GL3INTERFACES_H
#define GL3INTERFACES_H
#include "GLCommon.h"
#define GL_MAX_SHADER_STORAGE_BUFFER_BINDINGS 0x90DD
#define GL_SHADER_STORAGE_BUFFER_OFFSET_ALIGNMENT 0x90DF
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
#define GL_READ_ONLY 0x88B8
#define GL_WRITE_ONLY 0x88B9
#define GL_RGBA8 0x8058

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

typedef void(GLAPIENTRY* PFNGLDISPATCHCOMPUTEPROC)(GLuint num_groups_x,
                                                   GLuint num_groups_y,
                                                   GLuint num_groups_z);
typedef void(GLAPIENTRY* PFNGLTEXSTORAGE2DPROC)(GLenum target, GLsizei levels,
                                                GLenum internalformat,
                                                GLsizei width, GLsizei height);

struct GL3Interfaces
{
  PFNGLCREATESHADERPROGRAMVPROC glCreateShaderProgramv;
  PFNGLBINDBUFFERRANGEPROC glBindBufferRange;
  PFNGLBINDIMAGETEXTUREPROC glBindImageTexture;
  PFNGLMEMORYBARRIERPROC glMemoryBarrier;
  PFNGLDISPATCHCOMPUTEPROC glDispatchCompute;
  PFNGLTEXSTORAGE2DPROC glTexStorage2D;
};
#endif /* GL3INTERFACES_H */
