#include "BinarizeProcessor.h"
#include "GL3Interfaces.h"
#include "GLProgramManager.h"
#include "GLResources.h"
#include "ImageProcessorWorkflow.h"
#include <string.h>
#define LUMINANCE_BITS 5
#define LUMINANCE_SHIFT (8 - LUMINANCE_BITS)
#define LUMINANCE_BUCKETS (1 << LUMINANCE_BITS)

#define USE_GL_MEMORY_BARRIER
#ifdef USE_GL_MEMORY_BARRIER
#define GL_CMD_BARRIER interfaces.glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT)
#else
#define GL_CMD_BARRIER glFinish()
#endif // USE_GL_MEMORY_BARRIER

BinarizeProcessor::BinarizeProcessor()
  : m_binarizerSum(0)
  , m_binarizerFirstPeak(0)
  , m_binarizerSecondScore(0)
  , m_binarizerSecondPeak(0)
  , m_bestValleyScore(0)
  , m_bestValley(0)
  , m_binarizerAssign(0)
{
}

bool
BinarizeProcessor::init(GLProgramManager* pm)
{
  m_binarizerSum = pm->getProgram(GLProgramManager::BINARIZERSUM);
  m_binarizerFirstPeak = pm->getProgram(GLProgramManager::BINARIZERFIRSTPEAK);
  m_binarizerSecondScore =
    pm->getProgram(GLProgramManager::BINARIZERSECONDSCORE);
  m_binarizerSecondPeak = pm->getProgram(GLProgramManager::BINARIZERSECONDPEAK);
  m_bestValleyScore = pm->getProgram(GLProgramManager::BESTVALLEYSCORE);
  m_bestValley = pm->getProgram(GLProgramManager::BESTVALLEY);
  m_binarizerAssign = pm->getProgram(GLProgramManager::BINARIZERASSIGN);
  return (!!m_binarizerSum) & (!!m_binarizerFirstPeak) &
         (!!m_binarizerSecondScore) & (!!m_binarizerSecondPeak) &
         (!!m_bestValleyScore) & (!!m_bestValley) & (!!m_binarizerAssign);
}

ProcessorOutput
BinarizeProcessor::process(const GL3Interfaces& interfaces,
                           const ProcessorInput& desc)
{
  GLuint ssbo;
  GLintptr offset;
  glGenBuffers(1, &ssbo);
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
  GLint ssboAlignment;
  glGetIntegerv(GL_SHADER_STORAGE_BUFFER_OFFSET_ALIGNMENT, &ssboAlignment);
  GLsizeiptr rowSumSize =
    roundUp(sizeof(GLint) * LUMINANCE_BUCKETS * desc.height, ssboAlignment);
  GLsizeiptr firstPeakSize =
    roundUp(sizeof(GLint) * desc.height, ssboAlignment);
  GLsizeiptr secondPeakSize =
    roundUp(sizeof(GLint) * desc.height, ssboAlignment);
  GLsizeiptr secondPeakScoreSize =
    roundUp(sizeof(GLint) * LUMINANCE_BUCKETS * desc.height, ssboAlignment);
  GLsizeiptr bestValleyScoreSize =
    roundUp(sizeof(GLint) * LUMINANCE_BUCKETS * desc.height, ssboAlignment);
  GLsizeiptr bestValleySize =
    roundUp(sizeof(GLint) * desc.height, ssboAlignment);
  GLsizeiptr size = rowSumSize + firstPeakSize + secondPeakSize +
                    secondPeakScoreSize + bestValleyScoreSize + bestValleySize;
  glBufferData(GL_SHADER_STORAGE_BUFFER, size, nullptr, GL_STATIC_COPY);
  {
    // invalidate the buffer
    void* ptr = interfaces.glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, size,
                                            GL_MAP_INVALIDATE_BUFFER_BIT |
                                              GL_MAP_WRITE_BIT);
    memset(ptr, 0, size);
    interfaces.glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
  }
  std::shared_ptr<GLBuffer> output = desc.wf->requestBuffer();
  offset = 0;
  interfaces.glBindBufferRange(GL_SHADER_STORAGE_BUFFER, 0, ssbo, offset,
                               rowSumSize);
  offset += rowSumSize;
  interfaces.glBindBufferRange(GL_SHADER_STORAGE_BUFFER, 1, ssbo, offset,
                               firstPeakSize);
  offset += firstPeakSize;
  interfaces.glBindBufferRange(GL_SHADER_STORAGE_BUFFER, 2, ssbo, offset,
                               secondPeakSize);
  offset += secondPeakSize;
  interfaces.glBindBufferRange(GL_SHADER_STORAGE_BUFFER, 3, ssbo, offset,
                               secondPeakScoreSize);
  offset += secondPeakScoreSize;
  interfaces.glBindBufferRange(GL_SHADER_STORAGE_BUFFER, 4, ssbo, offset,
                               bestValleyScoreSize);
  offset += bestValleyScoreSize;
  interfaces.glBindBufferRange(GL_SHADER_STORAGE_BUFFER, 5, ssbo, offset,
                               bestValleySize);
  interfaces.glBindBufferRange(GL_SHADER_STORAGE_BUFFER, 6, desc.color->id(), 0,
                               desc.color->size());
  interfaces.glBindBufferRange(GL_SHADER_STORAGE_BUFFER, 7, output->id(), 0,
                               output->size());
  glUseProgram(m_binarizerSum);
  interfaces.glDispatchCompute(desc.width, desc.height, 1);
  GL_CMD_BARRIER;
  glUseProgram(m_binarizerFirstPeak);
  interfaces.glDispatchCompute(LUMINANCE_BUCKETS, desc.height, 1);
  GL_CMD_BARRIER;
  glUseProgram(m_binarizerSecondScore);
  interfaces.glDispatchCompute(LUMINANCE_BUCKETS, desc.height, 1);
  GL_CMD_BARRIER;
  glUseProgram(m_binarizerSecondPeak);
  interfaces.glDispatchCompute(LUMINANCE_BUCKETS, desc.height, 1);
  GL_CMD_BARRIER;
  glUseProgram(m_bestValleyScore);
  interfaces.glDispatchCompute(LUMINANCE_BUCKETS, desc.height, 1);
  GL_CMD_BARRIER;
  glUseProgram(m_bestValley);
  interfaces.glDispatchCompute(LUMINANCE_BUCKETS, desc.height, 1);
  GL_CMD_BARRIER;

  glUseProgram(m_binarizerAssign);
  interfaces.glDispatchCompute(desc.width - 2, desc.height, 1);
  interfaces.glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
  glDeleteBuffers(1, &ssbo);
  checkError("BinarizeProcessor");
  return ProcessorOutput{ output };
}
