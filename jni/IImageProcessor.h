#ifndef IIMAGEPROCESSOR_H
#define IIMAGEPROCESSOR_H
#include <GLES2/gl2.h>
#include <memory>
#include <stdint.h>

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

class IImageProcessor
{
public:
  virtual ~IImageProcessor() = default;
  virtual ImageOutput process(const ImageDesc& desc) = 0;
};

#endif /* IIMAGEPROCESSOR_H */
