#include "BinarizeProcessorCPU.h"
#include "log.h"
#include <stdlib.h>
#include <string.h>
#if defined(__arm__)
#define USE_NEON_SIMD 1
#else
#define USE_NEON_SIMD 0
#endif
#if USE_NEON_SIMD
#include <arm_neon.h>
#endif
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
  if (secondPeak - firstPeak <= numBuckets / 16) {
    return -1;
  }

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
  volatile bool failed = false;

  {
    for (int y = 0; y < height; ++y) {
      if (failed)
        continue;
      const uint8_t* line = data + y * width;
      int bucket[LUMINANCE_BUCKETS];
      memset(bucket, 0, sizeof(bucket));
// GLIMPROC_LOGE("handling line : %d.\n", y);
#if USE_NEON_SIMD
      for (int x = 0; x < width; x += 32) {
        uint8x16x2_t v = vld2q_u8(line + x);
        uint8x8_t d1 = vget_low_u8(v.val[0]);
        d1 = vshr_n_u8(d1, LUMINANCE_SHIFT);
        uint8x8_t d2 = vget_high_u8(v.val[0]);
        d2 = vshr_n_u8(d2, LUMINANCE_SHIFT);

        uint8x8_t d3 = vget_low_u8(v.val[1]);
        d3 = vshr_n_u8(d3, LUMINANCE_SHIFT);
        uint8x8_t d4 = vget_high_u8(v.val[1]);
        d4 = vshr_n_u8(d4, LUMINANCE_SHIFT);
        bucket[d1[0]]++;
        bucket[d1[1]]++;
        bucket[d1[2]]++;
        bucket[d1[3]]++;
        bucket[d1[4]]++;
        bucket[d1[5]]++;
        bucket[d1[6]]++;
        bucket[d1[7]]++;

        bucket[d2[0]]++;
        bucket[d2[1]]++;
        bucket[d2[2]]++;
        bucket[d2[3]]++;
        bucket[d2[4]]++;
        bucket[d2[5]]++;
        bucket[d2[6]]++;
        bucket[d2[7]]++;

        bucket[d3[0]]++;
        bucket[d3[1]]++;
        bucket[d3[2]]++;
        bucket[d3[3]]++;
        bucket[d3[4]]++;
        bucket[d3[5]]++;
        bucket[d3[6]]++;
        bucket[d3[7]]++;

        bucket[d4[0]]++;
        bucket[d4[1]]++;
        bucket[d4[2]]++;
        bucket[d4[3]]++;
        bucket[d4[4]]++;
        bucket[d4[5]]++;
        bucket[d4[6]]++;
        bucket[d4[7]]++;
      }
#else
      for (int x = 0; x < width; ++x) {
        int pixel = line[x] & 0xff;
        bucket[pixel >> LUMINANCE_SHIFT]++;
      }
#endif

      int blackPoint = estimateBlackPoint(bucket, y);
      int x = 1;
#if USE_NEON_SIMD
      int16x8_t vblackPoint = vdupq_n_s16(blackPoint);
      for (; x < width - 1; x += 8) {
        uint8x8_t left = vld1_u8(line + x - 1);
        uint8x8_t center = vld1_u8(line + x);
        uint8x8_t right = vld1_u8(line + x + 1);
        int16x8_t centerl = vreinterpretq_s16_u16(vmovl_u8(center));
        centerl = vshlq_n_s16(centerl, 2);
        int16x8_t luminance = vsubq_s16(centerl, vreinterpretq_s16_u16(vmovl_u8(left)));
        luminance = vsubq_s16(luminance, vreinterpretq_s16_u16(vmovl_u8(right)));
        luminance = vshrq_n_s16(luminance, 1);
        uint8x8_t vout = vmovn_u16(vcgeq_s16(luminance, vblackPoint));
        vst1_u8(output.get() + y * width + x, vout);
      }
#else
      int left = line[x - 1] & 0xff;
      int center = line[x] & 0xff;
      for (; x < width - 1; x++) {
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
#endif
    }
  }
  if (failed)
    throw 1;
  return output;
}
