#ifndef DILATENONZEROPROCESSOR_H
#define DILATENONZEROPROCESSOR_H
#include "IImageProcessor.h"
class GLProgramManager;

class DilateNonZeroProcessor final : public IImageProcessor
{
public:
  DilateNonZeroProcessor();
  ~DilateNonZeroProcessor();
  bool init(GLProgramManager* pm, unsigned kwidth, unsigned kheight,
            unsigned iterations);
  ProcessorOutput process(const ProcessorInput& desc) override;

private:
  bool initProgram(GLProgramManager* pm);
  GLint m_uTextureRow;
  GLint m_uKWidthRow;
  GLint m_programRow;

  GLint m_uTextureColumn;
  GLint m_uKHeightColumn;
  GLint m_programColumn;
  unsigned m_kwidth;
  unsigned m_kheight;
};
#endif /* DILATENONZEROPROCESSOR_H */
