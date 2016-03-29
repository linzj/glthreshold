#include "GenericGF.h"
#include "GenericGFPoly.h"
#include <vector>

const GenericGF GenericGF::AZTEC_DATA_12(0x1069, 4096,
                                         1); // x^12 + x^6 + x^5 + x^3 + 1
const GenericGF GenericGF::AZTEC_DATA_10(0x409, 1024, 1); // x^10 + x^3 + 1
const GenericGF GenericGF::AZTEC_DATA_6(0x43, 64, 1);     // x^6 + x + 1
const GenericGF GenericGF::AZTEC_PARAM(0x13, 16, 1);      // x^4 + x + 1
const GenericGF GenericGF::QR_CODE_FIELD_256(0x011D, 256,
                                             0); // x^8 + x^4 + x^3 + x^2 + 1
const GenericGF GenericGF::DATA_MATRIX_FIELD_256(
  0x012D, 256,
  1); // x^8 + x^5 + x^3 + x^2 + 1
const GenericGF GenericGF::AZTEC_DATA_8(0x012D, 256,
                                        1); //= DATA_MATRIX_FIELD_256;
const GenericGF GenericGF::MAXICODE_FIELD_64(0x43, 64, 1); // = AZTEC_DATA_6;

GenericGF::GenericGF(int _primitive, int _size, int b)
  : primitive(_primitive)
  , size(_size)
  , generatorBase(b)
  , expTable(new int[_size])
  , logTable(new int[_size])
{
  int x = 1;
  for (int i = 0; i < size; i++) {
    expTable.get()[i] = x;
    x *= 2; // we're assuming the generator alpha is 2
    if (x >= size) {
      x ^= primitive;
      x &= size - 1;
    }
  }
  for (int i = 0; i < size - 1; i++) {
    logTable.get()[expTable.get()[i]] = i;
  }
  // logTable[0] == 0 but this should never be used
  zero.reset(new GenericGFPoly(this, { 0 }));
  one.reset(new GenericGFPoly(this, { 1 }));
}

std::shared_ptr<GenericGFPoly>
GenericGF::buildMonomial(int degree, int coefficient) const
{
  if (degree < 0) {
    throw 1;
  }
  if (coefficient == 0) {
    return zero;
  }
  std::vector<int> coefficients(degree + 1);
  coefficients[0] = coefficient;
  return std::shared_ptr<GenericGFPoly>(
    new GenericGFPoly(this, std::move(coefficients)));
}

int
GenericGF::log(int a) const
{
  if (a == 0) {
    throw 1;
  }
  return logTable.get()[a];
}

int
GenericGF::inverse(int a) const
{
  if (a == 0) {
    throw 1;
  }
  return expTable.get()[size - logTable.get()[a] - 1];
}

int
GenericGF::multiply(int a, int b) const
{
  if (a == 0 || b == 0) {
    return 0;
  }
  return expTable.get()[(logTable.get()[a] + logTable.get()[b]) % (size - 1)];
}
