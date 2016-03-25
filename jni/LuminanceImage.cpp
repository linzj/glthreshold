#include "LuminanceImage.h"
#include <string.h>

void
LuminanceImage::reset(int width, int height, const void* data)
{
  m_width = width;
  m_height = height;
  m_data.reset(new uint8_t[width * height]);
  if (data)
    memcpy(m_data.get(), data, width * height);
  else
    memset(m_data.get(), 0xff, width * height);
}

LuminanceImage::LuminanceImage(LuminanceImage&& rhs)
{
  m_width = rhs.m_width;
  m_height = rhs.m_height;
  m_data = std::move(rhs.m_data);
}

LuminanceImage&
LuminanceImage::operator=(LuminanceImage&& rhs)
{
  m_width = rhs.m_width;
  m_height = rhs.m_height;
  m_data = std::move(rhs.m_data);
  return *this;
}

LuminanceImage::LuminanceImage(int width, int height)
{
  reset(width, height, nullptr);
}

void
LuminanceImage::set(int x, int y)
{
  m_data[y * m_width + x] = 0;
}
bool
LuminanceImage::get(int x, int y) const
{
  return !m_data[y * m_width + x];
}
