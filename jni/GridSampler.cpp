#include "GridSampler.h"

void
GridSampler::checkAndNudgePoints(const LuminanceImage& image,
                                 std::vector<float>& points)
{
  int width = image.getWidth();
  int height = image.getHeight();
  // Check and nudge points from start until we see some that are OK:
  bool nudged = true;
  for (int offset = 0; offset < points.size() && nudged; offset += 2) {
    int x = (int)points[offset];
    int y = (int)points[offset + 1];
    if (x < -1 || x > width || y < -1 || y > height) {
      throw std::exception();
    }
    nudged = false;
    if (x == -1) {
      points[offset] = 0.0f;
      nudged = true;
    } else if (x == width) {
      points[offset] = width - 1;
      nudged = true;
    }
    if (y == -1) {
      points[offset + 1] = 0.0f;
      nudged = true;
    } else if (y == height) {
      points[offset + 1] = height - 1;
      nudged = true;
    }
  }
  // Check and nudge points from end:
  nudged = true;
  for (int offset = points.size() - 2; offset >= 0 && nudged; offset -= 2) {
    int x = (int)points[offset];
    int y = (int)points[offset + 1];
    if (x < -1 || x > width || y < -1 || y > height) {
      throw std::exception();
    }
    nudged = false;
    if (x == -1) {
      points[offset] = 0.0f;
      nudged = true;
    } else if (x == width) {
      points[offset] = width - 1;
      nudged = true;
    }
    if (y == -1) {
      points[offset + 1] = 0.0f;
      nudged = true;
    } else if (y == height) {
      points[offset + 1] = height - 1;
      nudged = true;
    }
  }
}
