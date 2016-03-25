#ifndef ALIGNMENTPATTERN_H
#define ALIGNMENTPATTERN_H
#include "ResultPoint.h"
#include <memory>

class AlignmentPattern final : public ResultPoint
{

private:
  float estimatedModuleSize;

public:
  AlignmentPattern(float posX, float posY, float estimatedModuleSize);

  /**
   * <p>Determines if this alignment pattern "about equals" an alignment pattern
   * at the stated
   * position and size -- meaning, it is at nearly the same center with nearly
   * the same size.</p>
   */
  bool aboutEquals(float moduleSize, float i, float j);
  /**
   * Combines this object's current estimate of a finder pattern position and
   * module size
   * with a new estimate. It returns a new {@code FinderPattern} containing an
   * average of the two.
   */
  std::unique_ptr<AlignmentPattern> combineEstimate(float i, float j,
                                                    float newModuleSize);
};
#endif /* ALIGNMENTPATTERN_H */
