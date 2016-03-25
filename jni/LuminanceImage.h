#ifndef LUMINANCEIMAGE_H
#define LUMINANCEIMAGE_H
#include <memory>
#include <stdint.h>
class LuminanceImage
{
public:
  LuminanceImage() = default;
  LuminanceImage& operator=(const LuminanceImage&) = delete;
  LuminanceImage(const LuminanceImage&) = delete;
  LuminanceImage(LuminanceImage&&);
  LuminanceImage& operator=(LuminanceImage&&);
  LuminanceImage(int width, int height);
  void reset(int width, int height, const void* data);
  void set(int x, int y);
  bool get(int x, int y) const;
  inline int getWidth() const { return m_width; }
  inline int getHeight() const { return m_height; }
  int m_width;
  int m_height;
  std::unique_ptr<uint8_t[]> m_data;
};
#endif /* LUMINANCEIMAGE_H */
