#include "ThresholdProcessor.h"
#include "GLProgramManager.h"
#include "GLResources.h"
#include "ImageProcessorWorkflow.h"
#include <stdlib.h>

ThresholdProcessor::ThresholdProcessor()
  : m_uTexture(0)
  , m_uMaxValue(0)
  , m_uThreshold(0)
  , m_program(0)
  , m_maxValue(0)
  , m_threshold(0)
{
}

bool
ThresholdProcessor::init(GLProgramManager* pm, int maxValue, int threshold)
{
  m_maxValue = maxValue;
  m_threshold = threshold;
  return initProgram(pm);
}

ProcessorOutput
ThresholdProcessor::process(const ProcessorInput& pin)
{
  ImageProcessorWorkflow* wf = pin.wf;
  FBOScope fboscope(wf);
  std::shared_ptr<GLTexture> tmpTexture[1] = {
    wf->requestTextureForFramebuffer()
  };

  // bind fbo and complete it.
  wf->setColorAttachmentForFramebuffer(tmpTexture[0]->id());

  if (GL_FRAMEBUFFER_COMPLETE != wf->checkFramebuffer()) {
    GLIMPROC_LOGE("fbo is not completed %d, %x.\n", __LINE__,
                  wf->checkFramebuffer());
    exit(1);
  }
  glUseProgram(m_program);
  // setup uniforms
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, pin.color->id());
  glUniform1i(m_uTexture, 0);

  // setup kernel and block size

  glUniform1f(m_uMaxValue, static_cast<GLfloat>(m_maxValue) / 255.0f);
  glUniform1f(m_uThreshold, static_cast<GLfloat>(m_threshold) / 255.0f);
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
  return ProcessorOutput{ tmpTexture[0] };
}

bool
ThresholdProcessor::initProgram(GLProgramManager* pm)
{
  m_program = pm->getProgram(GLProgramManager::THRESHOLD);
  if (!m_program)
    return false;
  GLint program = m_program;
  m_uTexture = glGetUniformLocation(program, "u_texture");
  m_uMaxValue = glGetUniformLocation(program, "u_maxValue");
  m_uThreshold = glGetUniformLocation(program, "u_threshold");
  GLIMPROC_LOGI("m_uTexture: %d, m_uMaxValue: %d, "
                "m_uThreshold: %d.\n",
                m_uTexture, m_uMaxValue, m_uThreshold);
  return true;
}
