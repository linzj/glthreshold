#include "DataMask.h"
#include "LuminanceImage.h"

namespace {
/**
 * 000: mask bits for which (x + y) mod 2 == 0
 */
class DataMask000 final : public DataMask
{
  bool isMasked(int i, int j) const override final
  {
    return ((i + j) & 0x01) == 0;
  }
};

/**
 * 001: mask bits for which x mod 2 == 0
 */
class DataMask001 final : public DataMask
{
  bool isMasked(int i, int j) const override final { return (i & 0x01) == 0; }
};

/**
 * 010: mask bits for which y mod 3 == 0
 */
class DataMask010 final : public DataMask
{
  bool isMasked(int i, int j) const override final { return j % 3 == 0; }
};

/**
 * 011: mask bits for which (x + y) mod 3 == 0
 */
class DataMask011 final : public DataMask
{
  bool isMasked(int i, int j) const override final { return (i + j) % 3 == 0; }
};

/**
 * 100: mask bits for which (x/2 + y/3) mod 2 == 0
 */
class DataMask100 final : public DataMask
{
  bool isMasked(int i, int j) const override final
  {
    return (((i / 2) + (j / 3)) & 0x01) == 0;
  }
};

/**
 * 101: mask bits for which xy mod 2 + xy mod 3 == 0
 */
class DataMask101 final : public DataMask
{
  bool isMasked(int i, int j) const override final
  {
    int temp = i * j;
    return (temp & 0x01) + (temp % 3) == 0;
  }
};

/**
 * 110: mask bits for which (xy mod 2 + xy mod 3) mod 2 == 0
 */
class DataMask110 final : public DataMask
{
  bool isMasked(int i, int j) const override final
  {
    int temp = i * j;
    return (((temp & 0x01) + (temp % 3)) & 0x01) == 0;
  }
};

/**
 * 111: mask bits for which ((x+y)mod 2 + xy mod 3) mod 2 == 0
 */
class DataMask111 final : public DataMask
{
  bool isMasked(int i, int j) const override final
  {
    return ((((i + j) & 0x01) + ((i * j) % 3)) & 0x01) == 0;
  }
};
}

const DataMask&
DataMask::getStatic(int i)
{
  static std::unique_ptr<DataMask> DATA_MASKS[] = {
    std::unique_ptr<DataMask>(new DataMask000),
    std::unique_ptr<DataMask>(new DataMask001),
    std::unique_ptr<DataMask>(new DataMask010),
    std::unique_ptr<DataMask>(new DataMask011),
    std::unique_ptr<DataMask>(new DataMask100),
    std::unique_ptr<DataMask>(new DataMask101),
    std::unique_ptr<DataMask>(new DataMask110),
    std::unique_ptr<DataMask>(new DataMask111),
  };
  return *DATA_MASKS[i];
}

void
DataMask::unmaskBitMatrix(LuminanceImage* bits, int dimension) const
{
  for (int i = 0; i < dimension; i++) {
    for (int j = 0; j < dimension; j++) {
      if (isMasked(i, j)) {
        bits->flip(j, i);
      }
    }
  }
}

const DataMask&
DataMask::forReference(int reference)
{
  if (reference < 0 || reference > 7) {
    throw 1;
  }
  return getStatic(reference);
}
