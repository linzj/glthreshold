#ifndef IIMAGEPROCESSOR_H
#define IIMAGEPROCESSOR_H
#include <GLES2/gl2.h>
#include <memory>
#include <stdint.h>

class GLTexture;
class ImageProcessorWorkflow;

struct ProcessorOutput
{
  std::shared_ptr<GLTexture> color;
};

struct ProcessorInput
{
  GLint width, height;
  std::shared_ptr<GLTexture> color;
  ImageProcessorWorkflow* wf;
};

class IImageProcessor
{
public:
  virtual ~IImageProcessor() = default;
  virtual ProcessorOutput process(const ProcessorInput& desc) = 0;
  static inline const char** getVertexSourceLocation()
  {
    static const char* vertexShaderSource = "attribute vec4 v_position;\n"
                                            "void main()\n"
                                            "{\n"
                                            "   gl_Position = v_position;\n"
                                            "}\n";
    return &vertexShaderSource;
  }
};

#endif /* IIMAGEPROCESSOR_H */
