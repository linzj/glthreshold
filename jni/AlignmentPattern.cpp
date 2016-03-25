#include "AlignmentPattern.h"

AlignmentPattern::AlignmentPattern(float posX, float posY,
                                   float estimatedModuleSize)
  : ResultPoint(posX, posY)
{
  this->estimatedModuleSize = estimatedModuleSize;
}

bool
AlignmentPattern::aboutEquals(float moduleSize, float i, float j)
{
  if (std::abs(i - getY()) <= moduleSize &&
      std::abs(j - getX()) <= moduleSize) {
    float moduleSizeDiff = std::abs(moduleSize - estimatedModuleSize);
    return moduleSizeDiff <= 1.0f || moduleSizeDiff <= estimatedModuleSize;
  }
  return false;
}

std::unique_ptr<AlignmentPattern>
AlignmentPattern::combineEstimate(float i, float j, float newModuleSize)
{
  float combinedX = (getX() + j) / 2.0f;
  float combinedY = (getY() + i) / 2.0f;
  float combinedModuleSize = (estimatedModuleSize + newModuleSize) / 2.0f;
  return std::unique_ptr<AlignmentPattern>(
    new AlignmentPattern(combinedX, combinedY, combinedModuleSize));
}
