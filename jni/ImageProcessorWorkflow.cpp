#include "ImageProcessorWorkflow.h"
#include "GLResources.h"
#include "IImageProcessor.h"

static const int s_preallocateTextureCount = 3;

namespace {
class GLRebornTexture : public GLTexture
{
public:
  GLRebornTexture(GLuint id, ImageProcessorWorkflow* wf);
  ~GLRebornTexture();

private:
  ImageProcessorWorkflow* m_wf;
};
}

ImageProcessorWorkflow::ImageProcessorWorkflow()
  : m_fbo(0)
  , m_width(0)
  , m_height(0)
  , m_vbo(0)
  , m_staled(false)
{
  CHECK_CONTEXT_NOT_NULL();
  glGenFramebuffers(1, &m_fbo);
  glGenBuffers(1, &m_vbo);
  static float positions[][4] = {
    { -1.0, 1.0, 0.0, 1.0 },
    { -1.0, -1.0, 0.0, 1.0 },
    { 1.0, 1.0, 0.0, 1.0 },
    { 1.0, -1.0, 0.0, 1.0 },
  };
  glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(positions), positions, GL_STATIC_DRAW);
}

ImageProcessorWorkflow::~ImageProcessorWorkflow()
{
  m_staled = true;
  CHECK_CONTEXT_NOT_NULL();
  glDeleteFramebuffers(1, &m_fbo);
  glDeleteBuffers(1, &m_vbo);
}

void
ImageProcessorWorkflow::registerIImageProcessor(IImageProcessor* processor)
{
  m_processors.push_back(processor);
}

ImageOutput
ImageProcessorWorkflow::process(const ImageDesc& desc)
{
  m_width = desc.width;
  m_height = desc.height;
  GLuint texture;

  CHECK_CONTEXT_NOT_NULL();
  glGenTextures(1, &texture);
  allocateTexture(texture, m_width, m_height, desc.format, desc.data);
  std::shared_ptr<GLTexture> scope(new GLTexture(texture));

  // save old viewport
  glViewport(0, 0, m_width, m_height);

  ProcessorInput pin = { m_width, m_height, scope, this };
  scope.reset();
  preallocateTextures();
  glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
  glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(0);
  for (auto& p : m_processors) {
    ProcessorOutput pout = p->process(pin);
    pin.color = pout.color;
  }
  glDisableVertexAttribArray(0);
  FBOScope fboscope(this);
  setColorAttachmentForFramebuffer(pin.color->id());

  glPixelStorei(GL_PACK_ALIGNMENT, 4);
  std::unique_ptr<uint8_t[]> readback(new uint8_t[m_width * m_height * 4]);
  glReadPixels(0, 0, m_width, m_height, GL_RGBA, GL_UNSIGNED_BYTE,
               readback.get());
  // mimic stale scope
  m_staled = true;
  m_fbotextures.clear();
  m_width = 0;
  m_height = 0;
  m_staled = false;
  // clean up state.
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  return ImageOutput{ std::move(readback) };
}

void
ImageProcessorWorkflow::enterFramebuffer()
{
  glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
}
void
ImageProcessorWorkflow::leaveFramebuffer()
{
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

GLint
ImageProcessorWorkflow::checkFramebuffer()
{
  return glCheckFramebufferStatus(GL_FRAMEBUFFER);
}

std::shared_ptr<GLTexture>
ImageProcessorWorkflow::requestTextureForFramebuffer()
{
  if (m_fbotextures.empty()) {
    GLuint texture;
    CHECK_CONTEXT_NOT_NULL();
    glGenTextures(1, &texture);
    allocateTexture(texture, m_width, m_height, GL_RGBA);
    std::shared_ptr<GLTexture> resource(new GLRebornTexture(texture, this));
    return resource;
  }
  std::shared_ptr<GLTexture> resource(m_fbotextures.back());
  m_fbotextures.pop_back();
  return resource;
}

void
ImageProcessorWorkflow::setColorAttachmentForFramebuffer(GLuint texture)
{
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                         texture, 0);
}

bool
ImageProcessorWorkflow::rebornTexture(GLuint texture)
{
  if (m_staled || m_fbotextures.size() > s_preallocateTextureCount) {
    return false;
  }

  m_fbotextures.push_back(
    std::move(std::unique_ptr<GLTexture>(new GLRebornTexture(texture, this))));
  return true;
}

void
ImageProcessorWorkflow::allocateTexture(GLuint texture, GLint width,
                                        GLint height, GLenum format, void* data)
{
  glBindTexture(GL_TEXTURE_2D, texture);
  glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format,
               GL_UNSIGNED_BYTE, data);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
}

void
ImageProcessorWorkflow::preallocateTextures()
{
  GLuint textures[s_preallocateTextureCount];

  CHECK_CONTEXT_NOT_NULL();
  glGenTextures(s_preallocateTextureCount, textures);
  m_fbotextures.reserve(s_preallocateTextureCount);
  for (int i = 0; i < s_preallocateTextureCount; ++i) {
    m_fbotextures.push_back(std::move(
      std::unique_ptr<GLTexture>(new GLRebornTexture(textures[i], this))));
    allocateTexture(textures[i], m_width, m_height, GL_RGBA);
  }
}

FBOScope::FBOScope(ImageProcessorWorkflow* wf)
  : m_wf(wf)
{
  wf->enterFramebuffer();
}

FBOScope::~FBOScope()
{
  m_wf->leaveFramebuffer();
}

GLRebornTexture::GLRebornTexture(GLuint id, ImageProcessorWorkflow* wf)
  : GLTexture(id)
  , m_wf(wf)
{
}

GLRebornTexture::~GLRebornTexture()
{
  if (m_wf->rebornTexture(id())) {
    reset();
  }
}
