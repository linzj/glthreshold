#include "AdaptiveThresholdProcessor.h"
#include "GLResources.h"
#include "ImageProcessorWorkflow.h"
#include <cmath>
#include <stdio.h>
#include <stdlib.h>

AdaptiveThresholdProcessor::AdaptiveThresholdProcessor()
  : m_vPositionIndexRow(0)
  , m_uTextureRow(0)
  , m_uScreenGeometryRow(0)
  , m_uKernelRow(0)
  , m_programRow(0)
  , m_vPositionIndexColumn(0)
  , m_uTextureColumn(0)
  , m_uScreenGeometryColumn(0)
  , m_uKernelColumn(0)
  , m_programColumn(0)
  , m_maxValue(0)
  , m_vPositionIndexThreshold(0)
  , m_uTextureOrigThreshold(0)
  , m_uTextureBlurThreshold(0)
  , m_uScreenGeometryThresholdg(0)
  , m_uMaxValueThreshold(0)
  , m_programThreshold(0)
{
}

AdaptiveThresholdProcessor::~AdaptiveThresholdProcessor()
{
  if (m_programRow) {
    glDeleteProgram(m_programRow);
  }
  if (m_programColumn) {
    glDeleteProgram(m_programColumn);
  }
  if (m_programThreshold) {
    glDeleteProgram(m_programThreshold);
  }
}

bool
AdaptiveThresholdProcessor::init(int maxValue)
{
  m_maxValue = maxValue;
  initGaussianBlurKernel();
  return initProgram();
}

std::vector<GLfloat>
AdaptiveThresholdProcessor::getGaussianKernel(int n)
{
  std::vector<GLfloat> kernel(n);
  float* cf = const_cast<float*>(kernel.data());

  double sigmaX = ((n - 1) * 0.5 - 1) * 0.3 + 0.8;
  double scale2X = -0.5 / (sigmaX * sigmaX);
  double sum = 0;

  int i;
  for (i = 0; i < n; i++) {
    double x = i - (n - 1) * 0.5;
    double t = std::exp(scale2X * x * x);
    {
      cf[i] = (float)t;
      sum += cf[i];
    }
  }

  sum = 1. / sum;
  for (i = 0; i < n; i++) {
    cf[i] = (float)(cf[i] * sum);
  }

  return kernel;
}

void
AdaptiveThresholdProcessor::initGaussianBlurKernel()
{
  m_kernel = std::move(getGaussianKernel(s_block_size));
}

extern "C" {
extern const char* gaussianFragRowSource;
extern const char* gaussianFragColumnSource;
extern const char* thresholdFragSource;
}

/* report GL errors, if any, to stderr */
static bool
checkError(const char* functionName)
{
  GLenum error;
  bool entered = false;
  while ((error = glGetError()) != GL_NO_ERROR) {
    fprintf(stderr, "GL error 0x%X detected in %s\n", error, functionName);
    entered = true;
  }
  return !entered;
}

static void
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
    fprintf(stderr, "compile log: %s\n", infoLog);
    free(infoLog);
  }
}

static GLuint
compileShaderSource(GLenum type, GLsizei count, char const** string)
{
  GLuint shader = glCreateShader(type);
  glShaderSource(shader, count, string, NULL);
  compileAndCheck(shader);
  return shader;
}

static void
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
    fprintf(stderr, "link log: %s\n", infoLog);
    free(infoLog);
  }
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
  linkAndCheck(program);
  return program;
}

bool
AdaptiveThresholdProcessor::initProgram()
{
  GLuint vertexShader =
    compileShaderSource(GL_VERTEX_SHADER, 1, getVertexSourceLocation());

  GLuint fragmentRowShader =
    compileShaderSource(GL_FRAGMENT_SHADER, 1, &gaussianFragRowSource);

  GLuint fragmentColumnShader =
    compileShaderSource(GL_FRAGMENT_SHADER, 1, &gaussianFragColumnSource);

  m_programRow = createProgram(vertexShader, fragmentRowShader);
  m_programColumn = createProgram(vertexShader, fragmentColumnShader);
  GLint program = m_programRow;
  m_vPositionIndexRow = glGetAttribLocation(program, "v_position");
  printf("m_vPositionIndexRow: %d.\n", m_vPositionIndexRow);
  m_uTextureRow = glGetUniformLocation(program, "u_texture");
  m_uScreenGeometryRow = glGetUniformLocation(program, "u_screenGeometry");
  m_uKernelRow = glGetUniformLocation(program, "u_kernel");
  printf("m_uTextureRow: %d, m_uScreenGeometryRow: %d, m_uKernelRow: %d.\n",
         m_uTextureRow, m_uScreenGeometryRow, m_uKernelRow);

  program = m_programColumn;

  m_vPositionIndexColumn = glGetAttribLocation(program, "v_position");
  printf("m_vPositionIndexColumn: %d.\n", m_vPositionIndexColumn);
  m_uTextureColumn = glGetUniformLocation(program, "u_texture");
  m_uScreenGeometryColumn = glGetUniformLocation(program, "u_screenGeometry");
  m_uKernelColumn = glGetUniformLocation(program, "u_kernel");
  printf("m_uTextureColumn: %d, m_uScreenGeometryColumn: %d, m_uKernelColumn: "
         "%d.\n",
         m_uTextureColumn, m_uScreenGeometryColumn, m_uKernelColumn);

  GLuint fragmentThreshold =
    compileShaderSource(GL_FRAGMENT_SHADER, 1, &thresholdFragSource);
  m_programThreshold = createProgram(vertexShader, fragmentThreshold);
  glDeleteShader(fragmentThreshold);
  program = m_programThreshold;

  m_vPositionIndexThreshold = glGetAttribLocation(program, "v_position");
  printf("m_vPositionIndexThreshold: %d.\n", m_vPositionIndexThreshold);
  m_uTextureOrigThreshold = glGetUniformLocation(program, "u_textureOrig");
  m_uTextureBlurThreshold = glGetUniformLocation(program, "u_textureBlur");
  m_uScreenGeometryThresholdg =
    glGetUniformLocation(program, "u_screenGeometry");
  m_uMaxValueThreshold = glGetUniformLocation(program, "u_maxValue");
  printf("m_uTextureOrigThreshold: %d, m_uTextureBlurThreshold: %d, "
         "m_uScreenGeometryThreshold: %d, m_uMaxValueThreshold: %d.\n",
         m_uTextureOrigThreshold, m_uTextureBlurThreshold,
         m_uScreenGeometryThresholdg, m_uMaxValueThreshold);
  // clean up
  glDeleteShader(vertexShader);
  glDeleteShader(fragmentRowShader);
  glDeleteShader(fragmentColumnShader);
  return checkError("initProgram");
}

ProcessorOutput
AdaptiveThresholdProcessor::process(const ProcessorInput& pin)
{
  static float positions[][4] = {
    { -1.0, 1.0, 0.0, 1.0 },
    { -1.0, -1.0, 0.0, 1.0 },
    { 1.0, 1.0, 0.0, 1.0 },
    { 1.0, -1.0, 0.0, 1.0 },
  };
  ImageProcessorWorkflow* wf = pin.wf;
  FBOScope fboscope(wf);
  // zero for row blur, one for column blur
  // zero additionally used as the final threshold output.
  std::shared_ptr<GLTexture> tmpTexture[2] = {
    wf->requestTextureForFramebuffer(), wf->requestTextureForFramebuffer()
  };

  // bind fbo and complete it.
  wf->setColorAttachmentForFramebuffer(tmpTexture[0]->id());

  if (GL_FRAMEBUFFER_COMPLETE != wf->checkFramebuffer()) {
    fprintf(stderr, "fbo is not completed %d, %x.\n", __LINE__,
            glCheckFramebufferStatus(GL_FRAMEBUFFER));
    exit(1);
  }
  GLint imageGeometry[2] = { pin.width, pin.height };
  glVertexAttribPointer(m_vPositionIndexRow, 4, GL_FLOAT, GL_FALSE, 0,
                        positions);
  glEnableVertexAttribArray(m_vPositionIndexRow);
  glUseProgram(m_programRow);
  // setup uniforms
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, pin.color->id());
  glUniform1i(m_uTextureRow, 0);

  glUniform2iv(m_uScreenGeometryRow, 1, imageGeometry);
  // setup kernel and block size

  glUniform4fv(m_uKernelRow, s_block_size / 4, m_kernel.data());
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
  // bind fbo and complete it.
  wf->setColorAttachmentForFramebuffer(tmpTexture[1]->id());
  if (GL_FRAMEBUFFER_COMPLETE != wf->checkFramebuffer()) {
    fprintf(stderr, "fbo is not completed %d.\n", __LINE__);
    exit(1);
  }
  glDisableVertexAttribArray(m_vPositionIndexRow);

  glVertexAttribPointer(m_vPositionIndexColumn, 4, GL_FLOAT, GL_FALSE, 0,
                        positions);
  glEnableVertexAttribArray(m_vPositionIndexColumn);
  glUseProgram(m_programColumn);
  // setup uniforms
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, tmpTexture[0]->id());
  glUniform1i(m_uTextureColumn, 0);

  glUniform2iv(m_uScreenGeometryColumn, 1, imageGeometry);
  // setup kernel and block size

  glUniform4fv(m_uKernelColumn, s_block_size / 4, m_kernel.data());
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
  glDisableVertexAttribArray(m_vPositionIndexColumn);
  // render to screen

  glEnableVertexAttribArray(m_vPositionIndexThreshold);

  glUseProgram(m_programThreshold);
  // setup uniforms
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, pin.color->id());
  glUniform1i(m_uTextureOrigThreshold, 0);
  glActiveTexture(GL_TEXTURE0 + 1);
  glBindTexture(GL_TEXTURE_2D, tmpTexture[1]->id());
  glUniform1i(m_uTextureBlurThreshold, 1);

  glUniform2iv(m_uScreenGeometryThresholdg, 1, imageGeometry);
  glUniform1f(m_uMaxValueThreshold, static_cast<float>(m_maxValue) / 255.0f);

  wf->setColorAttachmentForFramebuffer(tmpTexture[0]->id());
  if (GL_FRAMEBUFFER_COMPLETE != wf->checkFramebuffer()) {
    fprintf(stderr, "fbo is not completed %d.\n", __LINE__);
    exit(1);
  }
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
  glDisableVertexAttribArray(m_vPositionIndexThreshold);
  checkError("image process");
  return ProcessorOutput{ tmpTexture[0] };
}
