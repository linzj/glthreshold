#include "ImageProcessor.h"
#include <cmath>
#include <stdio.h>
#include <stdlib.h>

ImageProcessor::ImageProcessor()
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

ImageProcessor::~ImageProcessor()
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
ImageProcessor::init(int maxValue)
{
  m_maxValue = maxValue;
  initGaussianBlurKernel();
  return initProgram();
}

std::vector<GLfloat>
ImageProcessor::getGaussianKernel(int n)
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
ImageProcessor::initGaussianBlurKernel()
{
  m_kernel = std::move(getGaussianKernel(s_block_size));
}

static const char* vertexShaderSource = "attribute vec4 v_position;\n"
                                        "void main()\n"
                                        "{\n"
                                        "   gl_Position = v_position;\n"
                                        "}\n";

static const char* gaussianFragRowSource =
  "uniform sampler2D u_texture;\n"
  "uniform ivec2 u_screenGeometry;\n"
  "uniform mediump float u_kernel[91];\n"
  "const int c_blockSize = 91;\n"
  "\n"
  "void main(void)\n"
  "{\n"
  "    int i;\n"
  "    highp vec2 texcoord = (gl_FragCoord.xy - vec2(c_blockSize / 2, 0)) / "
  "vec2(u_screenGeometry);\n"
  "    highp float toffset = 1.0 / float(u_screenGeometry.x);\n"
  "    mediump vec3 color = texture2D(u_texture, texcoord).rgb * "
  "vec3(u_kernel[0]);\n"
  "    for (i = 1; i < c_blockSize; ++i) {\n"
  "        color += texture2D(u_texture, texcoord + vec2(float(i) * toffset, "
  "0.0)).rgb * vec3(u_kernel[i]);\n"
  "    }\n"
  "    gl_FragColor = vec4(color, 1.0);\n"
  "}\n";

static const char* gaussianFragColumnSource =
  "uniform sampler2D u_texture;\n"
  "uniform ivec2 u_screenGeometry;\n"
  "uniform mediump float u_kernel[91];\n"
  "const int c_blockSize = 91;\n"
  "\n"
  "void main(void)\n"
  "{\n"
  "    int i;\n"
  "    highp vec2 texcoord = (gl_FragCoord.xy + vec2(0, c_blockSize / 2)) / "
  "vec2(u_screenGeometry);\n"
  "    highp float toffset = 1.0 / float(u_screenGeometry.y);\n"
  "    mediump vec3 color = texture2D(u_texture, texcoord).rgb * "
  "vec3(u_kernel[0]);\n"
  "    for (i = 1; i < c_blockSize; ++i) {\n"
  "        color += texture2D(u_texture, texcoord - vec2(0.0, float(i) * "
  "toffset)).rgb * vec3(u_kernel[i]);\n"
  "    }\n"
  "    gl_FragColor = vec4(color, 1.0);\n"
  "}\n";

static const char* thresholdFragSource =
  "\n"
  "uniform mediump float u_maxValue;\n"
  "uniform ivec2 u_screenGeometry;\n"
  "uniform sampler2D u_textureOrig;\n"
  "uniform sampler2D u_textureBlur;\n"
  "\n"
  "void main(void)\n"
  "{\n"
  "    highp vec2 texcoord = gl_FragCoord.xy / vec2(u_screenGeometry);\n"
  "    mediump vec3 colorOrig = texture2D(u_textureOrig, texcoord).rgb;\n"
  "    mediump vec3 colorBlur = texture2D(u_textureBlur, texcoord).rgb;\n"
  "    mediump vec3 result;\n"
  "    result.r = colorOrig.r > colorBlur.r ? u_maxValue : 0.0;\n"
  "    result.g = colorOrig.g > colorBlur.g ? u_maxValue : 0.0;\n"
  "    result.b = colorOrig.b > colorBlur.b ? u_maxValue : 0.0;\n"
  "    gl_FragColor = vec4(result, 1.0);\n"
  "}\n";

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
ImageProcessor::initProgram()
{
  GLuint vertexShader =
    compileShaderSource(GL_VERTEX_SHADER, 1, &vertexShaderSource);

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

std::unique_ptr<uint8_t[]>
ImageProcessor::process(const ImageDesc& desc)
{
  static float positions[][4] = {
    { -1.0, 1.0, 0.0, 1.0 },
    { -1.0, -1.0, 0.0, 1.0 },
    { 1.0, 1.0, 0.0, 1.0 },
    { 1.0, -1.0, 0.0, 1.0 },
  };
  GLuint tmpFramebuffer;
  // zero for row blur, one for column blur, two for input image.
  // zero additionally used as the final threshold output.
  GLuint tmpTexture[3];
  GLint oldViewport[4];
  glGetIntegerv(GL_VIEWPORT, oldViewport);
  glViewport(0, 0, desc.width, desc.height);
  // allocate fbo and a texture
  glGenFramebuffers(1, &tmpFramebuffer);
  glGenTextures(3, tmpTexture);
  allocateTexture(tmpTexture[0], desc.width, desc.height, GL_RGBA);
  allocateTexture(tmpTexture[1], desc.width, desc.height, GL_RGBA);
  allocateTexture(tmpTexture[2], desc.width, desc.height, desc.format,
                  desc.data);

  // bind fbo and complete it.
  glBindFramebuffer(GL_FRAMEBUFFER, tmpFramebuffer);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                         tmpTexture[0], 0);

  if (GL_FRAMEBUFFER_COMPLETE != glCheckFramebufferStatus(GL_FRAMEBUFFER)) {
    fprintf(stderr, "fbo is not completed %d, %x.\n", __LINE__,
            glCheckFramebufferStatus(GL_FRAMEBUFFER));
    exit(1);
  }
  GLint imageGeometry[2] = { desc.width, desc.height };
  glVertexAttribPointer(m_vPositionIndexRow, 4, GL_FLOAT, GL_FALSE, 0,
                        positions);
  glEnableVertexAttribArray(m_vPositionIndexRow);
  glUseProgram(m_programRow);
  // setup uniforms
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, tmpTexture[2]);
  glUniform1i(m_uTextureRow, 0);

  glUniform2iv(m_uScreenGeometryRow, 1, imageGeometry);
  // setup kernel and block size

  glUniform1fv(m_uKernelRow, s_block_size, m_kernel.data());
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
  // bind fbo and complete it.
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                         tmpTexture[1], 0);
  if (GL_FRAMEBUFFER_COMPLETE != glCheckFramebufferStatus(GL_FRAMEBUFFER)) {
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
  glBindTexture(GL_TEXTURE_2D, tmpTexture[0]);
  glUniform1i(m_uTextureColumn, 0);

  glUniform2iv(m_uScreenGeometryColumn, 1, imageGeometry);
  // setup kernel and block size

  glUniform1fv(m_uKernelColumn, s_block_size, m_kernel.data());
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
  glDisableVertexAttribArray(m_vPositionIndexColumn);
  // render to screen

  glEnableVertexAttribArray(m_vPositionIndexThreshold);

  glUseProgram(m_programThreshold);
  // setup uniforms
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, tmpTexture[2]);
  glUniform1i(m_uTextureOrigThreshold, 0);
  glActiveTexture(GL_TEXTURE0 + 1);
  glBindTexture(GL_TEXTURE_2D, tmpTexture[1]);
  glUniform1i(m_uTextureBlurThreshold, 1);

  glUniform2iv(m_uScreenGeometryThresholdg, 1, imageGeometry);
  glUniform1f(m_uMaxValueThreshold, static_cast<float>(m_maxValue) / 255.0f);

  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                         tmpTexture[0], 0);
  if (GL_FRAMEBUFFER_COMPLETE != glCheckFramebufferStatus(GL_FRAMEBUFFER)) {
    fprintf(stderr, "fbo is not completed %d.\n", __LINE__);
    exit(1);
  }
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
  glDisableVertexAttribArray(m_vPositionIndexThreshold);
  glPixelStorei(GL_PACK_ALIGNMENT, 4);
  std::unique_ptr<uint8_t[]> readback(
    new uint8_t[desc.width * desc.height * 4]);
  glReadPixels(0, 0, desc.width, desc.height, GL_RGBA, GL_UNSIGNED_BYTE,
               readback.get());
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glDeleteTextures(3, tmpTexture);
  glDeleteFramebuffers(1, &tmpFramebuffer);
  glViewport(oldViewport[0], oldViewport[1], oldViewport[2], oldViewport[3]);
  checkError("image process");
  return readback;
}

void
ImageProcessor::allocateTexture(GLuint texture, GLint width, GLint height,
                                GLenum format, void* data)
{
  glBindTexture(GL_TEXTURE_2D, texture);
  glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format,
               GL_UNSIGNED_BYTE, data);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
}
