#include "RGBToLuminance.h"
#include "log.h"
#if defined(__i386__) || defined(__x86_64__)
#define USE_INTEL_SIMD 1
#else
#define USE_INTEL_SIMD 0
#endif
#if defined(__arm__)
#define USE_NEON_SIMD 0
#else
#define USE_NEON_SIMD 0
#endif

#if USE_INTEL_SIMD
#include <emmintrin.h>
#endif

#if USE_NEON_SIMD
#include <arm_neon.h>
#endif

std::unique_ptr<uint8_t[]>
RGBToLuminance(int width, int height, const void* data)
{
  int rowBytes = (width * 24 + 31) / 32 * 4;
  std::unique_ptr<uint8_t[]> result(new uint8_t[width * height]);
  uint8_t* rp = result.get();
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
  const __m128i factor1 = _mm_set_epi16(601, 601, 601, 601, 306, 306, 306, 306);
  const __m128i factor2 = _mm_set_epi16(0, 0, 0, 0, 117, 117, 117, 117);
  const __m128i zero = _mm_set_epi16(0, 0, 0, 0, 0, 0, 0, 0);
#endif
#if USE_NEON_SIMD
  const uint32x4_t rmask = { 0xff, 0xff, 0xff, 0xff };
  const uint32x4_t gmask = { 0xff00, 0xff00, 0xff00, 0xff00 };
  const uint32x4_t bmask = { 0xff0000, 0xff0000, 0xff0000, 0xff0000 };

#endif
  // #pragma omp parallel
  {
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
          sum1 = _mm_shuffle_epi32(sum1, _MM_SHUFFLE(3, 1, 2, 0));
        }
        {
          __m128i two =
            _mm_load_si128(reinterpret_cast<const __m128i*>(line) + 1);
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
          sum2 = _mm_shuffle_epi32(sum2, _MM_SHUFFLE(3, 1, 2, 0));
        }
        __m128i sum = _mm_packs_epi32(sum1, sum2);
        __m128i lrpspacked = _mm_packus_epi16(sum, zero);
        _mm_storel_epi64(reinterpret_cast<__m128i*>(lrp), lrpspacked);
      }
#endif // USE_INTEL_SIMD
      __builtin_prefetch(line);
#if USE_NEON_SIMD
      int aligned_width = width & ~7;
      for (; x < aligned_width; x += 8, line += 32, lrp += 8) {
        uint16x4_t result1, result2;
        uint8x8_t result;
        {
          uint32x4_t tmp, r, g, b;
          uint16x4_t r16, g16, b16;
          uint32x4_t factoredr, factoredg, factoredb;
          uint32x4_t sum;

          tmp = vld1q_u32(reinterpret_cast<const uint32_t*>(line));
          r = vandq_u32(tmp, rmask);
          r16 = vmovn_u32(r);
          g = vandq_u32(tmp, gmask);
          g16 = vqrshrun_n_s32((int32x4_t)g, 16);
          b = vandq_u32(tmp, bmask);
          b16 = vqrshrun_n_s32((int32x4_t)b, 16);
          b16 = vshr_n_u16(b16, 8);
          factoredr = vmull_n_u16(r16, 306);
          factoredg = vmull_n_u16(g16, 601);
          factoredb = vmull_n_u16(b16, 117);
          sum = vaddq_u32(factoredr, factoredg);
          sum = vaddq_u32(sum, factoredb);
          result1 = vqrshrun_n_s32((int32x4_t)sum, 10);
        }
        {
          uint32x4_t tmp, r, g, b;
          uint16x4_t r16, g16, b16;
          uint32x4_t factoredr, factoredg, factoredb;
          uint32x4_t sum;

          tmp = vld1q_u32(reinterpret_cast<const uint32_t*>(line) + 4);
          r = vandq_u32(tmp, rmask);
          r16 = vmovn_u32(r);
          g = vandq_u32(tmp, gmask);
          g16 = vqrshrun_n_s32((int32x4_t)g, 16);
          b = vandq_u32(tmp, bmask);
          b16 = vqrshrun_n_s32((int32x4_t)b, 16);
          b16 = vshr_n_u16(b16, 8);
          factoredr = vmull_n_u16(r16, 306);
          factoredg = vmull_n_u16(g16, 601);
          factoredb = vmull_n_u16(b16, 117);
          sum = vaddq_u32(factoredr, factoredg);
          sum = vaddq_u32(sum, factoredb);
          result2 = vqrshrun_n_s32((int32x4_t)sum, 10);
        }
        result = vmovn_u16(vcombine_u16(result1, result2));
        *((uint8x8_t*)lrp) = result;
      }
#endif // USE_NEON_SIMD
      for (; x < width; ++x, line += 4, lrp += 1) {
        uint8_t val =
          (306 * (line[0]) + 601 * (line[1]) + 117 * (line[2])) >> 10;
        *lrp = val;
      }
    }
  }
  return std::move(result);
}
