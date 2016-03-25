#ifndef GRIDSAMPLER_H
#define GRIDSAMPLER_H
#include "LuminanceImage.h"
#include <memory>
#include <vector>

class PerspectiveTransform;
class GridSampler
{
public:
  /**
   * Samples an image for a rectangular matrix of bits of the given dimension.
   * The sampling
   * transformation is determined by the coordinates of 4 points, in the
   * original and transformed
   * image space.
   *
   * @param image image to sample
   * @param dimensionX width of {@link BitMatrix} to sample from image
   * @param dimensionY height of {@link BitMatrix} to sample from image
   * @param p1ToX point 1 preimage X
   * @param p1ToY point 1 preimage Y
   * @param p2ToX point 2 preimage X
   * @param p2ToY point 2 preimage Y
   * @param p3ToX point 3 preimage X
   * @param p3ToY point 3 preimage Y
   * @param p4ToX point 4 preimage X
   * @param p4ToY point 4 preimage Y
   * @param p1FromX point 1 image X
   * @param p1FromY point 1 image Y
   * @param p2FromX point 2 image X
   * @param p2FromY point 2 image Y
   * @param p3FromX point 3 image X
   * @param p3FromY point 3 image Y
   * @param p4FromX point 4 image X
   * @param p4FromY point 4 image Y
   * @return {@link BitMatrix} representing a grid of points sampled from the
   * image within a region
   *   defined by the "from" parameters
   * @throws NotFoundException if image can't be sampled, for example, if the
   * transformation defined
   *   by the given points is invalid or results in sampling outside the image
   * boundaries
   */
  virtual std::unique_ptr<LuminanceImage> sampleGrid(
    const LuminanceImage& image, int dimensionX, int dimensionY, float p1ToX,
    float p1ToY, float p2ToX, float p2ToY, float p3ToX, float p3ToY,
    float p4ToX, float p4ToY, float p1FromX, float p1FromY, float p2FromX,
    float p2FromY, float p3FromX, float p3FromY, float p4FromX,
    float p4FromY) = 0;

  virtual std::unique_ptr<LuminanceImage> sampleGrid(
    const LuminanceImage& image, int dimensionX, int dimensionY,
    const PerspectiveTransform& transform) = 0;

protected:
  /**
   * <p>Checks a set of points that have been transformed to sample points on an
   * image against
   * the image's dimensions to see if the point are even within the image.</p>
   *
   * <p>This method will actually "nudge" the endpoints back onto the image if
   * they are found to be
   * barely (less than 1 pixel) off the image. This accounts for imperfect
   * detection of finder
   * patterns in an image where the QR Code runs all the way to the image
   * border.</p>
   *
   * <p>For efficiency, the method will check points from either end of the line
   * until one is found
   * to be within the image. Because the set of points are assumed to be linear,
   * this is valid.</p>
   *
   * @param image image into which the points should map
   * @param points actual points in x1,y1,...,xn,yn form
   * @throws NotFoundException if an endpoint is lies outside the image
   * boundaries
   */
  static void checkAndNudgePoints(const LuminanceImage& image,
                                  std::vector<float>& points);
};
#endif /* GRIDSAMPLER_H */
