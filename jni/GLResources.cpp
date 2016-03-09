#include "GLResources.h"

GLTexture::GLTexture(GLuint id)
  : m_id(id)
{
}

GLTexture::~GLTexture()
{
  if (m_id) {
    glDeleteTextures(1, &m_id);
  }
}
