#include "ImageProcessorWorkflow.h"
#include "GL3Interfaces.h"
#include "GLResources.h"
#include "IImageProcessor.h"
#include <string.h>

ImageProcessorWorkflow::ImageProcessorWorkflow()
  : m_width(0)
  , m_height(0)
  , m_interfaces(nullptr)
  , m_staled(false)
{
  CHECK_CONTEXT_NOT_NULL();
}

ImageProcessorWorkflow::~ImageProcessorWorkflow()
{
  m_staled = true;
  CHECK_CONTEXT_NOT_NULL();
}

void
ImageProcessorWorkflow::registerIImageProcessor(IImageProcessor* processor)
{
  m_processors.push_back(processor);
}

ImageOutput
ImageProcessorWorkflow::process(const GL3Interfaces& interfaces,
                                const ImageDesc& desc)
{
  m_width = desc.width;
  m_height = desc.height;
  GLuint texture;

  CHECK_CONTEXT_NOT_NULL();
  m_interfaces = &interfaces;
  std::shared_ptr<GLBuffer> scope(allocateBuffer(desc.data));

  // save old viewport

  ProcessorInput pin = { m_width, m_height, scope, this };
  scope.reset();
  for (auto& p : m_processors) {
    ProcessorOutput pout = p->process(interfaces, pin);
    pin.color = pout.color;
  }

  std::unique_ptr<uint8_t[]> readback(new uint8_t[m_width * m_height]);
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, pin.color->id());
  const GLint* iptr = static_cast<const GLint*>(m_interfaces->glMapBufferRange(
    GL_SHADER_STORAGE_BUFFER, 0, pin.color->size(), GL_MAP_READ_BIT));
  for (int i = 0; i < m_width * m_height; ++i) {
    readback.get()[i] = iptr[i] & 0xffU;
  }
  m_interfaces->glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
  // mimic stale scope
  m_staled = true;
  m_width = 0;
  m_height = 0;
  m_staled = false;
  m_interfaces = nullptr;
  return ImageOutput{ std::move(readback) };
}

std::shared_ptr<GLBuffer>
ImageProcessorWorkflow::requestBuffer()
{
  GLuint buffer;
  glGenBuffers(1, &buffer);
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, buffer);
  GLsizei size = m_width * m_height * sizeof(GLint);
  std::shared_ptr<GLBuffer> resource(new GLBuffer(buffer, size));
  glBufferData(GL_SHADER_STORAGE_BUFFER, size, nullptr, GL_DYNAMIC_COPY);
  return resource;
}

std::shared_ptr<GLBuffer>
ImageProcessorWorkflow::allocateBuffer(const void* data)
{
  std::shared_ptr<GLBuffer> buffer(requestBuffer());
  const GLubyte* uptr = static_cast<const GLubyte*>(data);
  GLint* iptr = static_cast<GLint*>(m_interfaces->glMapBufferRange(
    GL_SHADER_STORAGE_BUFFER, 0, buffer->size(),
    GL_MAP_INVALIDATE_BUFFER_BIT | GL_MAP_WRITE_BIT));
  for (int i = 0; i < m_width * m_height; ++i) {
    iptr[i] = uptr[i] & 0xffU;
  }
  m_interfaces->glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
  checkError("allocateBuffer");
  return buffer;
}
