#include "DecodedBitStreamParser.h"
#include "BitSource.h"
#include "Version.h"
#include <algorithm>
#include <iterator>
#include <string.h>
#include <unordered_map>

/**
 * See ISO 18004:2006, 6.4.4 Table 5
 */

static const char ALPHANUMERIC_CHARS[] = {
  '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E',
  'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T',
  'U', 'V', 'W', 'X', 'Y', 'Z', ' ', '$', '%', '*', '+', '-', '.', '/', ':'
};
static const int GB2312_SUBSET = 1;

static DecodedBitStreamParser::Mode
forBits(int bits)
{
  switch (bits) {
    case 0x0:
      return DecodedBitStreamParser::Mode::TERMINATOR;
    case 0x1:
      return DecodedBitStreamParser::Mode::NUMERIC;
    case 0x2:
      return DecodedBitStreamParser::Mode::ALPHANUMERIC;
    case 0x3:
      return DecodedBitStreamParser::Mode::STRUCTURED_APPEND;
    case 0x4:
      return DecodedBitStreamParser::Mode::BYTE;
    case 0x5:
      return DecodedBitStreamParser::Mode::FNC1_FIRST_POSITION;
    case 0x7:
      return DecodedBitStreamParser::Mode::ECI;
    case 0x8:
      return DecodedBitStreamParser::Mode::KANJI;
    case 0x9:
      return DecodedBitStreamParser::Mode::FNC1_SECOND_POSITION;
    case 0xD:
      // 0xD is defined in GBT 18284-2000, may not be supported in foreign
      // country
      return DecodedBitStreamParser::Mode::HANZI;
    default:
      throw 1;
  }
}
struct ModeInfo
{
  int characterCountBitsForVersions[3];
  int bits;
};

static const ModeInfo&
getModeInfo(DecodedBitStreamParser::Mode mode)
{
  static std::unordered_map<int, ModeInfo> map = {
    { static_cast<int>(DecodedBitStreamParser::Mode::TERMINATOR),
      { { 0, 0, 0 }, 0x00 } }, // Not really a mode...
    { static_cast<int>(DecodedBitStreamParser::Mode::NUMERIC),
      { { 10, 12, 14 }, 0x01 } },
    { static_cast<int>(DecodedBitStreamParser::Mode::ALPHANUMERIC),
      { { 9, 11, 13 }, 0x02 } },
    { static_cast<int>(DecodedBitStreamParser::Mode::STRUCTURED_APPEND),
      { { 0, 0, 0 }, 0x03 } }, // Not supported
    { static_cast<int>(DecodedBitStreamParser::Mode::BYTE),
      { { 8, 16, 16 }, 0x04 } },
    { static_cast<int>(DecodedBitStreamParser::Mode::ECI),
      { { 0, 0, 0 }, 0x07 } }, // character counts don't apply
    { static_cast<int>(DecodedBitStreamParser::Mode::KANJI),
      { { 8, 10, 12 }, 0x08 } },
    { static_cast<int>(DecodedBitStreamParser::Mode::FNC1_FIRST_POSITION),
      { { 0, 0, 0 }, 0x05 } },
    { static_cast<int>(DecodedBitStreamParser::Mode::FNC1_SECOND_POSITION),
      { { 0, 0, 0 }, 0x09 } },
    /** See GBT 18284-2000; "Hanzi" is a transliteration of this mode name. */
    { static_cast<int>(DecodedBitStreamParser::Mode::HANZI),
      { { 8, 10, 12 }, 0x0D } },
  };
  auto found = map.find(static_cast<int>(mode));
  if (found == map.end()) {
    throw 1;
  }
  return found->second;
}
/**
 * @param version version in question
 * @return number of bits used, in this QR Code symbol {@link Version}, to
 * encode the
 *         count of characters that will follow encoded in this Mode
 */
static int
getCharacterCountBits(DecodedBitStreamParser::Mode mode, const Version& version)
{
  int number = version.getVersionNumber();
  int offset;
  if (number <= 9) {
    offset = 0;
  } else if (number <= 26) {
    offset = 1;
  } else {
    offset = 2;
  }
  return getModeInfo(mode).characterCountBitsForVersions[offset];
}

static int
getBits(DecodedBitStreamParser::Mode mode)
{
  return getModeInfo(mode).bits;
}

static DecodedBitStreamParser::CharacterSetECI
getCharacterSetECIByValue(int value)
{
  if (value < 0 || value >= 900) {
    throw 1;
  }
  static std::unordered_map<int, DecodedBitStreamParser::CharacterSetECI> map =
    {
      { 0, DecodedBitStreamParser::CharacterSetECI::Cp437 },
      { 2, DecodedBitStreamParser::CharacterSetECI::Cp437 },
      { 1, DecodedBitStreamParser::CharacterSetECI::ISO8859_1 },
      { 3, DecodedBitStreamParser::CharacterSetECI::ISO8859_1 },
      { 4, DecodedBitStreamParser::CharacterSetECI::ISO8859_2 },
      { 5, DecodedBitStreamParser::CharacterSetECI::ISO8859_3 },
      { 6, DecodedBitStreamParser::CharacterSetECI::ISO8859_4 },
      { 7, DecodedBitStreamParser::CharacterSetECI::ISO8859_5 },
      { 8, DecodedBitStreamParser::CharacterSetECI::ISO8859_6 },
      { 9, DecodedBitStreamParser::CharacterSetECI::ISO8859_7 },
      { 10, DecodedBitStreamParser::CharacterSetECI::ISO8859_8 },
      { 11, DecodedBitStreamParser::CharacterSetECI::ISO8859_9 },
      { 12, DecodedBitStreamParser::CharacterSetECI::ISO8859_10 },
      { 13, DecodedBitStreamParser::CharacterSetECI::ISO8859_11 },
      { 15, DecodedBitStreamParser::CharacterSetECI::ISO8859_13 },
      { 16, DecodedBitStreamParser::CharacterSetECI::ISO8859_14 },
      { 17, DecodedBitStreamParser::CharacterSetECI::ISO8859_15 },
      { 18, DecodedBitStreamParser::CharacterSetECI::ISO8859_16 },
      { 20, DecodedBitStreamParser::CharacterSetECI::SJIS },
      { 21, DecodedBitStreamParser::CharacterSetECI::Cp1250 },
      { 22, DecodedBitStreamParser::CharacterSetECI::Cp1251 },
      { 23, DecodedBitStreamParser::CharacterSetECI::Cp1252 },
      { 24, DecodedBitStreamParser::CharacterSetECI::Cp1256 },
      { 25, DecodedBitStreamParser::CharacterSetECI::UnicodeBigUnmarked },
      { 26, DecodedBitStreamParser::CharacterSetECI::UTF8 },
      { 27, DecodedBitStreamParser::CharacterSetECI::ASCII },
      { 170, DecodedBitStreamParser::CharacterSetECI::ASCII },
      { 28, DecodedBitStreamParser::CharacterSetECI::Big5 },
      { 29, DecodedBitStreamParser::CharacterSetECI::GB18030 },
      { 30, DecodedBitStreamParser::CharacterSetECI::EUC_KR },
    };
  auto found = map.find(value);
  if (found == map.end()) {
    throw 1;
  }
  return found->second;
}

std::unique_ptr<DecoderResult>
DecodedBitStreamParser::decode(const std::vector<uint8_t>& bytes,
                               const Version& version,
                               ErrorCorrectionLevel ecLevel)
{
  BitSource bits(bytes);
  std::string result;
  DecoderResult::ByteSegments byteSegments;
  int symbolSequence = -1;
  int parityData = -1;
  CharacterSetECI currentCharacterSetECI = CharacterSetECI::ISO8859_1;
  bool fc1InEffect = false;
  Mode mode;
  do {
    // While still another segment to read...
    if (bits.available() < 4) {
      // OK, assume we're done. Really, a TERMINATOR mode should have been
      // recorded here
      mode = Mode::TERMINATOR;
    } else {
      mode = forBits(bits.readBits(4)); // mode is encoded by 4 bits
    }
    if (mode != Mode::TERMINATOR) {
      if (mode == Mode::FNC1_FIRST_POSITION ||
          mode == Mode::FNC1_SECOND_POSITION) {
        // We do little with FNC1 except alter the parsed result a bit according
        // to the spec
        fc1InEffect = true;
      } else if (mode == Mode::STRUCTURED_APPEND) {
        if (bits.available() < 16) {
          throw 1;
        }
        // sequence number and parity is added later to the result metadata
        // Read next 8 bits (symbol sequence #) and 8 bits (parity data), then
        // continue
        symbolSequence = bits.readBits(8);
        parityData = bits.readBits(8);
      } else if (mode == Mode::ECI) {
        // Count doesn't apply to ECI
        int value = parseECIValue(bits);
        currentCharacterSetECI = getCharacterSetECIByValue(value);
      } else {
        // First handle Hanzi mode which does not start with character count
        if (mode == Mode::HANZI) {
          // chinese mode contains a sub set indicator right after mode
          // indicator
          int subset = bits.readBits(4);
          int countHanzi = bits.readBits(getCharacterCountBits(mode, version));
          if (subset == GB2312_SUBSET) {
            decodeHanziSegment(bits, result, countHanzi);
          }
        } else {
          // "Normal" QR code modes:
          // How many characters will follow, encoded in this mode?
          int count = bits.readBits(getCharacterCountBits(mode, version));
          if (mode == Mode::NUMERIC) {
            decodeNumericSegment(bits, result, count);
          } else if (mode == Mode::ALPHANUMERIC) {
            decodeAlphanumericSegment(bits, result, count, fc1InEffect);
          } else if (mode == Mode::BYTE) {
            decodeByteSegment(bits, result, count, currentCharacterSetECI,
                              byteSegments);
          } else if (mode == Mode::KANJI) {
            decodeKanjiSegment(bits, result, count);
          } else {
            throw 1;
          }
        }
      }
    }
  } while (mode != Mode::TERMINATOR);

  return std::unique_ptr<DecoderResult>(
    new DecoderResult(bytes, std::move(result),
                      byteSegments.empty() ? DecoderResult::ByteSegments()
                                           : std::move(byteSegments),
                      // FIXME: enable this:
                      // ecLevel == null ? null : ecLevel.toString(),
                      "", symbolSequence, parityData));
}

void
DecodedBitStreamParser::decodeHanziSegment(BitSource& bits, std::string& result,
                                           int count)
{
  // Don't crash trying to read more bits than we have available.
  if (count * 13 > bits.available()) {
    throw 1;
  }

  // Each character will require 2 bytes. Read the characters as 2-byte pairs
  // and decode as GB2312 afterwards
  char buffer[2 * count];
  int offset = 0;
  while (count > 0) {
    // Each 13 bits encodes a 2-byte character
    int twoBytes = bits.readBits(13);
    int assembledTwoBytes = ((twoBytes / 0x060) << 8) | (twoBytes % 0x060);
    if (assembledTwoBytes < 0x003BF) {
      // In the 0xA1A1 to 0xAAFE range
      assembledTwoBytes += 0x0A1A1;
    } else {
      // In the 0xB0A1 to 0xFAFE range
      assembledTwoBytes += 0x0A6A1;
    }
    buffer[offset] = (char)((assembledTwoBytes >> 8) & 0xFF);
    buffer[offset + 1] = (char)(assembledTwoBytes & 0xFF);
    offset += 2;
    count--;
  }

  // FIXME: decode from gb2312 to utf-8 first.
  result.append(buffer, 2 * count);
}

void
DecodedBitStreamParser::decodeKanjiSegment(BitSource& bits, std::string& result,
                                           int count)
{
  // Don't crash trying to read more bits than we have available.
  if (count * 13 > bits.available()) {
    throw 1;
  }

  // Each character will require 2 bytes. Read the characters as 2-byte pairs
  // and decode as Shift_JIS afterwards
  char buffer[2 * count];
  int offset = 0;
  while (count > 0) {
    // Each 13 bits encodes a 2-byte character
    int twoBytes = bits.readBits(13);
    int assembledTwoBytes = ((twoBytes / 0x0C0) << 8) | (twoBytes % 0x0C0);
    if (assembledTwoBytes < 0x01F00) {
      // In the 0x8140 to 0x9FFC range
      assembledTwoBytes += 0x08140;
    } else {
      // In the 0xE040 to 0xEBBF range
      assembledTwoBytes += 0x0C140;
    }
    buffer[offset] = (char)(assembledTwoBytes >> 8);
    buffer[offset + 1] = (char)assembledTwoBytes;
    offset += 2;
    count--;
  }
  // Shift_JIS may not be supported in some environments:
  //
  // FIXME: decode from gb2312 to utf-8 first.
  result.append(buffer, 2 * count);
}

void
DecodedBitStreamParser::decodeByteSegment(
  BitSource& bits, std::string result, int count,
  CharacterSetECI currentCharacterSetECI,
  DecoderResult::ByteSegments& byteSegments)
{
  // Don't crash trying to read more bits than we have available.
  if (8 * count > bits.available()) {
    throw 1;
  }

  char readBytes[count];
  for (int i = 0; i < count; i++) {
    readBytes[i] = (char)bits.readBits(8);
  }
// FIXME: use encoding
#if 0
    String encoding;
    if (currentCharacterSetECI == null) {
      // The spec isn't clear on this mode; see
      // section 6.4.5: t does not say which encoding to assuming
      // upon decoding. I have seen ISO-8859-1 used as well as
      // Shift_JIS -- without anything like an ECI designator to
      // give a hint.
      encoding = StringUtils.guessEncoding(readBytes, hints);
    } else {
      encoding = currentCharacterSetECI.name();
    }

    try {
      result.append(new String(readBytes, encoding));
    } catch (UnsupportedEncodingException ignored) {
      throw FormatException.getFormatInstance();
    }
#endif
  result.append(readBytes, count);
  std::vector<uint8_t> tmp(count);
  memcpy(const_cast<uint8_t*>(tmp.data()), readBytes, count);
  byteSegments.push_back(std::move(tmp));
}

char
DecodedBitStreamParser::toAlphaNumericChar(int value)
{
  if (value >= sizeof(ALPHANUMERIC_CHARS)) {
    throw 1;
  }
  return ALPHANUMERIC_CHARS[value];
}

void
DecodedBitStreamParser::decodeAlphanumericSegment(BitSource& bits,
                                                  std::string& result,
                                                  int count, bool fc1InEffect)
{
  // Read two characters at a time
  int start = result.length();
  while (count > 1) {
    if (bits.available() < 11) {
      throw 1;
    }
    int nextTwoCharsBits = bits.readBits(11);
    result.push_back(toAlphaNumericChar(nextTwoCharsBits / 45));
    result.push_back(toAlphaNumericChar(nextTwoCharsBits % 45));
    count -= 2;
  }
  if (count == 1) {
    // special case: one character left
    if (bits.available() < 6) {
      throw 1;
    }
    result.push_back(toAlphaNumericChar(bits.readBits(6)));
  }
  // See section 6.4.8.1, 6.4.8.2
  if (fc1InEffect) {
    // We need to massage the result a bit if in an FNC1 mode:
    for (int i = start; i < result.length(); i++) {
      if (result[i] == '%') {
        if (i < result.length() - 1 && result.at(i + 1) == '%') {
          // %% is rendered as %
          result.erase(i + 1, 1);
        } else {
          // In alpha mode, % should be converted to FNC1 separator 0x1D
          result[i] = (char)0x1D;
        }
      }
    }
  }
}

void
DecodedBitStreamParser::decodeNumericSegment(BitSource& bits,
                                             std::string& result, int count)
{
  // Read three digits at a time
  while (count >= 3) {
    // Each 10 bits encodes three digits
    if (bits.available() < 10) {
      throw 1;
    }
    int threeDigitsBits = bits.readBits(10);
    if (threeDigitsBits >= 1000) {
      throw 1;
    }
    result.push_back(toAlphaNumericChar(threeDigitsBits / 100));
    result.push_back(toAlphaNumericChar((threeDigitsBits / 10) % 10));
    result.push_back(toAlphaNumericChar(threeDigitsBits % 10));
    count -= 3;
  }
  if (count == 2) {
    // Two digits left over to read, encoded in 7 bits
    if (bits.available() < 7) {
      throw 1;
    }
    int twoDigitsBits = bits.readBits(7);
    if (twoDigitsBits >= 100) {
      throw 1;
    }
    result.push_back(toAlphaNumericChar(twoDigitsBits / 10));
    result.push_back(toAlphaNumericChar(twoDigitsBits % 10));
  } else if (count == 1) {
    // One digit left over to read
    if (bits.available() < 4) {
      throw 1;
    }
    int digitBits = bits.readBits(4);
    if (digitBits >= 10) {
      throw 1;
    }
    result.push_back(toAlphaNumericChar(digitBits));
  }
}

int
DecodedBitStreamParser::parseECIValue(BitSource& bits)
{
  int firstByte = bits.readBits(8);
  if ((firstByte & 0x80) == 0) {
    // just one byte
    return firstByte & 0x7F;
  }
  if ((firstByte & 0xC0) == 0x80) {
    // two bytes
    int secondByte = bits.readBits(8);
    return ((firstByte & 0x3F) << 8) | secondByte;
  }
  if ((firstByte & 0xE0) == 0xC0) {
    // three bytes
    int secondThirdBytes = bits.readBits(16);
    return ((firstByte & 0x1F) << 16) | secondThirdBytes;
  }
  throw 1;
}
