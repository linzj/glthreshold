#ifndef IMAGEPROCESSORWORKFLOW_H
#define IMAGEPROCESSORWORKFLOW_H
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

struct ImageOutput
{
  std::unique_ptr<uint8_t[]> outputBytes;
};

class GLTexture;
class IImageProcessor;

class ImageProcessorWorkflow final
{
public:
  ImageProcessorWorkflow();
  ~ImageProcessorWorkflow();
  void registerIImageProcessor(IImageProcessor* processor);
  ImageOutput process(const ImageDesc& desc);
  void enterFramebuffer();
  void leaveFramebuffer();
  GLint checkFramebuffer();
  std::shared_ptr<GLTexture> requestTextureForFramebuffer();
  void setColorAttachmentForFramebuffer(GLuint texture);
  bool rebornTexture(GLuint texture);

private:
  void preallocateTextures();
  void allocateTexture(GLuint texture, GLint width, GLint height, GLenum format,
                       void* data = nullptr);
  std::vector<IImageProcessor*> m_processors;
  std::vector<std::shared_ptr<GLTexture>> m_fbotextures;
  GLuint m_fbo;
  GLint m_width, m_height;
  GLuint m_vbo;
  bool m_staled;
};

class FBOScope
{
public:
  explicit FBOScope(ImageProcessorWorkflow* wf);
  ~FBOScope();

private:
  ImageProcessorWorkflow* m_wf;
};
#endif /* IMAGEPROCESSORWORKFLOW_H */
