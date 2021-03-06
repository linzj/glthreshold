#include "GLProgramManager.h"

extern "C" {
extern const char* const gaussianFragRowSource;
extern const char* const gaussianFragColumnSource;
extern const char* const adaptiveThresholdFragSource;
extern const char* const dilateNonZeroRowSource;
extern const char* const dilateNonZeroColumnSource;
extern const char* const erodeNonZeroRowSource;
extern const char* const erodeNonZeroColumnSource;
extern const char* const thresholdSource;
extern const char* const vertexShaderSource;
}

static inline const char**
getVertexSourceLocation()
{
  return const_cast<const char**>(&vertexShaderSource);
}

namespace {
typedef std::unordered_map<GLuint, const char* const*> SourceMap;

static SourceMap
getSourceMap()
{
  static SourceMap g_map = {
    { GLProgramManager::GAUSSIANROW, &gaussianFragRowSource },
    { GLProgramManager::GAUSSIANCOLUMN, &gaussianFragColumnSource },
    { GLProgramManager::ADAPTIVETHRESHOLD, &adaptiveThresholdFragSource },
    { GLProgramManager::DILATENONZEROROW, &dilateNonZeroRowSource },
    { GLProgramManager::DILATENONZEROCOLUMN, &dilateNonZeroColumnSource },
    { GLProgramManager::ERODENONZEROROW, &erodeNonZeroRowSource },
    { GLProgramManager::ERODENONZEROCOLUMN, &erodeNonZeroColumnSource },
    { GLProgramManager::THRESHOLD, &thresholdSource },
  };
  return g_map;
}
}

GLProgramManager::GLProgramManager()
  : m_vertexShader(0)
{
}

GLProgramManager::~GLProgramManager()
{
  CHECK_CONTEXT_NOT_NULL();
  for (auto p : m_programs) {
    glDeleteProgram(p.second);
  }
  glDeleteShader(m_vertexShader);
}

static bool
compileAndCheck(GLuint shader)
{
  GLint status;
  glCompileShader(shader);
  glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
  if (status == GL_FALSE) {
    GLint infoLogLength;
    char* infoLog;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLogLength);
    infoLog = (char*)malloc(infoLogLength);
    glGetShaderInfoLog(shader, infoLogLength, NULL, infoLog);
    GLIMPROC_LOGE("compile log: %s\n", infoLog);
    free(infoLog);
    return false;
  }
  return true;
}

static GLuint
compileShaderSource(GLenum type, GLsizei count, char const** string)
{
  GLuint shader = glCreateShader(type);
  glShaderSource(shader, count, string, NULL);
  if (!compileAndCheck(shader)) {
    glDeleteShader(shader);
    return 0;
  }
  return shader;
}

static bool
linkAndCheck(GLuint program)
{
  GLint status;
  glLinkProgram(program);
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

static GLuint
createProgram(GLuint vertexShader, GLuint fragmentShader)
{
  GLuint program = glCreateProgram();
  if (vertexShader != 0) {
    glAttachShader(program, vertexShader);
  }
  if (fragmentShader != 0) {
    glAttachShader(program, fragmentShader);
  }
  if (!linkAndCheck(program)) {
    glDeleteProgram(program);
    return 0;
  }
  return program;
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
  GLuint fragShader = compileShaderSource(
    GL_FRAGMENT_SHADER, 1, const_cast<const char**>(foundSource->second));
  if (!fragShader) {
    return 0;
  }
  GLuint program = createProgram(m_vertexShader, fragShader);
  glDeleteShader(fragShader);
  if (!program) {
    return 0;
  }
  m_programs.insert(std::make_pair(programType, program));
  return program;
}

bool
GLProgramManager::init()
{
  GLuint vertexShader =
    compileShaderSource(GL_VERTEX_SHADER, 1, getVertexSourceLocation());
  if (vertexShader == 0) {
    return false;
  }
  m_vertexShader = vertexShader;
  return true;
}
