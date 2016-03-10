#ifndef GLPROGRAMMANAGER_H
#define GLPROGRAMMANAGER_H
#include "GLCommon.h"
#include <unordered_map>

class GLProgramManager
{
public:
  enum ProgramType
  {
    GAUSSIANROW,
    GAUSSIANCOLUMN,
    THRESHOLD,
  };
  GLProgramManager();
  ~GLProgramManager();
  bool init();
  GLuint getProgram(ProgramType programType);

private:
  std::unordered_map<GLuint, GLuint> m_programs;
  GLuint m_vertexShader;
};

#endif /* GLPROGRAMMANAGER_H */
