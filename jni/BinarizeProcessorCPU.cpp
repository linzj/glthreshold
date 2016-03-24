#include "BinarizeProcessorCPU.h"
#include "GLCommon.h"
#include <stdlib.h>
#include <string.h>
#define LUMINANCE_BITS 5
#define LUMINANCE_SHIFT (8 - LUMINANCE_BITS)
#define LUMINANCE_BUCKETS (1 << LUMINANCE_BITS)
#if defined(__arm__)
#include <arm_neon.h>
#endif

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
#if defined(__arm__)
  int aligned_width = (width >> 4) << 4;
#else
  int aligned_width = 0;
#endif
#pragma omp parallel
  {
    for (int y = 0; y < height; ++y) {
      const uint8_t* line = data + y * width;
      int bucket[LUMINANCE_BUCKETS];
      memset(bucket, 0, sizeof(bucket));

#pragma omp for schedule(runtime)
#if defined(__arm__)
      for (int x = 0; x < aligned_width; x += 16) {
        uint8x16_t pixels = vld1q_u8(&line[x]);
        uint8x16_t result = vshrq_n_u8(pixels, LUMINANCE_SHIFT);
#pragma omp atomic
        bucket[result[0]]++;
#pragma omp atomic
        bucket[result[1]]++;
#pragma omp atomic
        bucket[result[2]]++;
#pragma omp atomic
        bucket[result[3]]++;
#pragma omp atomic
        bucket[result[4]]++;
#pragma omp atomic
        bucket[result[5]]++;
#pragma omp atomic
        bucket[result[6]]++;
#pragma omp atomic
        bucket[result[7]]++;
#pragma omp atomic
        bucket[result[8]]++;
#pragma omp atomic
        bucket[result[9]]++;
#pragma omp atomic
        bucket[result[10]]++;
#pragma omp atomic
        bucket[result[11]]++;
#pragma omp atomic
        bucket[result[12]]++;
#pragma omp atomic
        bucket[result[13]]++;
#pragma omp atomic
        bucket[result[14]]++;
#pragma omp atomic
        bucket[result[15]]++;
      }
#endif
      for (int x = aligned_width; x < width; ++x) {
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
  return output;
}
