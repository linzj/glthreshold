#include "ErrorCorrectionLevel.h"
#include <exception>
static const ErrorCorrectionLevel FOR_BITS[] = { ErrorCorrectionLevel::M,
                                                 ErrorCorrectionLevel::L,
                                                 ErrorCorrectionLevel::H,
                                                 ErrorCorrectionLevel::Q };

ErrorCorrectionLevel
getErrorCorrectionLevelForBits(int bits)
{
  if (bits < 0 || bits >= 4) {
    throw std::exception();
  }
  return FOR_BITS[bits];
}
