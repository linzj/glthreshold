#ifndef GLPROGRAMMANAGER_H
#define GLPROGRAMMANAGER_H
#include "GLCommon.h"
#include <unordered_map>
class GL3Interfaces;

class GLProgramManager
{
public:
  enum ProgramType
  {
    BINARIZERSUM,
    BINARIZERFIRSTPEAK,
    BINARIZERSECONDSCORE,
    BINARIZERSECONDPEAK,
    BESTVALLEYSCORE,
    BESTVALLEY,
    BINARIZERASSIGN,
  };
  explicit GLProgramManager(const GL3Interfaces& interfaces);
  ~GLProgramManager();
  GLuint getProgram(ProgramType programType);

private:
  GLuint doGetProgram(ProgramType programType);
  std::unordered_map<GLuint, GLuint> m_programs;
  const GL3Interfaces& m_interfaces;
};

#endif /* GLPROGRAMMANAGER_H */
