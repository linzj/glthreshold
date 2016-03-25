#ifndef PERSPECTIVETRANSFORM_H
#define PERSPECTIVETRANSFORM_H
#include <memory>
#include <vector>

/**
 * <p>This class implements a perspective transform in two dimensions. Given
 * four source and four
 * destination points, it will compute the transformation implied between them.
 * The code is based
 * directly upon section 3.4.2 of George Wolberg's "Digital Image Warping"; see
 * pages 54-56.</p>
 *
 * @author Sean Owen
 */
class PerspectiveTransform
{

private:
  float a11;
  float a12;
  float a13;
  float a21;
  float a22;
  float a23;
  float a31;
  float a32;
  float a33;

  PerspectiveTransform(float a11, float a21, float a31, float a12, float a22,
                       float a32, float a13, float a23, float a33);

public:
  static std::unique_ptr<PerspectiveTransform> quadrilateralToQuadrilateral(
    float x0, float y0, float x1, float y1, float x2, float y2, float x3,
    float y3, float x0p, float y0p, float x1p, float y1p, float x2p, float y2p,
    float x3p, float y3p);

  void transformPoints(std::vector<float>& points) const;

  void transformPoints(std::vector<float>& xValues,
                       std::vector<float>& yValues) const;

  static std::unique_ptr<PerspectiveTransform> squareToQuadrilateral(
    float x0, float y0, float x1, float y1, float x2, float y2, float x3,
    float y3);

  static std::unique_ptr<PerspectiveTransform> quadrilateralToSquare(
    float x0, float y0, float x1, float y1, float x2, float y2, float x3,
    float y3);

  std::unique_ptr<PerspectiveTransform> buildAdjoint();

  std::unique_ptr<PerspectiveTransform> times(
    const PerspectiveTransform& other);
};

#endif /* PERSPECTIVETRANSFORM_H */
