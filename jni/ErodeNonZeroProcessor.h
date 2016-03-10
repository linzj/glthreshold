#ifndef ERODENONZEROPROCESSOR_H
#define ERODENONZEROPROCESSOR_H
#include "IImageProcessor.h"
class GLProgramManager;

class ErodeNonZeroProcessor final : public IImageProcessor
{
public:
  ErodeNonZeroProcessor();
  ~ErodeNonZeroProcessor();
  bool init(GLProgramManager* pm, unsigned kwidth, unsigned kheight,
            unsigned iterations);
  ProcessorOutput process(const ProcessorInput& desc) override;

private:
  bool initProgram(GLProgramManager* pm);
  GLint m_vPositionIndexRow;
  GLint m_uTextureRow;
  GLint m_uScreenGeometryRow;
  GLint m_uKWidthRow;
  GLint m_programRow;

  GLint m_vPositionIndexColumn;
  GLint m_uTextureColumn;
  GLint m_uScreenGeometryColumn;
  GLint m_uKHeightColumn;
  GLint m_programColumn;
  unsigned m_kwidth;
  unsigned m_kheight;
};
#endif /* ERODENONZEROPROCESSOR_H */
