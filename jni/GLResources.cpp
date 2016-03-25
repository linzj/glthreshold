#include "GLResources.h"

GLTexture::GLTexture(GLuint id)
  : m_id(id)
{
}

GLTexture::~GLTexture()
{
  if (m_id) {
    CHECK_CONTEXT_NOT_NULL();
    glDeleteTextures(1, &m_id);
  }
}

GLBuffer::GLBuffer(GLuint id, GLsizei size)
  : m_id(id)
  , m_size(size)
{
}

GLBuffer::~GLBuffer()
{
  if (m_id) {
    CHECK_CONTEXT_NOT_NULL();
    glDeleteBuffers(1, &m_id);
  }
}
