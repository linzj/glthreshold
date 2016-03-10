#ifndef THRESHOLDPROCESSOR_H
#define THRESHOLDPROCESSOR_H
#include "IImageProcessor.h"

class GLProgramManager;

class ThresholdProcessor final : public IImageProcessor
{
public:
  ThresholdProcessor();
  ~ThresholdProcessor() = default;
  bool init(GLProgramManager* pm, int maxValue, int threshold);
  ProcessorOutput process(const ProcessorInput& desc) override;

private:
  GLint m_uTexture;
  GLint m_uScreenGeometry;
  GLint m_uMaxValue;
  GLint m_uThreshold;
  GLint m_program;
  int m_maxValue;
  int m_threshold;
  bool initProgram(GLProgramManager* pm);
};

#endif /* THRESHOLDPROCESSOR_H */
