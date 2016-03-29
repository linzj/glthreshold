#ifndef BITSOURCE_H
#define BITSOURCE_H
#include <stdint.h>
#include <vector>

class BitSource final
{

private:
  const std::vector<uint8_t>& bytes;
  int byteOffset;
  int bitOffset;

public:
  /**
   * @param bytes bytes from which this will read bits. Bits will be read from
   * the first byte first.
   * Bits are read within a byte from most-significant to least-significant bit.
   */
  BitSource(const std::vector<uint8_t>& bytes);

  /**
   * @return index of next bit in current byte which would be read by the next
   * call to {@link #readBits(int)}.
   */
  inline int getBitOffset() const { return bitOffset; }

  /**
   * @return index of next byte in input byte array which would be read by the
   * next call to {@link #readBits(int)}.
   */
  inline int getByteOffset() const { return byteOffset; }

  /**
   * @param numBits number of bits to read
   * @return int representing the bits read. The bits will appear as the
   * least-significant
   *         bits of the int
   * @throws IllegalArgumentException if numBits isn't in [1,32] or more than is
   * available
   */
  int readBits(int numBits);

  /**
   * @return number of bits that can be read successfully
   */
  int available() const { return 8 * (bytes.size() - byteOffset) - bitOffset; }
};
#endif /* BITSOURCE_H */
