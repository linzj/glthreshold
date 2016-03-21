#include "GLProgramManager.h"
#include "GL3Interfaces.h"

extern "C" {
extern const char* const binarizerSum;
}

namespace {
typedef std::unordered_map<GLuint, const char* const*> SourceMap;

static SourceMap
getSourceMap()
{
  static SourceMap g_map = {
    { GLProgramManager::BINARIZERSUM, &binarizerSum },
  };
  return g_map;
}
}

GLProgramManager::GLProgramManager(const GL3Interfaces& interfaces)
  : m_interfaces(interfaces)
{
}

GLProgramManager::~GLProgramManager()
{
  CHECK_CONTEXT_NOT_NULL();
  for (auto p : m_programs) {
    glDeleteProgram(p.second);
  }
}

static bool
checkProgram(GLuint program)
{
  GLint status;
  glGetProgramiv(program, GL_LINK_STATUS, &status);
  if (status == GL_FALSE) {
    GLint infoLogLength;
    char* infoLog;
    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infoLogLength);
    infoLog = (char*)malloc(infoLogLength);
    glGetProgramInfoLog(program, infoLogLength, NULL, infoLog);
    GLIMPROC_LOGE("link log: %s\n", infoLog);
    free(infoLog);
    return false;
  }
  return true;
}

GLuint
GLProgramManager::getProgram(GLProgramManager::ProgramType programType)
{
  auto found = m_programs.find(programType);
  if (found != m_programs.end()) {
    return found->second;
  }
  auto&& sourceMap = getSourceMap();
  auto foundSource = sourceMap.find(programType);
  if (foundSource == sourceMap.end()) {
    return 0;
  }
  GLuint program = m_interfaces.glCreateShaderProgramv(
    GL_COMPUTE_SHADER, 1, const_cast<const char**>(foundSource->second));
  if (!program) {
    return 0;
  }
  if (!checkProgram(program)) {
    glDeleteProgram(program);
    return 0;
  }
  m_programs.insert(std::make_pair(programType, program));
  return program;
}
