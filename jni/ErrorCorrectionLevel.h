#ifndef ERRORCORRECTIONLEVEL_H
#define ERRORCORRECTIONLEVEL_H
#include <stdint.h>

enum class ErrorCorrectionLevel : uint32_t
{

  /** L = ~7% correction */
  L = (0x01),
  /** M = ~15% correction */
  M = (0x00),
  /** Q = ~25% correction */
  Q = (0x03),
  /** H = ~30% correction */
  H = (0x02)
};

ErrorCorrectionLevel getErrorCorrectionLevelForBits(int bits);
#endif /* ERRORCORRECTIONLEVEL_H */
