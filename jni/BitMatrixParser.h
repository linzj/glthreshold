#ifndef BITMATRIXPARSER_H
#define BITMATRIXPARSER_H
#include <memory>
#include <vector>

class LuminanceImage;
class Version;
class FormatInformation;

class BitMatrixParser
{

private:
  LuminanceImage* bitMatrix;
  const Version* parsedVersion;
  std::unique_ptr<FormatInformation> parsedFormatInfo;
  bool mirror_;

public:
  /**
   * @param bitMatrix {@link BitMatrix} to parse
   * @throws FormatException if dimension is not >= 21 and 1 mod 4
   */
  BitMatrixParser(LuminanceImage* bitMatrix);

  /**
   * <p>Reads format information from one of its two locations within the QR
   * Code.</p>
   *
   * @return {@link FormatInformation} encapsulating the QR Code's format info
   * @throws FormatException if both format information locations cannot be
   * parsed as
   * the valid encoding of format information
   */
  const FormatInformation& readFormatInformation();

  /**
   * <p>Reads version information from one of its two locations within the QR
   * Code.</p>
   *
   * @return {@link Version} encapsulating the QR Code's version
   * @throws FormatException if both version information locations cannot be
   * parsed as
   * the valid encoding of version information
   */
  const Version& readVersion();

  /**
   * <p>Reads the bits in the {@link BitMatrix} representing the finder pattern
   * in the
   * correct order in order to reconstruct the codewords bytes contained within
   * the
   * QR Code.</p>
   *
   * @return bytes encoded within the QR Code
   * @throws FormatException if the exact number of bytes expected is not read
   */
  std::vector<uint8_t> readCodewords();

  /**
   * Revert the mask removal done while reading the code words. The bit matrix
   * should revert to its original state.
   */
  void remask();

  /**
   * Prepare the parser for a mirrored operation.
   * This flag has effect only on the {@link #readFormatInformation()} and the
   * {@link #readVersion()}. Before proceeding with {@link #readCodewords()} the
   * {@link #mirror()} method should be called.
   *
   * @param mirror Whether to read version and format information mirrored.
   */
  void setMirror(bool mirror);

  /** Mirror the bit matrix in order to attempt a second reading. */
  void mirror();

private:
  int copyBit(int i, int j, int versionBits);
};

#endif /* BITMATRIXPARSER_H */
