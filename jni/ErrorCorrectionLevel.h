#ifndef ERRORCORRECTIONLEVEL_H
#define ERRORCORRECTIONLEVEL_H
#include <stdint.h>

enum class ErrorCorrectionLevel
{

  /** L = ~7% correction */
  L,
  /** M = ~15% correction */
  M,
  /** Q = ~25% correction */
  Q,
  /** H = ~30% correction */
  H
};

ErrorCorrectionLevel getErrorCorrectionLevelForBits(int bits);
#endif /* ERRORCORRECTIONLEVEL_H */
