#ifndef DATABLOCK_H
#define DATABLOCK_H
#include "ErrorCorrectionLevel.h"
#include <memory>
#include <vector>

class Version;
/**
 * <p>Encapsulates a block of data within a QR Code. QR Codes may split their
 * data into
 * multiple blocks, each of which is a unit of data and error-correction
 * codewords. Each
 * is represented by an instance of this class.</p>
 *
 * @author Sean Owen
 */
class DataBlock final
{

private:
  int numDataCodewords;
  std::vector<uint8_t> codewords;

  DataBlock(int numDataCodewords, std::vector<uint8_t>&& codewords);

public:
  /**
   * <p>When QR Codes use multiple data blocks, they are actually interleaved.
   * That is, the first byte of data block 1 to n is written, then the second
   * bytes, and so on. This
   * method will separate the data into original blocks.</p>
   *
   * @param rawCodewords bytes as read directly from the QR Code
   * @param version version of the QR Code
   * @param ecLevel error-correction level of the QR Code
   * @return DataBlocks containing original bytes, "de-interleaved" from
   * representation in the
   *         QR Code
   */
  static std::vector<std::unique_ptr<DataBlock>> getDataBlocks(
    std::vector<uint8_t>&& rawCodewords, const Version& version,
    ErrorCorrectionLevel ecLevel);

  int getNumDataCodewords() const { return numDataCodewords; }

  inline std::vector<uint8_t>& getCodewords() { return codewords; }
};

#endif /* DATABLOCK_H */
