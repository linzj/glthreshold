#ifndef IMAGEPROCESSOR_H
#define IMAGEPROCESSOR_H
#include <GLES2/gl2.h>
#include <memory>
#include <stdint.h>
#include <vector>

struct ImageDesc
{
  GLint width, height;
  GLenum format;
  void* data;
};

class ImageProcessor
{
public:
  ImageProcessor();
  ~ImageProcessor();
  bool init(int maxValue);
  std::unique_ptr<uint8_t[]> process(const ImageDesc& desc);
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

private:
  static const GLint s_block_size = 91;
  void initGaussianBlurKernel();
  bool initProgram();
  void allocateTexture(GLuint texture, GLint width, GLint height, GLenum format,
                       void* data = nullptr);
  static std::vector<GLfloat> getGaussianKernel(int n);
};
#endif /* IMAGEPROCESSOR_H */
