#include "DilateNonZeroProcessor.h"
#include "GLProgramManager.h"
#include "GLResources.h"
#include "ImageProcessorWorkflow.h"
#include <stdlib.h>

DilateNonZeroProcessor::DilateNonZeroProcessor()
  : m_uTextureRow(0)
  , m_uScreenGeometryRow(0)
  , m_uKWidthRow(0)
  , m_programRow(0)

  , m_uTextureColumn(0)
  , m_uScreenGeometryColumn(0)
  , m_uKHeightColumn(0)
  , m_programColumn(0)
  , m_kwidth(0)
  , m_kheight(0)
{
}

DilateNonZeroProcessor::~DilateNonZeroProcessor()
{
}

bool
DilateNonZeroProcessor::init(GLProgramManager* pm, unsigned kwidth,
                             unsigned kheight, unsigned iterations)
{
  m_kwidth = kwidth + (kwidth - 1) * (iterations - 1);
  m_kheight = kheight + (kheight - 1) * (iterations - 1);
  return initProgram(pm);
}

ProcessorOutput
DilateNonZeroProcessor::process(const ProcessorInput& pin)
{
  ImageProcessorWorkflow* wf = pin.wf;
  FBOScope fboscope(wf);
  // zero for row process, one for column process
  std::shared_ptr<GLTexture> tmpTexture[2] = {
    wf->requestTextureForFramebuffer(), wf->requestTextureForFramebuffer()
  };

  // bind fbo and complete it.
  wf->setColorAttachmentForFramebuffer(tmpTexture[0]->id());

  if (GL_FRAMEBUFFER_COMPLETE != wf->checkFramebuffer()) {
    fprintf(stderr, "fbo is not completed %d, %x.\n", __LINE__,
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

  glUniform1i(m_uKWidthRow, m_kwidth);
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

  // bind fbo and complete it.
  wf->setColorAttachmentForFramebuffer(tmpTexture[1]->id());
  if (GL_FRAMEBUFFER_COMPLETE != wf->checkFramebuffer()) {
    fprintf(stderr, "fbo is not completed %d.\n", __LINE__);
    exit(1);
  }

  glUseProgram(m_programColumn);
  // setup uniforms
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, tmpTexture[0]->id());
  glUniform1i(m_uTextureColumn, 0);

  glUniform2iv(m_uScreenGeometryColumn, 1, imageGeometry);
  // setup kernel and block size

  glUniform1i(m_uKHeightColumn, m_kheight);
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
  return ProcessorOutput{ tmpTexture[1] };
}

bool
DilateNonZeroProcessor::initProgram(GLProgramManager* pm)
{
  m_programRow = pm->getProgram(GLProgramManager::DILATENONZEROROW);
  m_programColumn = pm->getProgram(GLProgramManager::DILATENONZEROCOLUMN);
  if (!m_programRow || !m_programColumn) {
    return false;
  }
  GLuint program = m_programRow;
  m_uTextureRow = glGetUniformLocation(program, "u_texture");
  m_uScreenGeometryRow = glGetUniformLocation(program, "u_screenGeometry");
  m_uKWidthRow = glGetUniformLocation(program, "u_kRowSize");
  printf("m_uTextureRow: %d, m_uScreenGeometryRow: %d, m_uKWidthRow: %d.\n",
         m_uTextureRow, m_uScreenGeometryRow, m_uKWidthRow);

  program = m_programColumn;
  m_uTextureColumn = glGetUniformLocation(program, "u_texture");
  m_uScreenGeometryColumn = glGetUniformLocation(program, "u_screenGeometry");
  m_uKHeightColumn = glGetUniformLocation(program, "u_kColumnSize");
  printf("m_uTextureColumn: %d, m_uScreenGeometryColumn: %d, m_uKHeightColumn: "
         "%d.\n",
         m_uTextureColumn, m_uScreenGeometryColumn, m_uKHeightColumn);
  return true;
}
