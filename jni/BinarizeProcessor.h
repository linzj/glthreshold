#ifndef BINARIZEPROCESSOR_H
#define BINARIZEPROCESSOR_H
#include "IImageProcessor.h"

class ImageProcessorWorkflow;
class GL3Interfaces;
class GLProgramManager;

class BinarizeProcessor : public IImageProcessor
{
public:
  explicit BinarizeProcessor();
  ~BinarizeProcessor() = default;
  bool init(GLProgramManager* pm);

private:
  ProcessorOutput process(const GL3Interfaces& interfaces,
                          const ProcessorInput& desc) override;
  GLuint m_binarizerSum;
  GLuint m_binarizerFirstPeak;
  GLuint m_binarizerSecondScore;
  GLuint m_binarizerSecondPeak;
  GLuint m_bestValleyScore;
  GLuint m_bestValley;
  GLuint m_binarizerAssign;
};
#endif /* BINARIZEPROCESSOR_H */
