#include "RGBToLuminance.h"

std::unique_ptr<uint8_t[]>
RGBToLuminance(int width, int height, const void* data)
{
  int rowBytes = (width * 24 + 31) / 32 * 4;
  std::unique_ptr<uint8_t[]> result(new uint8_t[width * height]);
  uint8_t* rp = result.get();
#pragma omp parallel
  {
#pragma omp parallel for
    for (int y = 0; y < height; ++y) {
      const uint8_t* line = static_cast<const uint8_t*>(data) + y * rowBytes;
      uint8_t* lrp = rp + y * width;
      for (int x = 0; x < width; ++x, line += 3, lrp += 1) {
        *lrp = (306 * (line[0]) + 601 * (line[1]) + 117 * (line[2])) >> 10;
      }
    }
  }
  return std::move(result);
}

std::unique_ptr<uint8_t[]>
RGBAToLuminance(int width, int height, const void* data)
{
  int rowBytes = width * 4;
  std::unique_ptr<uint8_t[]> result(new uint8_t[width * height]);
  uint8_t* rp = result.get();
#pragma omp parallel
  {
#pragma omp parallel for
    for (int y = 0; y < height; ++y) {
      const uint8_t* line = static_cast<const uint8_t*>(data) + y * rowBytes;
      uint8_t* lrp = rp + y * width;
      for (int x = 0; x < width; ++x, line += 4, lrp += 1) {
        *lrp = (306 * (line[0]) + 601 * (line[1]) + 117 * (line[2])) >> 10;
      }
    }
  }
  return std::move(result);
}
