#ifndef DILATENONZEROPROCESSOR_H
#define DILATENONZEROPROCESSOR_H
#include "IImageProcessor.h"
class DilateNoneZeroProcessor final : public IImageProcessor
{
public:
  DilateNoneZeroProcessor();
  ~DilateNoneZeroProcessor();
  bool init(unsigned kwidth, unsigned kheight);
  ProcessorOutput process(const ProcessorInput& desc) override;

private:
  bool initProgram();
  GLint m_vPositionIndexRowFirst;
  GLint m_uTextureRowFirst;
  GLint m_uScreenGeometryRowFirst;
  GLint m_uKWidthRowFirst;
  GLint m_programRowFirst;

  GLint m_vPositionIndexRowSecond;
  GLint m_uTextureRowSecond;
  GLint m_uScreenGeometryRowSecond;
  GLint m_uKWidthRowSecond;
  GLint m_programRowSecond;

  GLint m_vPositionIndexColumn;
  GLint m_uTextureColumn;
  GLint m_uScreenGeometryColumn;
  GLint m_uKHeightColumn;
  GLint m_programColumn;
  unsigned m_kwidth;
  unsigned m_kheight;
};
#endif /* DILATENONZEROPROCESSOR_H */
