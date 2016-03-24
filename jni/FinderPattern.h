#ifndef FINDERPATTERN_H
#define FINDERPATTERN_H
#include "ResultPoint.h"
#include <memory>

class FinderPattern : public ResultPoint
{

public:
  FinderPattern(float posX, float posY, float estimatedModuleSize);
  inline float getEstimatedModuleSize() { return estimatedModuleSize; }

  inline int getCount() { return count; }

  /*
  void incrementCount() {
    this.count++;
  }
   */

  /**
   * <p>Determines if this finder pattern "about equals" a finder pattern at the
   * stated
   * position and size -- meaning, it is at nearly the same center with nearly
   * the same size.</p>
   */
  bool aboutEquals(float moduleSize, float i, float j);
  /**
   * Combines this object's current estimate of a finder pattern position and
   * module size
   * with a new estimate. It returns a new {@code FinderPattern} containing a
   * weighted average
   * based on count.
   */
  std::unique_ptr<FinderPattern> combineEstimate(float i, float j,
                                                 float newModuleSize);

private:
  FinderPattern(float posX, float posY, float estimatedModuleSize, int count);
  float estimatedModuleSize;
  int count;
};

#endif /* FINDERPATTERN_H */
