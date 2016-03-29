#include "QRCodeDecoder.h"
#include "BitMatrixParser.h"
#include "DataBlock.h"
#include "DecodedBitStreamParser.h"
#include "FormatInformation.h"
#include "GenericGF.h"
#include "LuminanceImage.h"
#include "ReedSolomonDecoder.h"
#include "Version.h"

QRCodeDecoder::QRCodeDecoder()
  : rsDecoder(new ReedSolomonDecoder(&GenericGF::QR_CODE_FIELD_256))
{
}

std::unique_ptr<DecoderResult>
QRCodeDecoder::decode(LuminanceImage* bits)
{

  // Construct a parser and read version, error-correction level
  BitMatrixParser parser(bits);
  try {
    return decode(parser);
  } catch (int) {
  }

  // Revert the bit matrix
  parser.remask();

  // Will be attempting a mirrored reading of the version and format info.
  parser.setMirror(true);

  // Preemptively read the version.
  parser.readVersion();

  // Preemptively read the format information.
  parser.readFormatInformation();

  /*
   * Since we're here, this means we have successfully detected some kind
   * of version and format information when mirrored. This is a good sign,
   * that the QR code may be mirrored, and we should try once more with a
   * mirrored content.
   */
  // Prepare for a mirrored reading.
  parser.mirror();

  std::unique_ptr<DecoderResult> result = decode(parser);

  // Success! Notify the caller that the code was mirrored.
  // FIXME: enable this:
  // result.setOther(new QRCodeDecoderMetaData(true));

  return std::move(result);
}

std::unique_ptr<DecoderResult>
QRCodeDecoder::decode(BitMatrixParser& parser)
{
  const Version& version = parser.readVersion();
  ErrorCorrectionLevel ecLevel =
    parser.readFormatInformation().getErrorCorrectionLevel();

  // Read codewords
  std::vector<uint8_t> codewords(std::move(parser.readCodewords()));
  // Separate into data blocks
  auto dataBlocks =
    DataBlock::getDataBlocks(std::move(codewords), version, ecLevel);

  // Count total number of data bytes
  int totalBytes = 0;
  for (auto& dataBlock : dataBlocks) {
    totalBytes += dataBlock->getNumDataCodewords();
  }
  std::vector<uint8_t> resultBytes(totalBytes);
  int resultOffset = 0;

  // Error-correct and copy data blocks together into a stream of bytes
  for (auto& dataBlock : dataBlocks) {
    auto& codewordBytes = dataBlock->getCodewords();
    int numDataCodewords = dataBlock->getNumDataCodewords();
    correctErrors(codewordBytes, numDataCodewords);
    for (int i = 0; i < numDataCodewords; i++) {
      resultBytes[resultOffset++] = codewordBytes[i];
    }
  }

  // Decode the contents of that stream of bytes
  return DecodedBitStreamParser::decode(resultBytes, version, ecLevel);
}

void
QRCodeDecoder::correctErrors(std::vector<uint8_t>& codewordBytes,
                             int numDataCodewords)
{
  int numCodewords = codewordBytes.size();
  // First read into an array of ints
  std::vector<int> codewordsInts(numCodewords);
  for (int i = 0; i < numCodewords; i++) {
    codewordsInts[i] = codewordBytes[i] & 0xFF;
  }
  int numECCodewords = codewordBytes.size() - numDataCodewords;
  rsDecoder->decode(codewordsInts, numECCodewords);
  // Copy back into array of bytes -- only need to worry about the bytes that
  // were data
  // We don't care about errors in the error-correction codewords
  for (int i = 0; i < numDataCodewords; i++) {
    codewordBytes[i] = (char)codewordsInts[i];
  }
}
QRCodeDecoder::~QRCodeDecoder()
{
}
