#include "BitMatrixParser.h"
#include "DataMask.h"
#include "FormatInformation.h"
#include "LuminanceImage.h"
#include "Version.h"

BitMatrixParser::BitMatrixParser(const LuminanceImage* _bitMatrix)
  : bitMatrix(_bitMatrix)
  , parsedVersion(nullptr)
  , mirror(false)
{
  int dimension = bitMatrix.getHeight();
  if (dimension < 21 || (dimension & 0x03) != 1) {
    throw FormatException.getFormatInstance();
  }
}

const FormatInformation&
BitMatrixParser::readFormatInformation()
{

  if (parsedFormatInfo.get()) {
    return *parsedFormatInfo;
  }

  // Read top-left format info bits
  int formatInfoBits1 = 0;
  for (int i = 0; i < 6; i++) {
    formatInfoBits1 = copyBit(i, 8, formatInfoBits1);
  }
  // .. and skip a bit in the timing pattern ...
  formatInfoBits1 = copyBit(7, 8, formatInfoBits1);
  formatInfoBits1 = copyBit(8, 8, formatInfoBits1);
  formatInfoBits1 = copyBit(8, 7, formatInfoBits1);
  // .. and skip a bit in the timing pattern ...
  for (int j = 5; j >= 0; j--) {
    formatInfoBits1 = copyBit(8, j, formatInfoBits1);
  }

  // Read the top-right/bottom-left pattern too
  int dimension = bitMatrix.getHeight();
  int formatInfoBits2 = 0;
  int jMin = dimension - 7;
  for (int j = dimension - 1; j >= jMin; j--) {
    formatInfoBits2 = copyBit(8, j, formatInfoBits2);
  }
  for (int i = dimension - 8; i < dimension; i++) {
    formatInfoBits2 = copyBit(i, 8, formatInfoBits2);
  }

  parsedFormatInfo = std::move(FormatInformation::decodeFormatInformation(
    formatInfoBits1, formatInfoBits2));
  if (parsedFormatInfo.get()) {
    return *parsedFormatInfo;
  }
  throw 1;
}
const Version&
BitMatrixParser::readVersion()
{

  if (parsedVersion) {
    return *parsedVersion;
  }

  int dimension = bitMatrix->getHeight();

  int provisionalVersion = (dimension - 17) / 4;
  if (provisionalVersion <= 6) {
    return Version::getVersionForNumber(provisionalVersion);
  }

  // Read top-right version info: 3 wide by 6 tall
  int versionBits = 0;
  int ijMin = dimension - 11;
  for (int j = 5; j >= 0; j--) {
    for (int i = dimension - 9; i >= ijMin; i--) {
      versionBits = copyBit(i, j, versionBits);
    }
  }

  const Version* theParsedVersion =
    Version::decodeVersionInformation(versionBits);
  if (theParsedVersion != nullptr &&
      theParsedVersion->getDimensionForVersion() == dimension) {
    parsedVersion = theParsedVersion;
    return *theParsedVersion;
  }

  // Hmm, failed. Try bottom left: 6 wide by 3 tall
  versionBits = 0;
  for (int i = 5; i >= 0; i--) {
    for (int j = dimension - 9; j >= ijMin; j--) {
      versionBits = copyBit(i, j, versionBits);
    }
  }

  theParsedVersion = Version.decodeVersionInformation(versionBits);
  if (theParsedVersion != nullptr &&
      theParsedVersion->getDimensionForVersion() == dimension) {
    parsedVersion = theParsedVersion;
    return *theParsedVersion;
  }
  throw 1;
}

int
BitMatrixParser::copyBit(int i, int j, int versionBits)
{
  bool bit = mirror ? bitMatrix->get(j, i) : bitMatrix->get(i, j);
  return bit ? (versionBits << 1) | 0x1 : versionBits << 1;
}

std::unique_ptr<uint8_t[]>
BitMatrixParser::readCodewords()
{

  const FormatInformation& formatInfo = readFormatInformation();
  const Version& version = readVersion();

  // Get the data mask for the format used in this QR Code. This will exclude
  // some bits from reading as we wind through the bit matrix.
  const DataMask& dataMask = DataMask.forReference(formatInfo.getDataMask());
  int dimension = bitMatrix.getHeight();
  dataMask.unmaskBitMatrix(bitMatrix, dimension);

  std::unique_ptr<LuminanceImage> functionPattern =
    version.buildFunctionPattern();

  bool readingUp = true;
  std::unique_ptr<uint8_t[]> result(new uint8_t[version.getTotalCodewords()]);
  int resultOffset = 0;
  int currentByte = 0;
  int bitsRead = 0;
  // Read columns in pairs, from right to left
  for (int j = dimension - 1; j > 0; j -= 2) {
    if (j == 6) {
      // Skip whole column with vertical alignment pattern;
      // saves time and makes the other code proceed more cleanly
      j--;
    }
    // Read alternatingly from bottom to top then top to bottom
    for (int count = 0; count < dimension; count++) {
      int i = readingUp ? dimension - 1 - count : count;
      for (int col = 0; col < 2; col++) {
        // Ignore bits covered by the function pattern
        if (!functionPattern->get(j - col, i)) {
          // Read a bit
          bitsRead++;
          currentByte <<= 1;
          if (bitMatrix->get(j - col, i)) {
            currentByte |= 1;
          }
          // If we've made a whole byte, save it off
          if (bitsRead == 8) {
            result.get()[resultOffset++] = static_cast<uint8_t>(currentByte);
            bitsRead = 0;
            currentByte = 0;
          }
        }
      }
    }
    readingUp ^= true; // readingUp = !readingUp; // switch directions
  }
  if (resultOffset != version.getTotalCodewords()) {
    throw 1;
  }
  return result;
}

void
BitMatrixParser::remask()
{
  if (parsedFormatInfo.get() == nullptr) {
    return; // We have no format information, and have no data mask
  }
  const DataMask& dataMask =
    DataMask.forReference(parsedFormatInfo->getDataMask());
  int dimension = bitMatrix.getHeight();
  dataMask.unmaskBitMatrix(bitMatrix, dimension);
}

void
BitMatrixParser::setMirror(bool mirror)
{
  parsedVersion = null;
  parsedFormatInfo = null;
  this.mirror = mirror;
}

void
BitMatrixParser::mirror()
{
  for (int x = 0; x < bitMatrix->getWidth(); x++) {
    for (int y = x + 1; y < bitMatrix->getHeight(); y++) {
      if (bitMatrix->get(x, y) != bitMatrix->get(y, x)) {
        bitMatrix.flip(y, x);
        bitMatrix.flip(x, y);
      }
    }
  }
}
