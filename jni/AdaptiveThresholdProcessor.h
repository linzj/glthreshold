#ifndef ADAPTIVETHRESHOLDPROCESSOR_H
#define ADAPTIVETHRESHOLDPROCESSOR_H
#include "IImageProcessor.h"
#include <vector>

class AdaptiveThresholdProcessor final : public IImageProcessor
{
public:
  AdaptiveThresholdProcessor();
  ~AdaptiveThresholdProcessor();
  bool init(int maxValue);
  ProcessorOutput process(const ProcessorInput& desc) override;

private:
  std::vector<GLfloat> m_kernel;

  GLint m_vPositionIndexRow;
  GLint m_uTextureRow;
  GLint m_uScreenGeometryRow;
  GLint m_uKernelRow;
  GLint m_programRow;

  GLint m_vPositionIndexColumn;
  GLint m_uTextureColumn;
  GLint m_uScreenGeometryColumn;
  GLint m_uKernelColumn;
  GLint m_programColumn;

  GLint m_maxValue;

  GLint m_vPositionIndexThreshold;
  GLint m_uTextureOrigThreshold;
  GLint m_uTextureBlurThreshold;
  GLint m_uScreenGeometryThresholdg;
  GLint m_uMaxValueThreshold;
  GLint m_programThreshold;

  static const GLint s_block_size = 92;
  void initGaussianBlurKernel();
  bool initProgram();
  static std::vector<GLfloat> getGaussianKernel(int n);
};
#endif /* ADAPTIVETHRESHOLDPROCESSOR_H */
