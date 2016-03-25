#ifndef IIMAGEPROCESSOR_H
#define IIMAGEPROCESSOR_H
#include "GLCommon.h"
#include <memory>
#include <stdint.h>

class GLBuffer;
class ImageProcessorWorkflow;
class GL3Interfaces;

struct ProcessorOutput
{
  std::shared_ptr<GLBuffer> color;
};

struct ProcessorInput
{
  GLint width, height;
  std::shared_ptr<GLBuffer> color;
  ImageProcessorWorkflow* wf;
};

class IImageProcessor
{
public:
  virtual ~IImageProcessor() = default;
  virtual ProcessorOutput process(const GL3Interfaces& interfaces,
                                  const ProcessorInput& desc) = 0;
};

#endif /* IIMAGEPROCESSOR_H */
