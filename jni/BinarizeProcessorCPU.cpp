#include "BinarizeProcessorCPU.h"
#include "GLCommon.h"
#include <stdlib.h>
#include <string.h>
#define LUMINANCE_BITS 5
#define LUMINANCE_SHIFT (8 - LUMINANCE_BITS)
#define LUMINANCE_BUCKETS (1 << LUMINANCE_BITS)

static int
estimateBlackPoint(int buckets[LUMINANCE_BUCKETS], int y)
{
  // Find the tallest peak in the histogram.
  int numBuckets = LUMINANCE_BUCKETS;
  int maxBucketCount = 0;
  int firstPeak = 0;
  int firstPeakSize = 0;
  for (int x = 0; x < numBuckets; x++) {
    if (buckets[x] > firstPeakSize) {
      firstPeak = x;
      firstPeakSize = buckets[x];
    }
    if (buckets[x] > maxBucketCount) {
      maxBucketCount = buckets[x];
    }
  }

  // Find the second-tallest peak which is somewhat far from the tallest peak.
  int secondPeak = 0;
  int secondPeakScore = 0;
  for (int x = 0; x < numBuckets; x++) {
    int distanceToBiggest = x - firstPeak;
    // Encourage more distant second peaks by multiplying by square of distance.
    int score = buckets[x] * distanceToBiggest * distanceToBiggest;
    if (score > secondPeakScore) {
      secondPeak = x;
      secondPeakScore = score;
    }
  }

  // Make sure firstPeak corresponds to the black peak.
  if (firstPeak > secondPeak) {
    int temp = firstPeak;
    firstPeak = secondPeak;
    secondPeak = temp;
  }

  // If there is too little contrast in the image to pick a meaningful black
  // point, throw rather
  // than waste time trying to decode the image, and risk false positives.
  // if (secondPeak - firstPeak <= numBuckets / 16) {
  //   GLIMPROC_LOGE("too little contrast.\n");
  //   exit(1);
  // }

  // Find a valley between them that is low and closer to the white peak.
  int bestValley = secondPeak - 1;
  int bestValleyScore = -1;
  for (int x = secondPeak - 1; x > firstPeak; x--) {
    int fromFirst = x - firstPeak;
    int score =
      fromFirst * fromFirst * (secondPeak - x) * (maxBucketCount - buckets[x]);
    if (score > bestValleyScore) {
      bestValley = x;
      bestValleyScore = score;
    }
  }

  return bestValley << LUMINANCE_SHIFT;
}

std::unique_ptr<uint8_t[]>
binarizeProcessCPU(int width, int height, const uint8_t* data)
{
  std::unique_ptr<uint8_t[]> output(new uint8_t[width * height]);
  memset(output.get(), 0, width * height);
#pragma omp parallel
  {
#pragma omp for schedule(runtime) nowait
    for (int y = 0; y < height; ++y) {
      const uint8_t* line = data + y * width;
      int bucket[LUMINANCE_BUCKETS];
      memset(bucket, 0, sizeof(bucket));
      // GLIMPROC_LOGE("handling line : %d.\n", y);

      for (int x = 0; x < width; ++x) {
        int pixel = line[x] & 0xff;
        bucket[pixel >> LUMINANCE_SHIFT]++;
      }
      int blackPoint = estimateBlackPoint(bucket, y);
      int left = line[0] & 0xff;
      int center = line[1] & 0xff;
      for (int x = 1; x < width - 1; x++) {
        int right = line[x + 1] & 0xff;
        // A simple -1 4 -1 box filter with a weight of 2.
        int luminance = ((center * 4) - left - right) / 2;
        if (luminance >= blackPoint) {
          output.get()[y * width + x] = 255;
        } else {
          output.get()[y * width + x] = 0;
        }
        left = center;
        center = right;
      }
    }
  }
#pragma omp flush
  return output;
}
