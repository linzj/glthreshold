#ifndef FORMATINFORMATION_H
#define FORMATINFORMATION_H
#include "ErrorCorrectionLevel.h"
#include <memory>

class FormatInformation
{
public:
  static int numBitsDiffering(unsigned a, unsigned b);

  /**
   * @param maskedFormatInfo1 format info indicator, with mask still applied
   * @param maskedFormatInfo2 second copy of same info; both are checked at the
   * same time
   *  to establish best match
   * @return information about the format it specifies, or {@code null}
   *  if doesn't seem to match any known pattern
   */
  static std::unique_ptr<FormatInformation> decodeFormatInformation(
    int maskedFormatInfo1, int maskedFormatInfo2);

  inline ErrorCorrectionLevel getErrorCorrectionLevel()
  {
    return errorCorrectionLevel;
  }

  uint8_t getDataMask() { return dataMask; }

private:
  ErrorCorrectionLevel errorCorrectionLevel;
  uint8_t dataMask;

  explicit FormatInformation(int formatInfo);

  static std::unique_ptr<FormatInformation> doDecodeFormatInformation(
    int maskedFormatInfo1, int maskedFormatInfo2);
};
#endif /* FORMATINFORMATION_H */
