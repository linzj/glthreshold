#include "AdaptiveThresholdProcessor.h"
#include "GLProgramManager.h"
#include "GLResources.h"
#include "ImageProcessorWorkflow.h"
#include <cmath>
#include <stdlib.h>

AdaptiveThresholdProcessor::AdaptiveThresholdProcessor()
  : m_uTextureRow(0)
  , m_uScreenGeometryRow(0)
  , m_uKernelRow(0)
  , m_programRow(0)
  , m_uTextureColumn(0)
  , m_uScreenGeometryColumn(0)
  , m_uKernelColumn(0)
  , m_programColumn(0)
  , m_maxValue(0)
  , m_uTextureOrigThreshold(0)
  , m_uTextureBlurThreshold(0)
  , m_uScreenGeometryThresholdg(0)
  , m_uMaxValueThreshold(0)
  , m_programThreshold(0)
{
}

bool
AdaptiveThresholdProcessor::init(GLProgramManager* pm, int maxValue)
{
  m_maxValue = maxValue;
  initGaussianBlurKernel();
  return initProgram(pm);
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

bool
AdaptiveThresholdProcessor::initProgram(GLProgramManager* pm)
{
  m_programRow = pm->getProgram(GLProgramManager::GAUSSIANROW);
  m_programColumn = pm->getProgram(GLProgramManager::GAUSSIANCOLUMN);
  GLint program = m_programRow;
  m_uTextureRow = glGetUniformLocation(program, "u_texture");
  m_uScreenGeometryRow = glGetUniformLocation(program, "u_screenGeometry");
  m_uKernelRow = glGetUniformLocation(program, "u_kernel");
  GLIMPROC_LOGI(
    "m_uTextureRow: %d, m_uScreenGeometryRow: %d, m_uKernelRow: %d.\n",
    m_uTextureRow, m_uScreenGeometryRow, m_uKernelRow);

  program = m_programColumn;

  m_uTextureColumn = glGetUniformLocation(program, "u_texture");
  m_uScreenGeometryColumn = glGetUniformLocation(program, "u_screenGeometry");
  m_uKernelColumn = glGetUniformLocation(program, "u_kernel");
  GLIMPROC_LOGI(
    "m_uTextureColumn: %d, m_uScreenGeometryColumn: %d, m_uKernelColumn: "
    "%d.\n",
    m_uTextureColumn, m_uScreenGeometryColumn, m_uKernelColumn);

  m_programThreshold = pm->getProgram(GLProgramManager::ADAPTIVETHRESHOLD);
  program = m_programThreshold;

  m_uTextureOrigThreshold = glGetUniformLocation(program, "u_textureOrig");
  m_uTextureBlurThreshold = glGetUniformLocation(program, "u_textureBlur");
  m_uScreenGeometryThresholdg =
    glGetUniformLocation(program, "u_screenGeometry");
  m_uMaxValueThreshold = glGetUniformLocation(program, "u_maxValue");
  GLIMPROC_LOGI("m_uTextureOrigThreshold: %d, m_uTextureBlurThreshold: %d, "
                "m_uScreenGeometryThreshold: %d, m_uMaxValueThreshold: %d.\n",
                m_uTextureOrigThreshold, m_uTextureBlurThreshold,
                m_uScreenGeometryThresholdg, m_uMaxValueThreshold);
  return checkError("initProgram");
}

ProcessorOutput
AdaptiveThresholdProcessor::process(const ProcessorInput& pin)
{
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
    GLIMPROC_LOGE("fbo is not completed %d, %x.\n", __LINE__,
                  wf->checkFramebuffer());
    exit(1);
  }
  GLint imageGeometry[2] = { pin.width, pin.height };
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
    GLIMPROC_LOGE("fbo is not completed %d.\n", __LINE__);
    exit(1);
  }

  glUseProgram(m_programColumn);
  // setup uniforms
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, tmpTexture[0]->id());
  glUniform1i(m_uTextureColumn, 0);

  glUniform2iv(m_uScreenGeometryColumn, 1, imageGeometry);
  // setup kernel and block size

  glUniform4fv(m_uKernelColumn, s_block_size / 4, m_kernel.data());
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
  // render to screen

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
    GLIMPROC_LOGE("fbo is not completed %d.\n", __LINE__);
    exit(1);
  }
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
  checkError("image process");
  return ProcessorOutput{ tmpTexture[0] };
}
