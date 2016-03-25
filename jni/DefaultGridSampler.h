#ifndef DEFAULTGRIDSAMPLER_H
#define DEFAULTGRIDSAMPLER_H
#include "GridSampler.h"

class DefaultGridSampler : public GridSampler
{
public:
  std::unique_ptr<LuminanceImage> sampleGrid(
    const LuminanceImage& image, int dimensionX, int dimensionY, float p1ToX,
    float p1ToY, float p2ToX, float p2ToY, float p3ToX, float p3ToY,
    float p4ToX, float p4ToY, float p1FromX, float p1FromY, float p2FromX,
    float p2FromY, float p3FromX, float p3FromY, float p4FromX,
    float p4FromY) override;

  std::unique_ptr<LuminanceImage> sampleGrid(
    const LuminanceImage& image, int dimensionX, int dimensionY,
    const PerspectiveTransform& transform) override;
};
#endif /* DEFAULTGRIDSAMPLER_H */
