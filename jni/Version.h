#ifndef VERSION_H
#define VERSION_H
#include "LuminanceImage.h"
#include <initializer_list>
#include <memory>
#include <stdint.h>
#include <vector>

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

/**
 * <p>Encapsualtes the parameters for one error-correction block in one symbol
 * version.
 * This includes the number of data codewords, and the number of times a block
 * with these
 * parameters is used consecutively in the QR code version's format.</p>
 */
class ECB
{
private:
  int count;
  int dataCodewords;

public:
  ECB(int count, int dataCodewords)
  {
    this->count = count;
    this->dataCodewords = dataCodewords;
  }

  inline int getCount() { return count; }

  inline int getDataCodewords() { return dataCodewords; }
};

/**
 * <p>Encapsulates a set of error-correction blocks in one symbol version. Most
 * versions will
 * use blocks of differing sizes within one version, so, this encapsulates the
 * parameters for
 * each set of blocks. It also holds the number of error-correction codewords
 * per block since it
 * will be the same across all blocks within one version.</p>
 */
class ECBlocks
{
private:
  int ecCodewordsPerBlock;
  std::vector<ECB> ecBlocks;

public:
  ECBlocks(int ecCodewordsPerBlock, std::initializer_list<ECB> lists)
  {
    this->ecCodewordsPerBlock = ecCodewordsPerBlock;
    this->ecBlocks.insert(this->ecBlocks.end(), lists.begin(), lists.end());
  }

  inline int getECCodewordsPerBlock() { return ecCodewordsPerBlock; }

  int getNumBlocks();

  inline int getTotalECCodewords()
  {
    return ecCodewordsPerBlock * getNumBlocks();
  }

  inline std::vector<ECB>& getECBlocks() { return ecBlocks; }
};

class Version
{
public:
  static const Version& getProvisionalVersionForDimension(int dimension);

  static const Version& getVersionForNumber(int versionNumber);

  static const Version* decodeVersionInformation(int versionBits);

  inline int getVersionNumber() const { return versionNumber; }

  const std::vector<int>& getAlignmentPatternCenters() const
  {
    return alignmentPatternCenters;
  }

  int getTotalCodewords() const { return totalCodewords; }

  int getDimensionForVersion() const { return 17 + 4 * versionNumber; }

  const ECBlocks& getECBlocksForLevel(ErrorCorrectionLevel ecLevel) const
  {
    return (ecBlocks[static_cast<uint32_t>(ecLevel)]);
  }

private:
  static std::unique_ptr<Version[]> VERSIONS;
  static void ensureInitialize();

  int versionNumber;
  std::vector<int> alignmentPatternCenters;
  std::vector<ECBlocks> ecBlocks;
  int totalCodewords;

  Version(int versionNumber, std::initializer_list<int> alignmentPatternCenters,
          std::initializer_list<ECBlocks> b);
  /**
   * <p>Deduces version information purely from QR Code dimensions.</p>
   *
   * @param dimension dimension in modules
   * @return Version for a QR Code of that dimension
   * @throws FormatException if dimension is not 1 mod 4
   */
  /**
   * See ISO 18004:2006 6.5.1 Table 9
   */
  static void buildVersions();
};
#endif /* VERSION_H */
