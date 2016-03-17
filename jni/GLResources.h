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
#endif /* GLRESOURCES_H */
