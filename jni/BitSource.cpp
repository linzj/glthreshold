#include "BitSource.h"

BitSource::BitSource(const std::vector<uint8_t>& _bytes)
  : bytes(_bytes)
{
}

int
BitSource::readBits(int numBits)
{
  if (numBits < 1 || numBits > 32 || numBits > available()) {
    throw 1;
  }

  int result = 0;

  // First, read remainder from current byte
  if (bitOffset > 0) {
    int bitsLeft = 8 - bitOffset;
    int toRead = numBits < bitsLeft ? numBits : bitsLeft;
    int bitsToNotRead = bitsLeft - toRead;
    int mask = (0xFF >> (8 - toRead)) << bitsToNotRead;
    result = (bytes[byteOffset] & mask) >> bitsToNotRead;
    numBits -= toRead;
    bitOffset += toRead;
    if (bitOffset == 8) {
      bitOffset = 0;
      byteOffset++;
    }
  }

  // Next read whole bytes
  if (numBits > 0) {
    while (numBits >= 8) {
      result = (result << 8) | (bytes[byteOffset] & 0xFF);
      byteOffset++;
      numBits -= 8;
    }

    // Finally read a partial byte
    if (numBits > 0) {
      int bitsToNotRead = 8 - numBits;
      int mask = (0xFF >> bitsToNotRead) << bitsToNotRead;
      result =
        (result << numBits) | ((bytes[byteOffset] & mask) >> bitsToNotRead);
      bitOffset += numBits;
    }
  }

  return result;
}
