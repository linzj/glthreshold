#ifndef RESULTPOINT_H
#define RESULTPOINT_H
#include <cmath>

inline static float
mdistance(float aX, float aY, float bX, float bY)
{
  float xDiff = aX - bX;
  float yDiff = aY - bY;
  return sqrtf(xDiff * xDiff + yDiff * yDiff);
}

class ResultPoint
{

public:
  ResultPoint(float x, float y);
  inline float getX() const { return x; }

  inline float getY() const { return y; }

  static void orderBestPatterns(const ResultPoint** patterns);

  static float distance(const ResultPoint& pattern1,
                        const ResultPoint& pattern2);
  /**
   * Returns the z component of the cross product between vectors BC and BA.
   */
  static float crossProductZ(const ResultPoint& pointA,
                             const ResultPoint& pointB,
                             const ResultPoint& pointC);

private:
  ResultPoint() = default;
  float x;
  float y;
};
#endif /* RESULTPOINT_H */
