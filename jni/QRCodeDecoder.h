#ifndef QRCODEDECODER_H
#define QRCODEDECODER_H
#include "DecoderResult.h"
#include <memory>

class ReedSolomonDecoder;
class LuminanceImage;
class BitMatrixParser;

class QRCodeDecoder final
{
public:
  QRCodeDecoder();
  ~QRCodeDecoder();
  std::unique_ptr<DecoderResult> decode(LuminanceImage* bits);

private:
  std::unique_ptr<ReedSolomonDecoder> rsDecoder;

  std::unique_ptr<DecoderResult> decode(BitMatrixParser& parser);

  /**
   * <p>Given data and error-correction codewords received, possibly corrupted
   * by errors, attempts to
   * correct the errors in-place using Reed-Solomon error correction.</p>
   *
   * @param codewordBytes data and error correction codewords
   * @param numDataCodewords number of codewords that are data bytes
   * @throws ChecksumException if error correction fails
   */
  void correctErrors(std::vector<uint8_t>& codewordBytes, int numDataCodewords);
};
#endif /* QRCODEDECODER_H */
