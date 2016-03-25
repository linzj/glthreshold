#ifndef IMAGEPROCESSORWORKFLOW_H
#define IMAGEPROCESSORWORKFLOW_H
#include "GLCommon.h"
#include <memory>
#include <stdint.h>
#include <vector>

struct ImageDesc
{
  GLint width, height;
  GLenum format;
  void* data;
};

struct ImageOutput
{
  std::unique_ptr<uint8_t[]> outputBytes;
};

class GLBuffer;
class IImageProcessor;
class GL3Interfaces;

class ImageProcessorWorkflow final
{
public:
  ImageProcessorWorkflow();
  ~ImageProcessorWorkflow();
  void registerIImageProcessor(IImageProcessor* processor);
  ImageOutput process(const GL3Interfaces& interfaces, const ImageDesc& desc);
  std::shared_ptr<GLBuffer> requestBuffer();

private:
  std::shared_ptr<GLBuffer> allocateBuffer(const void* data);
  std::vector<IImageProcessor*> m_processors;
  GLint m_width, m_height;
  const GL3Interfaces* m_interfaces;
  bool m_staled;
};
#endif /* IMAGEPROCESSORWORKFLOW_H */
