#include "ResultPoint.h"
#include <cmath>
ResultPoint::ResultPoint(float x, float y)
{
  this->x = x;
  this->y = y;
}
void
ResultPoint::orderBestPatterns(ResultPoint* patterns)
{

  // Find distances between pattern centers
  float zeroOneDistance = distance(patterns[0], patterns[1]);
  float oneTwoDistance = distance(patterns[1], patterns[2]);
  float zeroTwoDistance = distance(patterns[0], patterns[2]);

  ResultPoint pointA;
  ResultPoint pointB;
  ResultPoint pointC;
  // Assume one closest to other two is B; A and C will just be guesses at first
  if (oneTwoDistance >= zeroOneDistance && oneTwoDistance >= zeroTwoDistance) {
    pointB = patterns[0];
    pointA = patterns[1];
    pointC = patterns[2];
  } else if (zeroTwoDistance >= oneTwoDistance &&
             zeroTwoDistance >= zeroOneDistance) {
    pointB = patterns[1];
    pointA = patterns[0];
    pointC = patterns[2];
  } else {
    pointB = patterns[2];
    pointA = patterns[0];
    pointC = patterns[1];
  }

  // Use cross product to figure out whether A and C are correct or flipped.
  // This asks whether BC x BA has a positive z component, which is the
  // arrangement
  // we want for A, B, C. If it's negative, then we've got it flipped around and
  // should swap A and C.
  if (crossProductZ(pointA, pointB, pointC) < 0.0f) {
    ResultPoint temp = pointA;
    pointA = pointC;
    pointC = temp;
  }

  patterns[0] = pointA;
  patterns[1] = pointB;
  patterns[2] = pointC;
}

static float
mdistance(float aX, float aY, float bX, float bY)
{
  float xDiff = aX - bX;
  float yDiff = aY - bY;
  return sqrtf(xDiff * xDiff + yDiff * yDiff);
}

float
ResultPoint::distance(const ResultPoint& pattern1, const ResultPoint& pattern2)
{
  return mdistance(pattern1.x, pattern1.y, pattern2.x, pattern2.y);
}

float
ResultPoint::crossProductZ(const ResultPoint& pointA, const ResultPoint& pointB,
                           const ResultPoint& pointC)
{
  float bX = pointB.x;
  float bY = pointB.y;
  return ((pointC.x - bX) * (pointA.y - bY)) -
         ((pointC.y - bY) * (pointA.x - bX));
}
