#include "FinderPattern.h"
#include <cmath>

FinderPattern::FinderPattern(float posX, float posY, float estimatedModuleSize)
  : FinderPattern(posX, posY, estimatedModuleSize, 1)
{
}

FinderPattern::FinderPattern(float posX, float posY, float estimatedModuleSize,
                             int count)
  : ResultPoint(posX, posY)
{
  this->estimatedModuleSize = estimatedModuleSize;
  this->count = count;
}

bool
FinderPattern::aboutEquals(float moduleSize, float i, float j)
{
  if (std::abs(i - getY()) <= moduleSize &&
      std::abs(j - getX()) <= moduleSize) {
    float moduleSizeDiff = std::abs(moduleSize - estimatedModuleSize);
    return moduleSizeDiff <= 1.0f || moduleSizeDiff <= estimatedModuleSize;
  }
  return false;
}

std::unique_ptr<FinderPattern>
FinderPattern::combineEstimate(float i, float j, float newModuleSize)
{
  int combinedCount = count + 1;
  float combinedX = (count * getX() + j) / combinedCount;
  float combinedY = (count * getY() + i) / combinedCount;
  float combinedModuleSize =
    (count * estimatedModuleSize + newModuleSize) / combinedCount;
  return std::unique_ptr<FinderPattern>(
    new FinderPattern(combinedX, combinedY, combinedModuleSize, combinedCount));
}
