#include "LuminanceImage.h"
#include <string.h>

void
LuminanceImage::reset(int width, int height, const void* data)
{
  m_width = width;
  m_height = height;
  m_data.reset(new uint8_t[width * height]);
  if (data) {
#pragma omp parallel for
    for (int y = 0; y < height; ++y) {
      const uint8_t* rline =
        static_cast<const uint8_t*>(data) + (height - 1 - y) * width;
      uint8_t* wline = static_cast<uint8_t*>(m_data.get()) + y * width;
      memcpy(wline, rline, width);
    }
  } else
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

LuminanceImage::LuminanceImage(int dimension)
  : LuminanceImage(dimension, dimension)
{
}

void
LuminanceImage::set(int x, int y)
{
  if (x < 0 || x >= m_width || y < 0 || y >= m_height)
    throw 1;
  m_data[y * m_width + x] = 0;
}

bool
LuminanceImage::get(int x, int y) const
{
  if (x < 0 || x >= m_width || y < 0 || y >= m_height)
    throw 1;
  return !m_data[y * m_width + x];
}

void
LuminanceImage::setRegion(int left, int top, int width, int height)
{
  if (top < 0 || left < 0) {
    throw 1;
  }
  if (height < 1 || width < 1) {
    throw 1;
  }
  int right = left + width;
  int bottom = top + height;
  if (bottom > this->m_height || right > this->m_width) {
    throw 1;
  }
  int rowSize = m_width;
  for (int y = top; y < bottom; y++) {
    int offset = y * rowSize;
    for (int x = left; x < right; x++) {
      m_data.get()[offset + (x)] = 0;
    }
  }
}

void
LuminanceImage::flip(int x, int y)
{
  int offset = y * m_width + x;
  int8_t v = static_cast<int8_t>(m_data.get()[offset]);
  v ^= 1 << 7;
  v >>= 7;
  m_data.get()[offset] = static_cast<uint8_t>(static_cast<int>(v) & 0xffU);
}
