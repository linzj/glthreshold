#ifndef RESULTPOINT_H
#define RESULTPOINT_H

class ResultPoint
{

public:
  ResultPoint(float x, float y);
  inline float getX() { return x; }

  inline float getY() { return y; }

  static void orderBestPatterns(ResultPoint* patterns);

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
