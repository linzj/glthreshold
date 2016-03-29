#include "RGBToLuminance.h"
#if defined(__i386__) || defined(__x86_64__)
#define USE_INTEL_SIMD 1
#else
#define USE_INTEL_SIMD 0
#endif

#if USE_INTEL_SIMD
#include <emmintrin.h>
#endif

std::unique_ptr<uint8_t[]>
RGBToLuminance(int width, int height, const void* data)
{
  int rowBytes = (width * 24 + 31) / 32 * 4;
  std::unique_ptr<uint8_t[]> result(new uint8_t[width * height]);
  uint8_t* rp = result.get();
#pragma omp parallel
  {
#pragma omp parallel for
    for (int y = 0; y < height; ++y) {
      const uint8_t* line = static_cast<const uint8_t*>(data) + y * rowBytes;
      uint8_t* lrp = rp + y * width;
      for (int x = 0; x < width; ++x, line += 3, lrp += 1) {
        *lrp = (306 * (line[0]) + 601 * (line[1]) + 117 * (line[2])) >> 10;
      }
    }
  }
  return std::move(result);
}

std::unique_ptr<uint8_t[]>
RGBAToLuminance(int width, int height, const void* data)
{
  int rowBytes = width * 4;
  std::unique_ptr<uint8_t[]> result(new uint8_t[width * height]);
  uint8_t* rp = result.get();
#if USE_INTEL_SIMD
  __m128i factor1 = _mm_set_epi16(601, 601, 601, 601, 306, 306, 306, 306);
  __m128i factor2 = _mm_set_epi16(0, 0, 0, 0, 117, 117, 117, 117);
  __m128i zero = _mm_set_epi16(0, 0, 0, 0, 0, 0, 0, 0);
#endif
#pragma omp parallel
  {
#pragma omp parallel for
    for (int y = 0; y < height; ++y) {
      const uint8_t* line = static_cast<const uint8_t*>(data) + y * rowBytes;
      uint8_t* lrp = rp + y * width;
      int x = 0;
#if USE_INTEL_SIMD
      int aligned_width = width & ~7;
      for (; x < aligned_width; x += 8, line += 32, lrp += 8) {
        __m128i sum1, sum2;
        {
          __m128i two = _mm_load_si128(reinterpret_cast<const __m128i*>(line));
          __m128i first = _mm_unpacklo_epi8(two, zero);
          __m128i second = _mm_unpackhi_epi8(two, zero);
          __m128i syn1 = _mm_unpacklo_epi16(first, second);
          __m128i syn2 = _mm_unpackhi_epi16(first, second);
          __m128i syn3 = _mm_unpacklo_epi32(syn1, syn2);
          __m128i syn4 = _mm_unpackhi_epi32(syn1, syn2);
          __m128i mullofirst = _mm_mullo_epi16(syn3, factor1);
          __m128i mullosecond = _mm_mullo_epi16(syn4, factor2);
          __m128i mulhifirst = _mm_mulhi_epi16(syn3, factor1);
          __m128i mulhisecond = _mm_mulhi_epi16(syn4, factor2);
          __m128i fourR = _mm_unpacklo_epi16(mullofirst, mulhifirst);
          __m128i fourG = _mm_unpackhi_epi16(mullofirst, mulhifirst);
          __m128i fourB = _mm_unpacklo_epi16(mullosecond, mulhisecond);
          sum1 = _mm_add_epi32(fourR, fourG);
          sum1 = _mm_add_epi32(sum1, fourB);
          sum1 = _mm_srli_epi32(sum1, 10);
        }
        {
          __m128i two = _mm_load_si128(reinterpret_cast<const __m128i*>(line) + 1);
          __m128i first = _mm_unpacklo_epi8(two, zero);
          __m128i second = _mm_unpackhi_epi8(two, zero);
          __m128i syn1 = _mm_unpacklo_epi16(first, second);
          __m128i syn2 = _mm_unpackhi_epi16(first, second);
          __m128i syn3 = _mm_unpacklo_epi32(syn1, syn2);
          __m128i syn4 = _mm_unpackhi_epi32(syn1, syn2);
          __m128i mullofirst = _mm_mullo_epi16(syn3, factor1);
          __m128i mullosecond = _mm_mullo_epi16(syn4, factor2);
          __m128i mulhifirst = _mm_mulhi_epi16(syn3, factor1);
          __m128i mulhisecond = _mm_mulhi_epi16(syn4, factor2);
          __m128i fourR = _mm_unpacklo_epi16(mullofirst, mulhifirst);
          __m128i fourG = _mm_unpackhi_epi16(mullofirst, mulhifirst);
          __m128i fourB = _mm_unpacklo_epi16(mullosecond, mulhisecond);
          sum2 = _mm_add_epi32(fourR, fourG);
          sum2 = _mm_add_epi32(sum2, fourB);
          sum2 = _mm_srli_epi32(sum2, 10);
        }
        __m128i sum = _mm_packs_epi32(sum1, sum2);
        __m128i lrpspacked = _mm_packus_epi16(sum, zero);
        _mm_storel_epi64(reinterpret_cast<__m128i*>(lrp), lrpspacked);
      }
#endif
      for (; x < width; ++x, line += 4, lrp += 1) {
        *lrp = (306 * (line[0]) + 601 * (line[1]) + 117 * (line[2])) >> 10;
      }
    }
  }
  return std::move(result);
}
