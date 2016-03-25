#ifndef GLRESOURCES_H
#define GLRESOURCES_H
#include "GLCommon.h"

class GLTexture
{
public:
  explicit GLTexture(GLuint id);
  virtual ~GLTexture();
  inline GLuint id() { return m_id; }
protected:
  inline void reset() { m_id = 0; }
  GLuint m_id;
};

class GLBuffer
{
public:
  explicit GLBuffer(GLuint id, GLsizei size);
  virtual ~GLBuffer();
  inline GLuint id() { return m_id; }
  inline GLsizei size() { return m_size; }
protected:
  GLuint m_id;
  GLsizei m_size;
};
#endif /* GLRESOURCES_H */
