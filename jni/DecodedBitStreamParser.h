#ifndef DECODEDBITSTREAMPARSER_H
#define DECODEDBITSTREAMPARSER_H
#include "DecoderResult.h"
#include "ErrorCorrectionLevel.h"
#include <memory>
#include <stdint.h>
#include <string>
#include <vector>

class BitSource;
class Version;

/**
 * <p>QR Codes can encode text as bits in one of several modes, and can use
 * multiple modes
 * in one QR Code. This class decodes the bits back into text.</p>
 *
 * <p>See ISO 18004:2006, 6.4.3 - 6.4.7</p>
 *
 * @author Sean Owen
 */
class DecodedBitStreamParser final
{
public:
  enum class CharacterSetECI
  {

    // Enum name is a Java encoding valid for java.lang and java.io
    Cp437,
    ISO8859_1,
    ISO8859_2,
    ISO8859_3,
    ISO8859_4,
    ISO8859_5,
    ISO8859_6,
    ISO8859_7,
    ISO8859_8,
    ISO8859_9,
    ISO8859_10,
    ISO8859_11,
    ISO8859_13,
    ISO8859_14,
    ISO8859_15,
    ISO8859_16,
    SJIS,
    Cp1250,
    Cp1251,
    Cp1252,
    Cp1256,
    UnicodeBigUnmarked,
    UTF8,
    ASCII,
    Big5,
    GB18030,
    EUC_KR,
    Error
  };
  enum class Mode
  {

    TERMINATOR, // Not really a mode...
    NUMERIC,
    ALPHANUMERIC,
    STRUCTURED_APPEND, // Not supported
    BYTE,
    ECI, // character counts don't apply
    KANJI,
    FNC1_FIRST_POSITION,
    FNC1_SECOND_POSITION,
    /** See GBT 18284-2000; "Hanzi" is a transliteration of this mode name. */
    HANZI,

    /**
     * @param bits four bits encoding a QR Code data mode
     * @return Mode encoded by these bits
     * @throws IllegalArgumentException if bits do not correspond to a known
     * mode
     */
  };
  DecodedBitStreamParser() = delete;
  static std::unique_ptr<DecoderResult> decode(
    const std::vector<uint8_t>& bytes, const Version& version,
    ErrorCorrectionLevel ecLevel);

private:
  /**
   * See specification GBT 18284-2000
   */
  static void decodeHanziSegment(BitSource& bits, std::string& result,
                                 int count);

  static void decodeKanjiSegment(BitSource& bits, std::string& result,
                                 int count);

  static void decodeByteSegment(BitSource& bits, std::string result, int count,
                                CharacterSetECI currentCharacterSetECI,
                                DecoderResult::ByteSegments& byteSegments);

  static char toAlphaNumericChar(int value);

  static void decodeAlphanumericSegment(BitSource& bits, std::string& result,
                                        int count, bool fc1InEffect);

  static void decodeNumericSegment(BitSource& bits, std::string& result,
                                   int count);

  static int parseECIValue(BitSource& bits);
};
#endif /* DECODEDBITSTREAMPARSER_H */
